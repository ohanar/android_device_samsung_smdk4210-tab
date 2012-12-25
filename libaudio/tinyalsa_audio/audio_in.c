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

#define LOG_TAG "TinyALSA-Audio Input"

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include <cutils/str_parms.h>
#include <cutils/log.h>

#define EFFECT_UUID_NULL EFFECT_UUID_NULL_IN
#define EFFECT_UUID_NULL_STR EFFECT_UUID_NULL_STR_IN
#include <audio_utils/resampler.h>
#include "audio_hw.h"
#include "mixer.h"

/*
 * Functions
 */

int audio_in_pcm_open(struct tinyalsa_audio_stream_in *stream_in)
{
	struct pcm *pcm = NULL;
	struct pcm_config pcm_config;

	if(stream_in == NULL)
		return -1;

	memset(&pcm_config, 0, sizeof(pcm_config));
	pcm_config.channels = popcount(stream_in->mixer_props->channel_mask);
	pcm_config.rate = stream_in->mixer_props->rate;
	switch(stream_in->mixer_props->format) {
		case AUDIO_FORMAT_PCM_16_BIT:
			pcm_config.format = PCM_FORMAT_S16_LE;
			break;
		case AUDIO_FORMAT_PCM_32_BIT:
			pcm_config.format = PCM_FORMAT_S32_LE;
			break;
		default:
			ALOGE("Invalid format: 0x%x", stream_in->mixer_props->format);
			return -1;
	}
	pcm_config.period_size = stream_in->mixer_props->period_size;
	pcm_config.period_count = stream_in->mixer_props->period_count;

	pcm = pcm_open(stream_in->mixer_props->card,
		stream_in->mixer_props->device, PCM_IN, &pcm_config);

	if(pcm == NULL || !pcm_is_ready(pcm)) {
		ALOGE("Unable to open pcm device: %s", pcm_get_error(pcm));
		return -1;
	}

	stream_in->pcm = pcm;

	if(stream_in->resampler != NULL)
		stream_in->resampler->reset(stream_in->resampler);

	return 0;
}

void audio_in_pcm_close(struct tinyalsa_audio_stream_in *stream_in)
{
	if(stream_in->pcm == NULL)
		return;

	pcm_close(stream_in->pcm);
	stream_in->pcm = NULL;
}

int audio_in_set_route(struct tinyalsa_audio_stream_in *stream_in,
	audio_devices_t device)
{
	int rc;

	if(stream_in == NULL)
		return -1;

	stream_in->device_current = device;

	if(device == 0) {
		pthread_mutex_unlock(&stream_in->lock);
		return stream_in->stream.common.standby((struct audio_stream *) stream_in);
	}

	tinyalsa_mixer_set_device(stream_in->device->mixer, stream_in->device_current);

#ifdef YAMAHA_MC1N2_AUDIO
	yamaha_mc1n2_audio_set_route(stream_in->device->mc1n2_pdata, device);
#endif

	return 0;
}

int audio_in_resampler_open(struct tinyalsa_audio_stream_in *stream_in)
{
	int rc;

	if(stream_in == NULL)
		return -1;

	rc = create_resampler(stream_in->mixer_props->rate,
		stream_in->rate,
		popcount(stream_in->mixer_props->channel_mask),
		RESAMPLER_QUALITY_DEFAULT,
		&stream_in->buffer_provider,
		&stream_in->resampler);
	if(rc < 0 || stream_in->resampler == NULL) {
		ALOGE("Failed to create resampler");
		return -1;
	}

	stream_in->buffer = malloc(stream_in->mixer_props->period_size *
                                   popcount(stream_in->mixer_props->channel_mask) *
                                   audio_bytes_per_sample(stream_in->mixer_props->format));

	return 0;
}

