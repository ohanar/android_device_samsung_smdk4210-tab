/*
 * Copyright (C) 2012 Paul Kocialkowski <contact@paulk.fr>
 *
 * This is based on Galaxy Nexus audio.primary.tuna implementation:
 * Copyright 2011, The Android Open-Source Project
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

#define LOG_TAG "TinyALSA-Audio Hardware"

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include <cutils/str_parms.h>
#include <cutils/log.h>

#ifdef YAMAHA_MC1N2_AUDIO
#include <yamaha-mc1n2-audio.h>
#endif

#include "audio_hw.h"
#include "mixer.h"

/*
 * Functions
 */

static int audio_hw_init_check(const struct audio_hw_device *dev)
{
	struct tinyalsa_audio_device *device;

	ALOGD("%s(%p)", __func__, dev);

	if(dev == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	if(device->mixer == NULL)
		return -1;

	return 0;
}

static int audio_hw_set_voice_volume(struct audio_hw_device *dev, float volume)
{
	struct tinyalsa_audio_device *device;
	audio_devices_t device_modem;

	ALOGD("%s(%p, %f)++", __func__, dev, volume);

	if(dev == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	if(device->mixer == NULL)
		return -1;

	if(volume != device->voice_volume) {
		pthread_mutex_lock(&device->lock);

		if(device->mode == AUDIO_MODE_IN_CALL) {
			if(device->ril_interface != NULL)
				device_modem = device->ril_interface->device_current;
			else if(device->stream_out != NULL)
				device_modem = device->stream_out->device_current;
			else
				device_modem = AUDIO_DEVICE_OUT_EARPIECE;

			tinyalsa_mixer_set_voice_volume(device->mixer,
				device_modem, volume);

			if(device->ril_interface != NULL)
				audio_ril_interface_set_voice_volume(device->ril_interface, device_modem, volume);
		}

		device->voice_volume = volume;

		pthread_mutex_unlock(&device->lock);
	}

	ALOGD("%s(%p, %f)--", __func__, dev, volume);

	return 0;
}

static int audio_hw_set_master_volume(struct audio_hw_device *dev, float volume)
{
	struct tinyalsa_audio_device *device;

	ALOGD("%s(%p, %f)++", __func__, dev, volume);

	if(dev == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	if(device->mixer == NULL)
		return -1;

	pthread_mutex_lock(&device->lock);
	tinyalsa_mixer_set_master_volume(device->mixer, volume);
	pthread_mutex_unlock(&device->lock);

	ALOGD("%s(%p, %f)--", __func__, dev, volume);

	return 0;
}

static int audio_hw_set_mode(struct audio_hw_device *dev, int mode)
{
	struct tinyalsa_audio_ril_interface *ril_interface;
	struct tinyalsa_audio_device *device;
	audio_devices_t device_modem;
	int rc;

	ALOGD("%s(%p, %d)++", __func__, dev, mode);

	if(dev == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	if(mode != device->mode) {
		pthread_mutex_lock(&device->lock);

		if(mode == AUDIO_MODE_IN_CALL) {
			tinyalsa_mixer_set_modem_state(device->mixer, 1);

			if(device->stream_out != NULL)
				device_modem = device->stream_out->device_current;
			else
				device_modem = AUDIO_DEVICE_OUT_EARPIECE;

			tinyalsa_mixer_set_device(device->mixer, device_modem);

#ifdef YAMAHA_MC1N2_AUDIO
			rc = yamaha_mc1n2_audio_modem_start(device->mc1n2_pdata);
			if(rc < 0) {
				ALOGE("Failed to set Yamaha-MC1N2-Audio route");
			}
#endif

			rc = audio_ril_interface_open((struct audio_hw_device *) device, device_modem, &ril_interface);
			if(rc < 0 || ril_interface == NULL) {
				ALOGE("Failed to open RIL interface");
				device->ril_interface = NULL;
			} else {
				device->ril_interface = ril_interface;

				//Only enable dualmic for earpiece.
				if(device_modem == AUDIO_DEVICE_OUT_EARPIECE)
				  audio_ril_interface_set_twomic(ril_interface,TWO_MIC_SOLUTION_ON);

				if(device->voice_volume)
					audio_ril_interface_set_voice_volume(ril_interface, device_modem, device->voice_volume);
			}
		} else if(device->mode == AUDIO_MODE_IN_CALL) {
			tinyalsa_mixer_set_modem_state(device->mixer, 0);

			/* 
			 * Should be safe to ALWAYS disable it on exit
			 * But we should instrument secril-client to be sure
			 * when this is/isn't controlled - FIXME
			 */
			if(device->ril_interface != NULL) {
				audio_ril_interface_set_twomic(device->ril_interface,TWO_MIC_SOLUTION_OFF);
			}

#ifdef YAMAHA_MC1N2_AUDIO
			rc = yamaha_mc1n2_audio_modem_stop(device->mc1n2_pdata);
			if(rc < 0) {
				ALOGE("Failed to set Yamaha-MC1N2-Audio route");
			}
#endif

			if(device->ril_interface != NULL) {
				audio_ril_interface_close((struct audio_hw_device *) device, device->ril_interface);
			}
		}

		device->mode = mode;

		pthread_mutex_unlock(&device->lock);
	}

	ALOGD("%s(%p, %d)--", __func__, dev, mode);

	return 0;
}

static int audio_hw_set_mic_mute(struct audio_hw_device *dev, bool state)
{
	struct tinyalsa_audio_device *device;
	audio_devices_t device_modem;

	ALOGD("%s(%p, %d)++", __func__, dev, state);

	if(dev == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	if(device->mixer == NULL)
		return -1;

	if(device->mic_mute != state) {
		pthread_mutex_lock(&device->lock);

		if(device->mode == AUDIO_MODE_IN_CALL) {
			if(device->ril_interface != NULL)
				device_modem = device->ril_interface->device_current;
			else if(device->stream_out != NULL)
				device_modem = device->stream_out->device_current;
			else
				device_modem = AUDIO_DEVICE_OUT_EARPIECE;

			tinyalsa_mixer_set_mic_mute(device->mixer,
				device_modem, state);

			if(device->ril_interface != NULL)
				audio_ril_interface_set_mic_mute(device->ril_interface, state);
		} else {
			if(device->stream_in != NULL) {
				tinyalsa_mixer_set_mic_mute(device->mixer,
					device->stream_in->device_current, state);
			}
		}

		device->mic_mute = state;

		pthread_mutex_unlock(&device->lock);
	}

	ALOGD("%s(%p, %d)--", __func__, dev, state);

	return 0;
}

static int audio_hw_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
	struct tinyalsa_audio_device *device;

	ALOGD("%s(%p, %p)", __func__, dev, state);

	if(dev == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	*state = device->mic_mute;

	return 0;
}

static int audio_hw_set_parameters(struct audio_hw_device *dev,
	const char *kvpairs)
{
	struct tinyalsa_audio_device *device;
	struct str_parms *parms;
	char value_string[32] = { 0 };
	int value;
	int rc;

	ALOGD("%s(%p, %s)++", __func__, dev, kvpairs);

	if(dev == NULL || kvpairs == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	if(device->mixer == NULL)
		return -1;

	parms = str_parms_create_str(kvpairs);
	if(parms == NULL)
		return -1;

	rc = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value_string, sizeof(value_string));
	if(rc < 0)
		goto error_params;

	value = atoi(value_string);

	pthread_mutex_lock(&device->lock);

	if(audio_is_output_device((audio_devices_t) value)) {
		if(device->stream_out != NULL && device->stream_out->device_current != (audio_devices_t) value) {
			pthread_mutex_lock(&device->stream_out->lock);
			audio_out_set_route(device->stream_out, (audio_devices_t) value);
			pthread_mutex_unlock(&device->stream_out->lock);
		}
		if(device->ril_interface != NULL && device->ril_interface->device_current != (audio_devices_t) value) {
			audio_ril_interface_set_route(device->ril_interface, (audio_devices_t) value);
		}
	} else if(audio_is_input_device((audio_devices_t) value)) {
		if(device->stream_in != NULL && device->stream_in->device_current != (audio_devices_t) value) {
			pthread_mutex_lock(&device->stream_in->lock);
			audio_in_set_route(device->stream_in, (audio_devices_t) value);
			pthread_mutex_unlock(&device->stream_in->lock);
		}
	}

	pthread_mutex_unlock(&device->lock);

	str_parms_destroy(parms);

	ALOGD("%s(%p, %s)--", __func__, dev, kvpairs);

	return 0;

error_params:
	str_parms_destroy(parms);

	ALOGD("%s(%p, %s)-- (PARAMETER ERROR)", __func__, dev, kvpairs);

	return -1;
}

static char *audio_hw_get_parameters(const struct audio_hw_device *dev,
	const char *keys)
{
	ALOGD("%s(%p, %s)", __func__, dev, keys);

	return strdup("");
}

static size_t audio_hw_get_input_buffer_size(const struct audio_hw_device *dev,
	                                     const struct audio_config *config)
{
	struct tinyalsa_audio_device *device;
	struct tinyalsa_mixer_io_props *mixer_props;
	size_t size;

        int channel_count = popcount(config->channel_mask);

	ALOGD("%s(%p, %d, %d, %d)++", __func__, dev, config->sample_rate, config->format, channel_count);

	if(dev == NULL)
		return -1;

	device = (struct tinyalsa_audio_device *) dev;

	if(device->mixer == NULL)
		return -1;

	mixer_props = tinyalsa_mixer_get_input_props(device->mixer);
	if(mixer_props == NULL)
		return -1;

	// Default value
	if(mixer_props->rate == 0)
		mixer_props->rate = 44100;

	size = (mixer_props->period_size * config->sample_rate) / mixer_props->rate;
	size = ((size + 15) / 16) * 16;
	size = size * channel_count * audio_bytes_per_sample(config->format);

	ALOGD("%s(%p, %d, %d, %d)--", __func__, dev, config->sample_rate, config->format, channel_count);

	return size;
}

static int audio_hw_dump(const audio_hw_device_t *device, int fd)
{
	ALOGD("%s(%p, %d)", __func__, device, fd);

	return 0;
}

/*
 * Interface
 */

int audio_hw_close(hw_device_t *device)
{
	struct tinyalsa_audio_device *tinyalsa_audio_device;

	ALOGD("%s(%p)++", __func__, device);

	if(device != NULL) {
		tinyalsa_audio_device = (struct tinyalsa_audio_device *) device;

		if(tinyalsa_audio_device->mixer != NULL) {
			tinyalsa_mixer_close(tinyalsa_audio_device->mixer);
			tinyalsa_audio_device->mixer = NULL;
		}

#ifdef YAMAHA_MC1N2_AUDIO
		if(tinyalsa_audio_device->mc1n2_pdata != NULL) {
			yamaha_mc1n2_audio_stop(tinyalsa_audio_device->mc1n2_pdata);
			tinyalsa_audio_device->mc1n2_pdata = NULL;
		}
#endif

		free(device);
	}

	ALOGD("%s(%p)--", __func__, device);

	return 0;
}

int audio_hw_open(const hw_module_t *module, const char *name,
	hw_device_t **device)
{
	struct tinyalsa_audio_device *tinyalsa_audio_device = NULL;
	struct tinyalsa_mixer *tinyalsa_mixer = NULL;
	struct audio_hw_device *dev;
	int rc;

	ALOGD("%s(%p, %s, %p)++", __func__, module, name, device);

	if(device == NULL)
		return -EINVAL;

	if(strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
		return -EINVAL;

	tinyalsa_audio_device = calloc(1, sizeof(struct tinyalsa_audio_device));
	if(tinyalsa_audio_device == NULL)
		return -ENOMEM;

	dev = &(tinyalsa_audio_device->device);

	dev->common.tag = HARDWARE_DEVICE_TAG;
	dev->common.version = AUDIO_DEVICE_API_VERSION_2_0;
	dev->common.module = (struct hw_module_t *) module;
	dev->common.close = audio_hw_close;

	dev->init_check = audio_hw_init_check;
	dev->set_voice_volume = audio_hw_set_voice_volume;
	dev->set_master_volume = audio_hw_set_master_volume;
	dev->set_mode = audio_hw_set_mode;
	dev->set_mic_mute = audio_hw_set_mic_mute;
	dev->get_mic_mute = audio_hw_get_mic_mute;
	dev->set_parameters = audio_hw_set_parameters;
	dev->get_parameters = audio_hw_get_parameters;
	dev->get_input_buffer_size = audio_hw_get_input_buffer_size;

	dev->open_output_stream = audio_hw_open_output_stream;
	dev->close_output_stream = audio_hw_close_output_stream;

	dev->open_input_stream = audio_hw_open_input_stream;
	dev->close_input_stream = audio_hw_close_input_stream;

	dev->dump = audio_hw_dump;

#ifdef YAMAHA_MC1N2_AUDIO
	rc = yamaha_mc1n2_audio_start(&tinyalsa_audio_device->mc1n2_pdata,
		YAMAHA_MC1N2_AUDIO_DEVICE);
	if(rc < 0) {
		ALOGE("Failed to open Yamaha-MC1N2-Audio");
		goto error_device;
	}

	rc = yamaha_mc1n2_audio_init(tinyalsa_audio_device->mc1n2_pdata);
	if(rc < 0) {
		ALOGE("Failed to init Yamaha-MC1N2-Audio");
	}
#endif

	rc = tinyalsa_mixer_open(&tinyalsa_mixer, TINYALSA_MIXER_CONFIG_FILE);
	if(rc < 0 || tinyalsa_mixer == NULL) {
		ALOGE("Failed to open mixer!");
		goto error_device;
	}

	tinyalsa_audio_device->mixer = tinyalsa_mixer;

	*device = &(dev->common);

	ALOGD("%s(%p, %s, %p)--", __func__, module, name, device);

	return 0;

error_device:
	*device = NULL;
	free(tinyalsa_audio_device);

	ALOGD("%s(%p, %s, %p)-- (DEVICE ERROR)", __func__, module, name, device);

	return -1;
}

struct hw_module_methods_t audio_hw_module_methods = {
	.open = audio_hw_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "TinyALSA-Audio",
        .author = "Paul Kocialkowski",
        .methods = &audio_hw_module_methods,
	},
};
