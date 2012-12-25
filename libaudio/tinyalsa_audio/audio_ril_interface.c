/*
 * Copyright (C) 2012 Paul Kocialkowski <contact@paulk.fr>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define LOG_TAG "TinyALSA-Audio RIL Interface"

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <dlfcn.h>
#include <sys/time.h>

#include <cutils/log.h>

#include <cutils/properties.h>

#define EFFECT_UUID_NULL EFFECT_UUID_NULL_RIL
#define EFFECT_UUID_NULL_STR EFFECT_UUID_NULL_STR_RIL
#include "audio_hw.h"

#include "audio_ril_interface.h"

/* Function pointers */
void *(*_ril_open_client)(void);
int (*_ril_close_client)(void *);
int (*_ril_connect)(void *);
int (*_ril_is_connected)(void *);
int (*_ril_disconnect)(void *);
int (*_ril_set_call_volume)(void *, enum ril_sound_type, int);
int (*_ril_set_call_audio_path)(void *, enum ril_audio_path);
int (*_ril_set_call_clock_sync)(void *, enum ril_clock_state);
int (*_ril_set_call_twomic)(void *, enum ril_twomic_device, enum ril_twomic_enable);
int (*_ril_register_unsolicited_handler)(void *, int, void *);
int (*_ril_get_wb_amr)(void *, void *);

#define VOLUME_STEPS_DEFAULT  "5"
#define VOLUME_STEPS_PROPERTY "ro.config.vc_call_vol_steps"

static int audio_ril_interface_connect_if_required(struct tinyalsa_audio_ril_interface *ril_interface)
{
    if (_ril_is_connected(ril_interface->interface))
        return 0;

    if (_ril_connect(ril_interface->interface) != RIL_CLIENT_ERR_SUCCESS) {
        ALOGE("ril_connect() failed");
        return -1;
    }

    /* get wb amr status to set pcm samplerate depending on
       wb amr status when ril is connected. */
    /* FIXME: AMR */
#if 0
    if(_ril_get_wb_amr)
        _ril_get_wb_amr(ril_interface->client, ril_set_wb_amr_callback);
#endif

    return 0;
}

int audio_ril_interface_set_mic_mute(struct tinyalsa_audio_ril_interface *ril_interface, bool state)
{
  /*
   * If you look at the Replicant libaudio-ril-interface
   * this function is just stubbed out there.  So let's not
   * bother with it
   */
	return 0;

}

int audio_ril_interface_set_voice_volume(struct tinyalsa_audio_ril_interface *ril_interface,
	audio_devices_t device, float volume)
{
	int rc;

	enum ril_sound_type sound_type;

	if(ril_interface == NULL)
		return -1;

	ALOGD("%s(%d, %f)", __func__, device, volume);

	pthread_mutex_lock(&ril_interface->lock);

	/* Should this be returning -1 when a failure occurs? */
	if (audio_ril_interface_connect_if_required(ril_interface))
	  return 0;

	if(_ril_set_call_volume == NULL)
		goto error;

	switch((int) device) {
		case AUDIO_DEVICE_OUT_EARPIECE:
			sound_type = SOUND_TYPE_VOICE;
			break;
		case AUDIO_DEVICE_OUT_SPEAKER:
			sound_type = SOUND_TYPE_SPEAKER;
			break;
		case AUDIO_DEVICE_OUT_WIRED_HEADSET:
		case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
			sound_type = SOUND_TYPE_HEADSET;
			break;
		case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
		case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
		case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
		case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
		case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
		case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
			sound_type = SOUND_TYPE_BTVOICE;
			break;
		default:
			sound_type = SOUND_TYPE_VOICE;
			break;
	}

	rc = _ril_set_call_volume(ril_interface->interface, sound_type,
				  (int)(volume * ril_interface->volume_steps_max));

	if(rc < 0) {
		ALOGE("Failed to set RIL interface voice volume");
		goto error;
	}

	pthread_mutex_unlock(&ril_interface->lock);

	return 0;

error:
	pthread_mutex_unlock(&ril_interface->lock);

	return -1;
}