void audio_in_resampler_close(struct tinyalsa_audio_stream_in *stream_in)
{
	if(stream_in == NULL)
		return;

	if(stream_in->resampler != NULL) {
		release_resampler(stream_in->resampler);
		stream_in->resampler = NULL;
	}

	if(stream_in->buffer != NULL) {
		free(stream_in->buffer);
		stream_in->buffer = NULL;
	}
}

int audio_in_get_next_buffer(struct resampler_buffer_provider *buffer_provider,
	struct resampler_buffer *buffer)
{
	struct tinyalsa_audio_stream_in *stream_in;
	int rc;

	if(buffer_provider == NULL || buffer == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) ((void *) buffer_provider -
		offsetof(struct tinyalsa_audio_stream_in, buffer_provider));

	if(stream_in->frames_left == 0) {
		if(stream_in->pcm == NULL || !pcm_is_ready(stream_in->pcm)) {
			ALOGE("pcm device is not ready");
			goto error_pcm;
		}

		rc = pcm_read(stream_in->pcm, stream_in->buffer,
			stream_in->mixer_props->period_size *
			popcount(stream_in->mixer_props->channel_mask) *
			audio_bytes_per_sample(stream_in->mixer_props->format));
		if(rc != 0) {
			ALOGE("pcm read failed!");
			goto error_pcm;
		}

		stream_in->frames_left = stream_in->mixer_props->period_size;
	}

	buffer->frame_count = (buffer->frame_count > stream_in->frames_left) ?
		stream_in->frames_left : buffer->frame_count;

	buffer->raw = stream_in->buffer +
		(stream_in->mixer_props->period_size - stream_in->frames_left) *
		popcount(stream_in->mixer_props->channel_mask) *
		audio_bytes_per_sample(stream_in->mixer_props->format);

	return 0;

error_pcm:
	buffer->raw = NULL;
	buffer->frame_count = 0;
	return -1;
}

void audio_in_release_buffer(struct resampler_buffer_provider *buffer_provider,
	struct resampler_buffer *buffer)
{
	struct tinyalsa_audio_stream_in *stream_in;

	if(buffer_provider == NULL || buffer == NULL)
		return;

	stream_in = (struct tinyalsa_audio_stream_in *) ((void *) buffer_provider -
		offsetof(struct tinyalsa_audio_stream_in, buffer_provider));

	stream_in->frames_left -= buffer->frame_count;
}

