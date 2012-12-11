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

#define LOG_TAG "TinyALSA-Audio Output"

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

#define EFFECT_UUID_NULL EFFECT_UUID_NULL_OUT
#define EFFECT_UUID_NULL_STR EFFECT_UUID_NULL_STR_OUT
#include <audio_utils/resampler.h>
#include "audio_hw.h"
#include "mixer.h"

/*
 * Functions
 */

int audio_out_pcm_open(struct tinyalsa_audio_stream_out *stream_out)
{
	struct pcm *pcm = NULL;
	struct pcm_config pcm_config;

	if(stream_out == NULL)
		return -1;

	memset(&pcm_config, 0, sizeof(pcm_config));
	pcm_config.channels = popcount(stream_out->mixer_props->channel_mask);
	pcm_config.rate = stream_out->mixer_props->rate;
	switch(stream_out->mixer_props->format) {
		case AUDIO_FORMAT_PCM_16_BIT:
			pcm_config.format = PCM_FORMAT_S16_LE;
			break;
		case AUDIO_FORMAT_PCM_32_BIT:
			pcm_config.format = PCM_FORMAT_S32_LE;
			break;
		default:
			ALOGE("Invalid format: 0x%x", stream_out->mixer_props->format);
			return -1;
	}
	pcm_config.period_size = stream_out->mixer_props->period_size;
	pcm_config.period_count = stream_out->mixer_props->period_count;

	pcm = pcm_open(stream_out->mixer_props->card,
		stream_out->mixer_props->device, PCM_OUT, &pcm_config);

	if(pcm == NULL || !pcm_is_ready(pcm)) {
		ALOGE("Unable to open pcm device: %s", pcm_get_error(pcm));
		return -1;
	}

	stream_out->pcm = pcm;

	if(stream_out->resampler != NULL)
		stream_out->resampler->reset(stream_out->resampler);

	return 0;
}

void audio_out_pcm_close(struct tinyalsa_audio_stream_out *stream_out)
{
	if(stream_out->pcm == NULL)
		return;

	pcm_close(stream_out->pcm);
	stream_out->pcm = NULL;
}

int audio_out_set_route(struct tinyalsa_audio_stream_out *stream_out,
	audio_devices_t device)
{
	int rc;

	if(stream_out == NULL)
		return -1;

	stream_out->device_current = device;

	if(device == 0) {
		pthread_mutex_unlock(&stream_out->lock);
		return stream_out->stream.common.standby((struct audio_stream *) stream_out);
	}

	tinyalsa_mixer_set_device(stream_out->device->mixer, stream_out->device_current);

#ifdef YAMAHA_MC1N2_AUDIO
	yamaha_mc1n2_audio_set_route(stream_out->device->mc1n2_pdata, device);
#endif

	return 0;
}

int audio_out_resampler_open(struct tinyalsa_audio_stream_out *stream_out)
{
	int rc;

	if(stream_out == NULL)
		return -1;

	rc = create_resampler(stream_out->rate,
		stream_out->mixer_props->rate,
		popcount(stream_out->channel_mask),
		RESAMPLER_QUALITY_DEFAULT,
		NULL,
		&stream_out->resampler);
	if(rc < 0 || stream_out->resampler == NULL) {
		ALOGE("Failed to create resampler");
		return -1;
	}

	return 0;
}

void audio_out_resampler_close(struct tinyalsa_audio_stream_out *stream_out)
{
	if(stream_out == NULL)
		return;

	if(stream_out->resampler != NULL) {
		release_resampler(stream_out->resampler);
		stream_out->resampler = NULL;
	}
}

