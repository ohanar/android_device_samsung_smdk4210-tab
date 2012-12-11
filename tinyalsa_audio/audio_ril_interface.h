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

#include "audio_hw.h"

#ifndef TINYALSA_AUDIO_RIL_INTERFACE_H
#define TINYALSA_AUDIO_RIL_INTERFACE_H

struct tinyalsa_audio_ril_interface {
	void *interface;
	struct tinyalsa_audio_device *device;

        int volume_steps_max;

	void *dl_handle;

	audio_devices_t device_current;

	pthread_mutex_t lock;
};

#define RIL_CLIENT_LIBPATH "libsecril-client.so"

#define RIL_CLIENT_ERR_SUCCESS      0
#define RIL_CLIENT_ERR_AGAIN        1
#define RIL_CLIENT_ERR_INIT         2 // Client is not initialized
#define RIL_CLIENT_ERR_INVAL        3 // Invalid value
#define RIL_CLIENT_ERR_CONNECT      4 // Connection error
#define RIL_CLIENT_ERR_IO           5 // IO error
#define RIL_CLIENT_ERR_RESOURCE     6 // Resource not available
#define RIL_CLIENT_ERR_UNKNOWN      7

enum ril_sound_type {
    SOUND_TYPE_VOICE,
    SOUND_TYPE_SPEAKER,
    SOUND_TYPE_HEADSET,
    SOUND_TYPE_BTVOICE
};

enum ril_audio_path {
    SOUND_AUDIO_PATH_HANDSET,
    SOUND_AUDIO_PATH_HEADSET,
    SOUND_AUDIO_PATH_SPEAKER,
    SOUND_AUDIO_PATH_BLUETOOTH,
    SOUND_AUDIO_PATH_BLUETOOTH_NO_NR,
    SOUND_AUDIO_PATH_HEADPHONE
};

enum ril_clock_state {
    SOUND_CLOCK_STOP,
    SOUND_CLOCK_START
};

/**
 * Two mic Solution control
 * Two MIC Solution Device
 */
enum ril_twomic_device {
    AUDIENCE,
    FORTEMEDIA
};

/**
 * Two MIC Solution Report
 */
enum ril_twomic_enable {
    TWO_MIC_SOLUTION_OFF,
    TWO_MIC_SOLUTION_ON
};

int audio_ril_interface_set_mic_mute(struct tinyalsa_audio_ril_interface *ril_interface, bool state);
int audio_ril_interface_set_voice_volume(struct tinyalsa_audio_ril_interface *ril_interface, audio_devices_t device, float volume);
int audio_ril_interface_set_route(struct tinyalsa_audio_ril_interface *ril_interface, audio_devices_t device);
int audio_ril_interface_set_twomic(struct tinyalsa_audio_ril_interface *ril_interface, enum ril_twomic_enable);

void audio_ril_interface_close(struct audio_hw_device *dev,
	struct tinyalsa_audio_ril_interface *interface);
int audio_ril_interface_open(struct audio_hw_device *dev, audio_devices_t device,
	struct tinyalsa_audio_ril_interface **ril_interface);

#endif
