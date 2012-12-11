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

#ifndef TINYALSA_AUDIO_MIXER_H
#define TINYALSA_AUDIO_MIXER_H

#include <tinyalsa/asoundlib.h>

#include <hardware/audio.h>
#include <system/audio.h>

#define TINYALSA_MIXER_CONFIG_FILE	"/system/etc/tinyalsa-audio.xml"

struct list_head {
	struct list_head *prev;
	struct list_head *next;

	void *data;
};

enum tinyalsa_mixer_data_type {
	MIXER_DATA_TYPE_CTRL,
	MIXER_DATA_TYPE_WRITE,
	MIXER_DATA_TYPE_MAX
};

struct tinyalsa_mixer_data {
	enum tinyalsa_mixer_data_type type;
	char *name;
	char *value;
	char *attr;
};

struct tinyalsa_mixer_device_props {
	audio_devices_t type;
};

struct tinyalsa_mixer_device {
	struct tinyalsa_mixer_device_props props;
	struct list_head *enable;
	struct list_head *disable;
};

struct tinyalsa_mixer_io_props {
	int card;
	int device;

	int rate;
	audio_channel_mask_t channel_mask;
	audio_format_t format;

	int period_size;
	int period_count;
};

struct tinyalsa_mixer_io {
	struct tinyalsa_mixer_io_props props;
	struct tinyalsa_mixer_device *device_current;
	struct list_head *devices;
	int state;
};

struct tinyalsa_mixer {
	struct tinyalsa_mixer_io output;
	struct tinyalsa_mixer_io input;
	struct tinyalsa_mixer_io modem;
	struct mixer *mixer;
};

enum tinyalsa_mixer_direction {
	TINYALSA_MIXER_DIRECTION_OUTPUT,
	TINYALSA_MIXER_DIRECTION_INPUT,
	TINYALSA_MIXER_DIRECTION_MODEM,
	TINYALSA_MIXER_DIRECTION_MAX
};

struct tinyalsa_mixer_config_data {
	struct tinyalsa_mixer *mixer;
	struct tinyalsa_mixer_io_props io_props;
	struct tinyalsa_mixer_device_props device_props;
	enum tinyalsa_mixer_direction direction;

	struct tinyalsa_mixer_device *device;
	struct list_head **list_start;
	struct list_head *list;
};

int tinyalsa_mixer_set_output_state(struct tinyalsa_mixer *mixer, int state);
int tinyalsa_mixer_set_input_state(struct tinyalsa_mixer *mixer, int state);
int tinyalsa_mixer_set_modem_state(struct tinyalsa_mixer *mixer, int state);

int tinyalsa_mixer_set_device(struct tinyalsa_mixer *mixer, audio_devices_t device);

int tinyalsa_mixer_set_output_volume(struct tinyalsa_mixer *mixer,
	audio_devices_t device, float volume);
int tinyalsa_mixer_set_master_volume(struct tinyalsa_mixer *mixer, float volume);
int tinyalsa_mixer_set_mic_mute(struct tinyalsa_mixer *mixer,
	audio_devices_t device, int mute);
int tinyalsa_mixer_set_input_gain(struct tinyalsa_mixer *mixer,
	audio_devices_t device, float gain);
int tinyalsa_mixer_set_voice_volume(struct tinyalsa_mixer *mixer,
	audio_devices_t device, float volume);

struct tinyalsa_mixer_io_props *tinyalsa_mixer_get_output_props(struct tinyalsa_mixer *mixer);
struct tinyalsa_mixer_io_props *tinyalsa_mixer_get_input_props(struct tinyalsa_mixer *mixer);
struct tinyalsa_mixer_io_props *tinyalsa_mixer_get_modem_props(struct tinyalsa_mixer *mixer);

void tinyalsa_mixer_close(struct tinyalsa_mixer *mixer);
int tinyalsa_mixer_open(struct tinyalsa_mixer **mixer_p, char *config_file);

#endif
