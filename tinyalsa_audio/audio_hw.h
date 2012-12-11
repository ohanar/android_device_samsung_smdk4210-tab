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

#ifndef TINYALSA_AUDIO_HW_H
#define TINYALSA_AUDIO_HW_H

#include <hardware/hardware.h>
#include <hardware/audio.h>
#include <system/audio.h>

#include <audio_utils/resampler.h>

#ifdef YAMAHA_MC1N2_AUDIO
#include <yamaha-mc1n2-audio.h>
#endif

#include "mixer.h"
#include "audio_ril_interface.h"

struct tinyalsa_audio_stream_out {
	struct audio_stream_out stream;
	struct tinyalsa_audio_device *device;

	struct tinyalsa_mixer_io_props *mixer_props;
	int rate;
        audio_channel_mask_t channel_mask;
	audio_format_t format;

	audio_devices_t device_current;

	struct resampler_itfe *resampler;

	struct pcm *pcm;
	int standby;

	pthread_mutex_t lock;
};

struct tinyalsa_audio_stream_in {
	struct audio_stream_in stream;
	struct tinyalsa_audio_device *device;

	struct tinyalsa_mixer_io_props *mixer_props;
	int rate;

        audio_channel_mask_t channel_mask;
	audio_format_t format;

	audio_devices_t device_current;

	struct resampler_itfe *resampler;
	struct resampler_buffer_provider buffer_provider;
	void *buffer;
	int frames_left;

	struct pcm *pcm;
	int standby;

	pthread_mutex_t lock;
};

struct tinyalsa_audio_device {
	struct audio_hw_device device;

	struct tinyalsa_audio_stream_out *stream_out;
	struct tinyalsa_audio_stream_in *stream_in;
	struct tinyalsa_audio_ril_interface *ril_interface;

#ifdef YAMAHA_MC1N2_AUDIO
	struct yamaha_mc1n2_audio_pdata *mc1n2_pdata;
#endif

	struct tinyalsa_mixer *mixer;
	audio_mode_t mode;

	float voice_volume;
	int mic_mute;

	pthread_mutex_t lock;
};

int audio_out_set_route(struct tinyalsa_audio_stream_out *stream_out,
	audio_devices_t device);

void audio_hw_close_output_stream(struct audio_hw_device *dev,
	struct audio_stream_out *stream);
int audio_hw_open_output_stream(struct audio_hw_device *dev,
                                audio_io_handle_t handle,
                                audio_devices_t devices,
                                audio_output_flags_t flags,
                                struct audio_config *config,
                                struct audio_stream_out **stream_out);

int audio_in_set_route(struct tinyalsa_audio_stream_in *stream_in,
	audio_devices_t device);

void audio_hw_close_input_stream(struct audio_hw_device *dev,
	struct audio_stream_in *stream);
int audio_hw_open_input_stream(struct audio_hw_device *dev,
                               audio_io_handle_t handle,
                               audio_devices_t devices,
                               struct audio_config *config,
                               struct audio_stream_in **stream_in);

#endif