int audio_out_write_process(struct tinyalsa_audio_stream_out *stream_out, void *buffer, int size)
{
	size_t frames_out;

	size_t frames_in;
	int size_in;
	void *buffer_in = NULL;

	int frames_out_resampler;
	int size_out_resampler;
	void *buffer_out_resampler = NULL;

	int frames_out_channels;
	int size_out_channels;
	void *buffer_out_channels = NULL;

	int i, j;
	int rc;

	if(stream_out == NULL || buffer == NULL || size <= 0)
		return -1;

	frames_in = size / audio_stream_frame_size((struct audio_stream *) stream_out);
	size_in = size;
	buffer_in = buffer;

	if(stream_out->resampler != NULL) {
		frames_out_resampler = (frames_in * stream_out->mixer_props->rate) /
			stream_out->rate;
		frames_out_resampler = ((frames_out_resampler + 15) / 16) * 16;
		size_out_resampler = frames_out_resampler * audio_stream_frame_size((struct audio_stream *) stream_out);
		buffer_out_resampler = calloc(1, size_out_resampler);

		frames_out = frames_out_resampler;
		stream_out->resampler->resample_from_input(stream_out->resampler,
			buffer_in, &frames_in, buffer_out_resampler, &frames_out);

		frames_in = frames_out;
		size_in = frames_out * audio_stream_frame_size((struct audio_stream *) stream_out);
		buffer_in = buffer_out_resampler;
	}

	if(buffer_in == NULL)
		goto error;

	//FIXME: This is only for PCM 16
	if(popcount(stream_out->mixer_props->channel_mask) < popcount(stream_out->channel_mask)) {
		frames_out_channels = frames_in;
		size_out_channels = frames_out_channels * popcount(stream_out->mixer_props->channel_mask) * audio_bytes_per_sample(stream_out->mixer_props->format);
		buffer_out_channels = calloc(1, size_out_channels);

		int channels_count_in = popcount(stream_out->channel_mask);
		int channels_count_out = popcount(stream_out->mixer_props->channel_mask);
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
	} else if(popcount(stream_out->channel_mask) != popcount(stream_out->mixer_props->channel_mask)) {
		ALOGE("Asked for more channels than software can provide!");
		goto error;
	}

	if(buffer_in != NULL) {
		if(stream_out->pcm == NULL || !pcm_is_ready(stream_out->pcm)) {
			ALOGE("pcm device is not ready");
			goto error;
		}

		rc = pcm_write(stream_out->pcm, buffer_in, size_in);
		if(rc != 0) {
			ALOGE("pcm write failed!");
			goto error;
		}
	}

	if(buffer_out_resampler != NULL)
		free(buffer_out_resampler);
	if(buffer_out_channels != NULL)
		free(buffer_out_channels);

	return 0;

error:
	if(buffer_out_resampler != NULL)
		free(buffer_out_resampler);
	if(buffer_out_channels != NULL)
		free(buffer_out_channels);

	return -1;
}

static uint32_t audio_out_get_sample_rate(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_out *stream_out;

	if(stream == NULL)
		return 0;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	return stream_out->rate;
}

static int audio_out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
	struct tinyalsa_audio_stream_out *stream_out;

	ALOGD("%s(%p, %d)", __func__, stream, rate);

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	if(stream_out->rate != (int) rate) {
		pthread_mutex_lock(&stream_out->lock);

		stream_out->rate = rate;

		if(stream_out->rate != stream_out->mixer_props->rate) {
			audio_out_resampler_close(stream_out);
			audio_out_resampler_open(stream_out);

			stream_out->standby = 1;
		}

		pthread_mutex_unlock(&stream_out->lock);
	}

	return 0;
}

static size_t audio_out_get_buffer_size(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_out *stream_out;
	size_t size;

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	size = (stream_out->mixer_props->period_size * stream_out->rate) /
		stream_out->mixer_props->rate;
	size = ((size + 15) / 16) * 16;
	size = size * audio_stream_frame_size((struct audio_stream *) stream);

	return size;
}

static audio_channel_mask_t audio_out_get_channels(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_out *stream_out;

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	return stream_out->channel_mask;
}

static audio_format_t audio_out_get_format(const struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_out *stream_out;

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	return stream_out->format;
}

static int audio_out_set_format(struct audio_stream *stream, audio_format_t format)
{
	struct tinyalsa_audio_stream_out *stream_out;

	ALOGD("%s(%p, %d)", __func__, stream, format);

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	if(stream_out->format != (audio_format_t) format) {
		pthread_mutex_lock(&stream_out->lock);

		stream_out->format = format;

		if(stream_out->format != stream_out->mixer_props->format)
			stream_out->standby = 1;

		pthread_mutex_unlock(&stream_out->lock);
	}

	return 0;
}