int audio_in_read_process(struct tinyalsa_audio_stream_in *stream_in, void *buffer, int size)
{
	size_t frames_out;

	size_t frames_in;
	int size_in;
	void *buffer_in = NULL;

	int frames_out_resampler;
	int size_out_resampler;
	void *buffer_out_resampler = NULL;

	int frames_out_read;
	int size_out_read;
	void *buffer_out_read = NULL;

	int frames_out_channels;
	int size_out_channels;
	void *buffer_out_channels = NULL;

	int i, j;
	int rc;

	if(stream_in == NULL || buffer == NULL || size <= 0)
		return -1;

	frames_in = size / audio_stream_frame_size((struct audio_stream *) stream_in);

	if(stream_in->resampler != NULL) {
		frames_out_resampler = frames_in;
		size_out_resampler = frames_out_resampler * popcount(stream_in->mixer_props->channel_mask) * audio_bytes_per_sample(stream_in->mixer_props->format);
		buffer_out_resampler = calloc(1, size_out_resampler);

		frames_out = 0;
		while(frames_out < frames_out_resampler) {
			frames_in = frames_out_resampler - frames_out;
			stream_in->resampler->resample_from_provider(stream_in->resampler,
				buffer_out_resampler + (size_out_resampler / frames_out_resampler) * frames_out,
				&frames_in);

			frames_out += frames_in;
		}

		frames_in = frames_out_resampler;
		size_in = size_out_resampler;
		buffer_in = buffer_out_resampler;
	} else {
		frames_out_read = frames_in;
		size_out_read = frames_out_read * popcount(stream_in->mixer_props->channel_mask) * audio_bytes_per_sample(stream_in->mixer_props->format);
		buffer_out_read = calloc(1, size_out_read);

		if(stream_in->pcm == NULL || !pcm_is_ready(stream_in->pcm)) {
			ALOGE("pcm device is not ready");
			goto error;
		}

		rc = pcm_read(stream_in->pcm, buffer_out_read, size_out_read);
		if(rc != 0) {
			ALOGE("pcm read failed!");
			goto error;
		}

		frames_in = frames_out_read;
		size_in = size_out_read;
		buffer_in = buffer_out_read;
	}

	if(buffer_in == NULL)
		goto error;

	//FIXME: This is only for PCM 16
	if(popcount(stream_in->channel_mask) < popcount(stream_in->mixer_props->channel_mask)) {
		frames_out_channels = frames_in;
		size_out_channels = frames_out_channels * audio_stream_frame_size((struct audio_stream *) stream_in);
		buffer_out_channels = calloc(1, size_out_channels);

		int channels_count_in = popcount(stream_in->mixer_props->channel_mask);
		int channels_count_out = popcount(stream_in->channel_mask);
		int ratio = channels_count_in / channels_count_out;

		int16_t *byte_in = (int16_t *) buffer_in;
		int16_t *byte_out = (int16_t *) buffer_out_channels;
		int16_t byte;

		for(i=0 ; i < frames_out_channels * channels_count_out; i++) {
			byte = 0;
			for(j=0 ; j < ratio ; j++) {
				byte += *byte_in / ratio;
				byte_in++;
			}

			*byte_out = byte;
			byte_out++;
		}

		frames_in = frames_out_channels;
		size_in = size_out_channels;
		buffer_in = buffer_out_channels;
	} else if(popcount(stream_in->channel_mask) != popcount(stream_in->mixer_props->channel_mask)) {
		ALOGE("Asked for more channels than hardware can provide!");
		goto error;
	}

	if(buffer_in != NULL)
		memcpy(buffer, buffer_in, size);

	if(buffer_out_resampler != NULL)
		free(buffer_out_resampler);
	if(buffer_out_read != NULL)
		free(buffer_out_read);
	if(buffer_out_channels != NULL)
		free(buffer_out_channels);

	return 0;

error:
	if(buffer_out_resampler != NULL)
		free(buffer_out_resampler);
	if(buffer_out_read != NULL)
		free(buffer_out_read);
	if(buffer_out_channels != NULL)
		free(buffer_out_channels);

	return -1;
}

static uint32_t audio_in_get_sample_rate(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_in *stream_in;

	if(stream == NULL)
		return 0;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	return stream_in->rate;
}

static int audio_in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
	struct tinyalsa_audio_stream_in *stream_in;

	ALOGD("%s(%p, %d)", __func__, stream, rate);

	if(stream == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	if(stream_in->rate != (int) rate) {
		pthread_mutex_lock(&stream_in->lock);

		stream_in->rate = rate;

		if(stream_in->rate != stream_in->mixer_props->rate) {
			audio_in_resampler_close(stream_in);
			audio_in_resampler_open(stream_in);

			stream_in->standby = 1;
		}

		pthread_mutex_unlock(&stream_in->lock);
	}

	return 0;
}