int audio_ril_interface_set_route(struct tinyalsa_audio_ril_interface *ril_interface, audio_devices_t device)
{
	int rc;

	enum ril_audio_path path;

	ALOGD("%s(%d)", __func__, device);

	if(ril_interface == NULL)
		return -1;

	pthread_mutex_lock(&ril_interface->lock);

	/* Should this be returning -1 when a failure occurs? */
	if (audio_ril_interface_connect_if_required(ril_interface))
	  return 0;

	ril_interface->device_current = device;

	if(_ril_set_call_audio_path == NULL)
		goto error;

	switch((int) device) {
		case AUDIO_DEVICE_OUT_EARPIECE:
			path = SOUND_AUDIO_PATH_HANDSET;
			break;
		case AUDIO_DEVICE_OUT_SPEAKER:
			path = SOUND_AUDIO_PATH_SPEAKER;
			break;
		case AUDIO_DEVICE_OUT_WIRED_HEADSET:
			path = SOUND_AUDIO_PATH_HEADSET;
			break;
		case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
			path = SOUND_AUDIO_PATH_HEADPHONE;
			break;
		// FIXME: Bluetooth values/path relation
		case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
		case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
		case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
			path = SOUND_AUDIO_PATH_BLUETOOTH;
		case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
		case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
		case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
			path = SOUND_AUDIO_PATH_BLUETOOTH_NO_NR;
			break;
		default:
			path = SOUND_AUDIO_PATH_HANDSET;
			break;
	}

	rc = _ril_set_call_audio_path(ril_interface->interface,path);

	if(rc < 0) {
		ALOGE("Failed to set RIL interface route");
		goto error;
	}

	pthread_mutex_unlock(&ril_interface->lock);

	return 0;

error:
	pthread_mutex_unlock(&ril_interface->lock);

	return -1;
}

int audio_ril_interface_set_twomic(struct tinyalsa_audio_ril_interface *ril_interface, enum ril_twomic_enable twomic)
{
	int rc;

	ALOGD("%s(%d)", __func__, twomic);

	if(ril_interface == NULL)
		return -1;

	pthread_mutex_lock(&ril_interface->lock);

	/* Should this be returning -1 when a failure occurs? */
	if (audio_ril_interface_connect_if_required(ril_interface))
	  return 0;

	if(_ril_set_call_twomic == NULL)
		goto error;

	rc = _ril_set_call_twomic(ril_interface->interface,AUDIENCE,twomic);

	if(rc < 0) {
		ALOGE("Failed to set RIL interface route");
		goto error;
	}

	pthread_mutex_unlock(&ril_interface->lock);

	return 0;

error:
	pthread_mutex_unlock(&ril_interface->lock);

	return -1;
}

/*
 * Interface
 */

void audio_ril_interface_close(struct audio_hw_device *dev,
	struct tinyalsa_audio_ril_interface *ril_interface)
{
	struct tinyalsa_audio_device *tinyalsa_audio_device;

	ALOGD("%s(%p)", __func__, ril_interface);

	if(ril_interface->dl_handle != NULL) {
	  if ((_ril_disconnect(ril_interface->interface) != RIL_CLIENT_ERR_SUCCESS) ||
	      (_ril_close_client(ril_interface->interface) != RIL_CLIENT_ERR_SUCCESS)) {
	    ALOGE("ril_disconnect() or ril_close_client() failed");
	    return;
	  }

	  dlclose(ril_interface->dl_handle);
	  ril_interface->dl_handle = NULL;
	}

	if(ril_interface != NULL)
		free(ril_interface);

	if(dev == NULL)
		return;

	tinyalsa_audio_device = (struct tinyalsa_audio_device *) dev;

	tinyalsa_audio_device->ril_interface = NULL;
}

int audio_ril_interface_open(struct audio_hw_device *dev, audio_devices_t device,
	struct tinyalsa_audio_ril_interface **ril_interface)
{
	struct audio_ril_interface *(*audio_ril_interface_open)(void);