static int audio_out_standby(struct audio_stream *stream)
{
	struct tinyalsa_audio_stream_out *stream_out;
	int rc;

	ALOGD("%s(%p)", __func__, stream);

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	pthread_mutex_lock(&stream_out->lock);

	if(stream_out->pcm != NULL)
		audio_out_pcm_close(stream_out);

#ifdef YAMAHA_MC1N2_AUDIO
	if(!stream_out->standby) {
		rc = yamaha_mc1n2_audio_output_stop(stream_out->device->mc1n2_pdata);
		if(rc < 0) {
			ALOGE("Failed to set Yamaha-MC1N2-Audio route");
		}
	}
#endif

	stream_out->standby = 1;

	pthread_mutex_unlock(&stream_out->lock);

	return 0;
}

static int audio_out_dump(const struct audio_stream *stream, int fd)
{
	ALOGD("%s(%p, %d)", __func__, stream, fd);

	return 0;
}

static int audio_out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
	struct tinyalsa_audio_stream_out *stream_out;
	struct str_parms *parms;
	char value_string[32] = { 0 };
	int value;
	int rc;

	ALOGD("%s(%p, %s)", __func__, stream, kvpairs);

	if(stream == NULL || kvpairs == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	if(stream_out->device == NULL || stream_out->device->mixer == NULL)
		return -1;

	parms = str_parms_create_str(kvpairs);
	if(parms == NULL)
		return -1;

	rc = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value_string, sizeof(value_string));
	if(rc < 0)
		goto error_params;

	value = atoi(value_string);

	pthread_mutex_lock(&stream_out->device->lock);

	if(stream_out->device_current != (audio_devices_t) value) {
		pthread_mutex_lock(&stream_out->lock);
		audio_out_set_route(stream_out, (audio_devices_t) value);
		pthread_mutex_unlock(&stream_out->lock);
	}
	if(stream_out->device->ril_interface != NULL && stream_out->device->ril_interface->device_current != (audio_devices_t) value) {
		audio_ril_interface_set_route(stream_out->device->ril_interface, (audio_devices_t) value);
	}

	pthread_mutex_unlock(&stream_out->device->lock);

	str_parms_destroy(parms);

	return 0;

error_params:
	str_parms_destroy(parms);

	return -1;
}

static char *audio_out_get_parameters(const struct audio_stream *stream, const char *keys)
{
	ALOGD("%s(%p, %s)", __func__, stream, keys);

	return strdup("");
}

static uint32_t audio_out_get_latency(const struct audio_stream_out *stream)
{
	struct tinyalsa_audio_stream_out *stream_out;
	uint32_t latency;

	ALOGD("%s(%p)", __func__, stream);

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	latency = (stream_out->mixer_props->period_size *
		stream_out->mixer_props->period_count * 1000) /
		stream_out->mixer_props->rate;

	return latency;
}

static int audio_out_set_volume(struct audio_stream_out *stream, float left,
	float right)
{
	struct tinyalsa_audio_stream_out *stream_out;
	float volume;

	ALOGD("%s(%p, %f, %f)", __func__, stream, left, right);

	if(stream == NULL)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	if(stream_out->device == NULL || stream_out->device->mixer == NULL)
		return -1;

	volume = (left + right) / 2;

	pthread_mutex_lock(&stream_out->device->lock);
	tinyalsa_mixer_set_output_volume(stream_out->device->mixer,
		stream_out->device_current, volume);
	pthread_mutex_unlock(&stream_out->device->lock);

	return 0;
}