static size_t audio_in_get_buffer_size(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_in *stream_in;
	size_t size;

	if(stream == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	size = (stream_in->mixer_props->period_size * stream_in->rate) /
		stream_in->mixer_props->rate;
	size = ((size + 15) / 16) * 16;
	size = size * audio_stream_frame_size((struct audio_stream *) stream);

	return size;
}

static audio_channel_mask_t audio_in_get_channels(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_in *stream_in;

	if(stream == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	return stream_in->channel_mask;
}

static audio_format_t audio_in_get_format(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_in *stream_in;

	if(stream == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	return stream_in->format;
}

static int audio_in_set_format(struct audio_stream *stream, audio_format_t format)
{
	struct tinyalsa_audio_stream_in *stream_in;

	ALOGD("%s(%p, %d)", __func__, stream, format);

	if(stream == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	if(stream_in->format != (audio_format_t) format) {
		pthread_mutex_lock(&stream_in->lock);

		stream_in->format = format;

		if(stream_in->format != stream_in->mixer_props->format)
			stream_in->standby = 1;

		pthread_mutex_unlock(&stream_in->lock);
	}

	return 0;
}

static int audio_in_standby(struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_in *stream_in;
	int rc;

	ALOGD("%s(%p)", __func__, stream);

	if(stream == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	pthread_mutex_lock(&stream_in->lock);

	if(stream_in->pcm != NULL)
		audio_in_pcm_close(stream_in);

#ifdef YAMAHA_MC1N2_AUDIO
	if(!stream_in->standby) {
		rc = yamaha_mc1n2_audio_input_stop(stream_in->device->mc1n2_pdata);
		if(rc < 0) {
			ALOGE("Failed to set Yamaha-MC1N2-Audio route");
		}
	}
#endif

	stream_in->standby = 1;

	pthread_mutex_unlock(&stream_in->lock);

	return 0;
}

static int audio_in_dump(const struct audio_stream *stream, int fd)
{
	ALOGD("%s(%p, %d)", __func__, stream, fd);

	return 0;
}

static int audio_in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
	struct tinyalsa_audio_stream_in *stream_in;
	struct str_parms *parms;
	char value_string[32] = { 0 };
	int value;
	int rc;

	ALOGD("%s(%p, %s)", __func__, stream, kvpairs);

	if(stream == NULL || kvpairs == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	if(stream_in->device == NULL || stream_in->device->mixer == NULL)
		return -1;

	parms = str_parms_create_str(kvpairs);
	if(parms == NULL)
		return -1;

	rc = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value_string, sizeof(value_string));
	if(rc < 0)
		goto error_params;

	value = atoi(value_string);

	pthread_mutex_lock(&stream_in->device->lock);

	if(stream_in->device_current != (audio_devices_t) value) {
		pthread_mutex_lock(&stream_in->lock);
		audio_in_set_route(stream_in, (audio_devices_t) value);
		pthread_mutex_unlock(&stream_in->lock);
	}

	pthread_mutex_unlock(&stream_in->device->lock);

	str_parms_destroy(parms);

	return 0;

error_params:
	str_parms_destroy(parms);

	return -1;
}

static char *audio_in_get_parameters(const struct audio_stream *stream,
	const char *keys)
{
	ALOGD("%s(%p, %s)", __func__, stream, keys);

	return strdup("");
}

static int audio_in_set_gain(struct audio_stream_in *stream, float gain)
{
	struct tinyalsa_audio_stream_in *stream_in;

	ALOGD("%s(%p, %f)", __func__, stream, gain);

	if(stream == NULL)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	if(stream_in->device == NULL || stream_in->device->mixer == NULL)
		return -1;

	pthread_mutex_lock(&stream_in->device->lock);
	tinyalsa_mixer_set_input_gain(stream_in->device->mixer,
		stream_in->device_current, gain);
	pthread_mutex_unlock(&stream_in->device->lock);

	return 0;
}

static ssize_t audio_in_read(struct audio_stream_in *stream,
	void *buffer, size_t bytes)
{
	struct tinyalsa_audio_stream_in *stream_in;
	int rc;

	if(stream == NULL || buffer == NULL || bytes <= 0)
		return -1;

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	if(stream_in->device == NULL)
		return -1;

	pthread_mutex_lock(&stream_in->lock);

	if(stream_in->standby) {
#ifdef YAMAHA_MC1N2_AUDIO
		rc = yamaha_mc1n2_audio_input_start(stream_in->device->mc1n2_pdata);
		if(rc < 0) {
			ALOGE("Failed to set Yamaha-MC1N2-Audio route");
		}
#endif

		rc = audio_in_pcm_open(stream_in);
		if(rc < 0) {
			ALOGE("Unable to open pcm device");
			goto error;
		}

		stream_in->standby = 0;
	}

	rc = audio_in_read_process(stream_in, buffer, (int) bytes);
	if(rc < 0) {
		ALOGE("Read and process failed!");
		goto error;
	}

	if(stream_in->device != NULL && stream_in->device->mic_mute)
		memset(buffer, 0, bytes);

	pthread_mutex_unlock(&stream_in->lock);

	return bytes;

error:
	pthread_mutex_unlock(&stream_in->lock);

	return -1;
}

static uint32_t audio_in_get_input_frames_lost(struct audio_stream_in *stream)
{
	return 0;
}

static int audio_in_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
	ALOGD("%s(%p, %p)", __func__, stream, effect);

	return 0;
}

static int audio_in_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
	ALOGD("%s(%p, %p)", __func__, stream, effect);

	return 0;
}

/*
 * Interface
 */

void audio_hw_close_input_stream(struct audio_hw_device *dev,
	struct audio_stream_in *stream)
{
	struct tinyalsa_audio_stream_in *stream_in;
	struct tinyalsa_audio_device *tinyalsa_audio_device;

	ALOGD("%s(%p)", __func__, stream);

	stream_in = (struct tinyalsa_audio_stream_in *) stream;

	if(stream_in != NULL && stream_in->resampler != NULL)
		audio_in_resampler_close(stream_in);

#ifdef YAMAHA_MC1N2_AUDIO
	if(stream_in != NULL && !stream_in->standby)
		yamaha_mc1n2_audio_input_stop(stream_in->device->mc1n2_pdata);
#endif

	if(stream != NULL)
		free(stream);

	if(dev == NULL)
		return;

	tinyalsa_audio_device = (struct tinyalsa_audio_device *) dev;

	pthread_mutex_lock(&tinyalsa_audio_device->lock);

	tinyalsa_mixer_set_input_state(tinyalsa_audio_device->mixer, 0);
	tinyalsa_audio_device->stream_in = NULL;

	pthread_mutex_unlock(&tinyalsa_audio_device->lock);
}

int audio_hw_open_input_stream(struct audio_hw_device *dev,
                               audio_io_handle_t handle,
                               audio_devices_t devices,
                               struct audio_config *config,
	                       struct audio_stream_in **stream_in)
{
	struct tinyalsa_audio_device *tinyalsa_audio_device;
	struct tinyalsa_audio_stream_in *tinyalsa_audio_stream_in;
	struct audio_stream_in *stream;
	int rc;

	ALOGD("%s(%p, %d, %p, %p)",
		__func__, dev, devices, config, stream_in);

	if(dev == NULL || config == NULL || stream_in == NULL)
		return -EINVAL;

        *stream_in = NULL;

	tinyalsa_audio_device = (struct tinyalsa_audio_device *) dev;
	tinyalsa_audio_stream_in = calloc(1, sizeof(struct tinyalsa_audio_stream_in));

	if(tinyalsa_audio_stream_in == NULL)
		return -ENOMEM;

	tinyalsa_audio_stream_in->device = tinyalsa_audio_device;
	tinyalsa_audio_device->stream_in = tinyalsa_audio_stream_in;
	stream = &(tinyalsa_audio_stream_in->stream);

	stream->common.get_sample_rate = audio_in_get_sample_rate;
	stream->common.set_sample_rate = audio_in_set_sample_rate;
	stream->common.get_buffer_size = audio_in_get_buffer_size;
	stream->common.get_channels = audio_in_get_channels;
	stream->common.get_format = audio_in_get_format;
	stream->common.set_format = audio_in_set_format;
	stream->common.standby = audio_in_standby;
	stream->common.dump = audio_in_dump;
	stream->common.set_parameters = audio_in_set_parameters;
	stream->common.get_parameters = audio_in_get_parameters;
	stream->common.add_audio_effect = audio_in_add_audio_effect;
	stream->common.remove_audio_effect = audio_in_remove_audio_effect;
	stream->set_gain = audio_in_set_gain;
	stream->read = audio_in_read;
	stream->get_input_frames_lost = audio_in_get_input_frames_lost;

	if(tinyalsa_audio_device->mixer == NULL)
		goto error_stream;

	tinyalsa_audio_stream_in->mixer_props =
		tinyalsa_mixer_get_input_props(tinyalsa_audio_device->mixer);

	if(tinyalsa_audio_stream_in->mixer_props == NULL)
		goto error_stream;

	// Default values
	if(tinyalsa_audio_stream_in->mixer_props->rate == 0)
		tinyalsa_audio_stream_in->mixer_props->rate = 44100;
	if(tinyalsa_audio_stream_in->mixer_props->channel_mask == 0)
		tinyalsa_audio_stream_in->mixer_props->channel_mask = AUDIO_CHANNEL_IN_STEREO;
	if(tinyalsa_audio_stream_in->mixer_props->format == 0)
		tinyalsa_audio_stream_in->mixer_props->format = AUDIO_FORMAT_PCM_16_BIT;

	if(config->sample_rate == 0)
		tinyalsa_audio_stream_in->rate =
			tinyalsa_audio_stream_in->mixer_props->rate;
	else
		tinyalsa_audio_stream_in->rate = config->sample_rate;
	if(config->channel_mask == 0)
		tinyalsa_audio_stream_in->channel_mask =
			tinyalsa_audio_stream_in->mixer_props->channel_mask;
	else
		tinyalsa_audio_stream_in->channel_mask = config->channel_mask;
	if(config->format == 0)
		tinyalsa_audio_stream_in->format =
			tinyalsa_audio_stream_in->mixer_props->format;
	else
		tinyalsa_audio_stream_in->format = config->format;

        tinyalsa_audio_stream_in->buffer_provider.get_next_buffer =
		audio_in_get_next_buffer;
        tinyalsa_audio_stream_in->buffer_provider.release_buffer =
		audio_in_release_buffer;

	if(tinyalsa_audio_stream_in->rate != tinyalsa_audio_stream_in->mixer_props->rate) {
		rc = audio_in_resampler_open(tinyalsa_audio_stream_in);
		if(rc < 0) {
			ALOGE("Unable to open resampler!");
			goto error_stream;
		}
	}

	config->sample_rate = tinyalsa_audio_stream_in->rate;
	config->channel_mask = tinyalsa_audio_stream_in->channel_mask;
	config->format = tinyalsa_audio_stream_in->format;

	pthread_mutex_lock(&tinyalsa_audio_device->lock);

	rc = tinyalsa_mixer_set_input_state(tinyalsa_audio_device->mixer, 1);
	if(rc < 0) {
		ALOGE("Unable to set input state");
		pthread_mutex_unlock(&tinyalsa_audio_device->lock);
		goto error_stream;
	}

	pthread_mutex_lock(&tinyalsa_audio_stream_in->lock);

	audio_in_set_route(tinyalsa_audio_stream_in, devices);

	pthread_mutex_unlock(&tinyalsa_audio_device->lock);

	rc = audio_in_pcm_open(tinyalsa_audio_stream_in);
	if(rc < 0) {
		ALOGE("Unable to open pcm device");
		pthread_mutex_unlock(&tinyalsa_audio_stream_in->lock);
		goto error_stream;
	}

	audio_in_pcm_close(tinyalsa_audio_stream_in);

	tinyalsa_audio_stream_in->standby = 1;

	pthread_mutex_unlock(&tinyalsa_audio_stream_in->lock);

	*stream_in = stream;

	return 0;

error_stream:
	if(tinyalsa_audio_stream_in->resampler != NULL)
		audio_in_resampler_close(tinyalsa_audio_stream_in);
	free(tinyalsa_audio_stream_in);
	tinyalsa_audio_device->stream_in = NULL;

	return -1;
}