	struct tinyalsa_audio_device *tinyalsa_audio_device;
	struct tinyalsa_audio_ril_interface *tinyalsa_audio_ril_interface;
	struct audio_ril_interface *interface;
	void *dl_handle;
	int rc;

	char property[PROPERTY_VALUE_MAX];

	ALOGD("%s(%p, %d, %p)", __func__, dev, device, ril_interface);

	if(dev == NULL || ril_interface == NULL)
		return -EINVAL;

	tinyalsa_audio_device = (struct tinyalsa_audio_device *) dev;
	tinyalsa_audio_ril_interface = calloc(1, sizeof(struct tinyalsa_audio_ril_interface));

	if(tinyalsa_audio_ril_interface == NULL)
		return -ENOMEM;

	tinyalsa_audio_ril_interface->device = tinyalsa_audio_device;
	tinyalsa_audio_device->ril_interface = tinyalsa_audio_ril_interface;

	dl_handle = dlopen(RIL_CLIENT_LIBPATH, RTLD_NOW);
	if(dl_handle == NULL) {
		ALOGE("Unable to dlopen lib: %s", RIL_CLIENT_LIBPATH);
		goto error_interface;
	}

	_ril_open_client = dlsym(dl_handle, "OpenClient_RILD");
	_ril_close_client = dlsym(dl_handle, "CloseClient_RILD");
	_ril_connect = dlsym(dl_handle, "Connect_RILD");
	_ril_is_connected = dlsym(dl_handle, "isConnected_RILD");
	_ril_disconnect = dlsym(dl_handle, "Disconnect_RILD");
	_ril_set_call_volume = dlsym(dl_handle, "SetCallVolume");
	_ril_set_call_audio_path = dlsym(dl_handle, "SetCallAudioPath");
	_ril_set_call_clock_sync = dlsym(dl_handle, "SetCallClockSync");
        _ril_set_call_twomic = dlsym(dl_handle, "SetTwoMicControl");

	_ril_register_unsolicited_handler = dlsym(dl_handle,
						  "RegisterUnsolicitedHandler");
	/* since this function is not supported in all RILs, don't require it */
	_ril_get_wb_amr = dlsym(dl_handle, "GetWB_AMR");

	if (!_ril_open_client || !_ril_close_client || !_ril_connect ||
	    !_ril_is_connected || !_ril_disconnect || !_ril_set_call_volume ||
	    !_ril_set_call_audio_path || !_ril_set_call_clock_sync ||
	    !_ril_register_unsolicited_handler || !_ril_set_call_twomic) {
	  ALOGE("Cannot get symbols from '%s'", RIL_CLIENT_LIBPATH);
	  dlclose(dl_handle);
	  goto error_interface;
	}

	interface = _ril_open_client();
	if(interface == NULL) {
		ALOGE("Unable to open audio ril interface");
		goto error_interface;
	}

	tinyalsa_audio_ril_interface->interface = interface;
	tinyalsa_audio_ril_interface->dl_handle = dl_handle;

	property_get(VOLUME_STEPS_PROPERTY, property, VOLUME_STEPS_DEFAULT);
	tinyalsa_audio_ril_interface->volume_steps_max = atoi(property);
	/* this catches the case where VOLUME_STEPS_PROPERTY does not contain
	   an integer */
	if (tinyalsa_audio_ril_interface->volume_steps_max == 0)
	  tinyalsa_audio_ril_interface->volume_steps_max = atoi(VOLUME_STEPS_DEFAULT);
	
	if(device) {
		tinyalsa_audio_ril_interface->device_current = device;
		audio_ril_interface_set_route(tinyalsa_audio_ril_interface, device);
	}

	*ril_interface = tinyalsa_audio_ril_interface;

	return 0;

error_interface:
	*ril_interface = NULL;
	free(tinyalsa_audio_ril_interface);
	tinyalsa_audio_device->ril_interface = NULL;

	if(dl_handle != NULL)
		dlclose(dl_handle);

	return -1;
}