static ssize_t audio_out_write(struct audio_stream_out *stream,
	const void *buffer, size_t bytes)
{
	struct tinyalsa_audio_stream_out *stream_out;
	int rc;

	if(stream == NULL || buffer == NULL || bytes <= 0)
		return -1;

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	if(stream_out->device == NULL)
		return -1;

	pthread_mutex_lock(&stream_out->lock);

	if(stream_out->standby) {
#ifdef YAMAHA_MC1N2_AUDIO
		rc = yamaha_mc1n2_audio_output_start(stream_out->device->mc1n2_pdata);
		if(rc < 0) {
			ALOGE("Failed to set Yamaha-MC1N2-Audio route");
		}
#endif

		rc = audio_out_pcm_open(stream_out);
		if(rc < 0) {
			ALOGE("Unable to open pcm device");
			goto error;
		}

		stream_out->standby = 0;
	}

	rc = audio_out_write_process(stream_out, (void *) buffer, (int) bytes);
	if(rc < 0) {
		ALOGE("Process and write failed!");
		goto error;
	}

	pthread_mutex_unlock(&stream_out->lock);

	return bytes;

error:
	pthread_mutex_unlock(&stream_out->lock);

	return -1;
}

static int audio_out_get_render_position(const struct audio_stream_out *stream,
	uint32_t *dsp_frames)
{
	ALOGD("%s(%p, %p)", __func__, stream, dsp_frames);

	return -EINVAL;
}

static int audio_out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
	ALOGD("%s(%p, %p)", __func__, stream, effect);

	return 0;
}

static int audio_out_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
	ALOGD("%s(%p, %p)", __func__, stream, effect);

	return 0;
}

/*
 * Interface
 */

void audio_hw_close_output_stream(struct audio_hw_device *dev,
	struct audio_stream_out *stream)
{
	struct tinyalsa_audio_stream_out *stream_out;
	struct tinyalsa_audio_device *tinyalsa_audio_device;

	ALOGD("%s(%p)", __func__, stream);

	stream_out = (struct tinyalsa_audio_stream_out *) stream;

	if(stream_out != NULL && stream_out->resampler != NULL)
		audio_out_resampler_close(stream_out);

#ifdef YAMAHA_MC1N2_AUDIO
	if(stream_out != NULL && !stream_out->standby)
		yamaha_mc1n2_audio_output_stop(stream_out->device->mc1n2_pdata);
#endif

	if(stream != NULL)
		free(stream);

	if(dev == NULL)
		return;

	tinyalsa_audio_device = (struct tinyalsa_audio_device *) dev;

	pthread_mutex_lock(&tinyalsa_audio_device->lock);

	tinyalsa_mixer_set_output_state(tinyalsa_audio_device->mixer, 0);
	tinyalsa_audio_device->stream_out = NULL;

	pthread_mutex_unlock(&tinyalsa_audio_device->lock);
}

int audio_hw_open_output_stream(struct audio_hw_device *dev,
                                audio_io_handle_t handle,
                                audio_devices_t devices,
                                audio_output_flags_t flags,
                                struct audio_config *config,
	                        struct audio_stream_out **stream_out)
{
	struct tinyalsa_audio_device *tinyalsa_audio_device;
	struct tinyalsa_audio_stream_out *tinyalsa_audio_stream_out;
	struct audio_stream_out *stream;
	int rc;

	ALOGD("%s(%p, %d, %p, %p)",
		__func__, dev, devices, config, stream_out);

	if(dev == NULL || config == NULL || stream_out == NULL)
		return -EINVAL;

        //tuna added this for JB...  but does it apply to us?
        *stream_out = NULL;

	tinyalsa_audio_device = (struct tinyalsa_audio_device *) dev;
	tinyalsa_audio_stream_out = calloc(1, sizeof(struct tinyalsa_audio_stream_out));

	if(tinyalsa_audio_stream_out == NULL)
		return -ENOMEM;

	tinyalsa_audio_stream_out->device = tinyalsa_audio_device;
	tinyalsa_audio_device->stream_out = tinyalsa_audio_stream_out;
	stream = &(tinyalsa_audio_stream_out->stream);

	stream->common.get_sample_rate = audio_out_get_sample_rate;
	stream->common.set_sample_rate = audio_out_set_sample_rate;
	stream->common.get_buffer_size = audio_out_get_buffer_size;
	stream->common.get_channels = audio_out_get_channels;
	stream->common.get_format = audio_out_get_format;
	stream->common.set_format = audio_out_set_format;
	stream->common.standby = audio_out_standby;
	stream->common.dump = audio_out_dump;
	stream->common.set_parameters = audio_out_set_parameters;
	stream->common.get_parameters = audio_out_get_parameters;
	stream->common.add_audio_effect = audio_out_add_audio_effect;
	stream->common.remove_audio_effect = audio_out_remove_audio_effect;
	stream->get_latency = audio_out_get_latency;
	stream->set_volume = audio_out_set_volume;
	stream->write = audio_out_write;
	stream->get_render_position = audio_out_get_render_position;

	if(tinyalsa_audio_device->mixer == NULL)
		goto error_stream;

	tinyalsa_audio_stream_out->mixer_props =
		tinyalsa_mixer_get_output_props(tinyalsa_audio_device->mixer);

	if(tinyalsa_audio_stream_out->mixer_props == NULL)
		goto error_stream;

	// Default values
	if(tinyalsa_audio_stream_out->mixer_props->rate == 0)
		tinyalsa_audio_stream_out->mixer_props->rate = 44100;
	if(tinyalsa_audio_stream_out->mixer_props->channel_mask == 0)
		tinyalsa_audio_stream_out->mixer_props->channel_mask = AUDIO_CHANNEL_OUT_STEREO;
	if(tinyalsa_audio_stream_out->mixer_props->format == 0)
		tinyalsa_audio_stream_out->mixer_props->format = AUDIO_FORMAT_PCM_16_BIT;

	//Default incoming data will always be 44100Hz, stereo, PCM 16
	if(config->sample_rate == 0)
		tinyalsa_audio_stream_out->rate = 44100;
	else
		tinyalsa_audio_stream_out->rate = config->sample_rate;
	if(config->channel_mask == 0)
		tinyalsa_audio_stream_out->channel_mask = AUDIO_CHANNEL_OUT_STEREO;
	else
		tinyalsa_audio_stream_out->channel_mask = config->channel_mask;
	if(config->format == 0)
		tinyalsa_audio_stream_out->format = AUDIO_FORMAT_PCM_16_BIT;
	else
		tinyalsa_audio_stream_out->format = config->format;

	if(tinyalsa_audio_stream_out->rate != tinyalsa_audio_stream_out->mixer_props->rate) {
		rc = audio_out_resampler_open(tinyalsa_audio_stream_out);
		if(rc < 0) {
			ALOGE("Unable to open resampler!");
			goto error_stream;
		}
	}

	config->sample_rate = (uint32_t) tinyalsa_audio_stream_out->rate;
	config->channel_mask = (uint32_t) tinyalsa_audio_stream_out->channel_mask;
	config->format = (uint32_t) tinyalsa_audio_stream_out->format;

	pthread_mutex_lock(&tinyalsa_audio_device->lock);

	rc = tinyalsa_mixer_set_output_state(tinyalsa_audio_device->mixer, 1);
	if(rc < 0) {
		ALOGE("Unable to set output state");
		pthread_mutex_unlock(&tinyalsa_audio_device->lock);
		goto error_stream;
	}

	pthread_mutex_lock(&tinyalsa_audio_stream_out->lock);

	audio_out_set_route(tinyalsa_audio_stream_out, devices);

	pthread_mutex_unlock(&tinyalsa_audio_device->lock);

	rc = audio_out_pcm_open(tinyalsa_audio_stream_out);
	if(rc < 0) {
		ALOGE("Unable to open pcm device");
		pthread_mutex_unlock(&tinyalsa_audio_stream_out->lock);
		goto error_stream;
	}

	audio_out_pcm_close(tinyalsa_audio_stream_out);

	tinyalsa_audio_stream_out->standby = 1;

	pthread_mutex_unlock(&tinyalsa_audio_stream_out->lock);

	*stream_out = stream;

	return 0;

error_stream:
	free(tinyalsa_audio_stream_out);
	tinyalsa_audio_device->stream_out = NULL;

	return -1;
}
