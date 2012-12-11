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

#define LOG_TAG "TinyALSA-Audio Mixer"

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#include <cutils/log.h>

#include <expat.h>

#define EFFECT_UUID_NULL EFFECT_UUID_NULL_MIXER
#define EFFECT_UUID_NULL_STR EFFECT_UUID_NULL_STR_MIXER
#include "audio_hw.h"
#include "mixer.h"

/*
 * List
 */

struct list_head *list_head_alloc(void)
{
	struct list_head *list = (struct list_head *)
		calloc(1, sizeof(struct list_head));

	return list;
}

void list_head_free(struct list_head *list)
{
	struct list_head *list_prev;

	while(list != NULL) {
		list_prev = list;
		list = list->next;

		free(list_prev);
	}
}

/*
 * Mixer data
 */

struct tinyalsa_mixer_data *tinyalsa_mixer_data_alloc(void)
{
	struct tinyalsa_mixer_data *mixer_data = (struct tinyalsa_mixer_data *)
		calloc(1, sizeof(struct tinyalsa_mixer_data));

	return mixer_data;
}

void tinyalsa_mixer_data_free(struct tinyalsa_mixer_data *mixer_data)
{
	if(mixer_data == NULL)
		return;

	if(mixer_data->name != NULL) {
		free(mixer_data->name);
		mixer_data->name = NULL;
	}

	if(mixer_data->value != NULL) {
		free(mixer_data->value);
		mixer_data->value = NULL;
	}

	if(mixer_data->attr != NULL) {
		free(mixer_data->attr);
		mixer_data->attr = NULL;
	}

	free(mixer_data);
}

struct tinyalsa_mixer_data *tinyalsa_mixer_get_data_with_attr(
	struct list_head *list_data, char *attr)
{
	struct tinyalsa_mixer_data *mixer_data = NULL;

	while(list_data != NULL) {
		mixer_data = (struct tinyalsa_mixer_data *) list_data->data;

		if(mixer_data->type == MIXER_DATA_TYPE_CTRL) {
			if(mixer_data->attr != NULL &&
				strcmp(mixer_data->attr, attr) == 0) {
				break;
			} else {
				mixer_data = NULL;
			}

		}

		if(list_data->next != NULL)
			list_data = list_data->next;
		else
			break;
	}

	return mixer_data;
}

/*
 * Mixer device
 */

struct tinyalsa_mixer_device *tinyalsa_mixer_device_alloc(void)
{
	struct tinyalsa_mixer_device *mixer_device = (struct tinyalsa_mixer_device *)
		calloc(1, sizeof(struct tinyalsa_mixer_device));

	return mixer_device;
}

void tinyalsa_mixer_device_free(struct tinyalsa_mixer_device *mixer_device)
{
	struct tinyalsa_mixer_data *mixer_data;
	struct list_head *list_data;
	struct list_head *list_prev;

	if(mixer_device == NULL)
		return;

	list_data = mixer_device->enable;

	while(list_data != NULL) {
		mixer_data = (struct tinyalsa_mixer_data *) list_data->data;

		tinyalsa_mixer_data_free(mixer_data);
		list_data->data = NULL;

		list_prev = list_data;
		list_data = list_data->next;

		list_head_free(list_prev);
	}

	list_data = mixer_device->disable;

	while(list_data != NULL) {
		mixer_data = (struct tinyalsa_mixer_data *) list_data->data;

		tinyalsa_mixer_data_free(mixer_data);
		list_data->data = NULL;

		list_prev = list_data;
		list_data = list_data->next;

		list_head_free(list_prev);
	}

	mixer_device->enable = NULL;
	mixer_device->disable = NULL;

	free(mixer_device);
}

struct tinyalsa_mixer_device *tinyalsa_mixer_get_device(struct tinyalsa_mixer_io *mixer_io,
	audio_devices_t device)
{
	struct tinyalsa_mixer_device *mixer_device = NULL;
	struct list_head *list = NULL;

	if(mixer_io == NULL)
		return NULL;

	list = mixer_io->devices;

	while(list != NULL) {
		mixer_device = (struct tinyalsa_mixer_device *) list->data;
		if(mixer_device != NULL && mixer_device->props.type == device) {
			break;
		} else {
			mixer_device = NULL;
		}

		list = list->next;
	}

	return mixer_device;
}

/*
 * Mixer I/O
 */

void tinyalsa_mixer_io_free_devices(struct tinyalsa_mixer_io *mixer_io)
{
	struct tinyalsa_mixer_device *mixer_device;
	struct list_head *list_device;
	struct list_head *list_prev;

	if(mixer_io == NULL)
		return;

	list_device = mixer_io->devices;

	while(list_device != NULL) {
		mixer_device = (struct tinyalsa_mixer_device *) list_device->data;

		tinyalsa_mixer_device_free(mixer_device);
		list_device->data = NULL;

		list_prev = list_device;
		list_device = list_device->next;

		list_head_free(list_prev);
	}
}

/*
 * Mixer config
 */

void tinyalsa_mixer_config_start(void *data, const XML_Char *elem,
	const XML_Char **attr)
{
	struct tinyalsa_mixer_config_data *config_data;
	struct tinyalsa_mixer_data *mixer_data;
	struct list_head *list;
	int i;

	if(data == NULL || elem == NULL || attr == NULL)
		return;

	config_data = (struct tinyalsa_mixer_config_data *) data;

	if(strcmp(elem, "tinyalsa-audio") == 0) {
		for(i=0 ; attr[i] != NULL && attr[i+1] != NULL ; i++) {
			if(strcmp(attr[i], "device") == 0) {
				i++;
				ALOGD("Parsing config for device: %s", attr[i]);
			}
		}
	} else if(strcmp(elem, "output") == 0) {
		config_data->direction = TINYALSA_MIXER_DIRECTION_OUTPUT;

		for(i=0 ; attr[i] != NULL && attr[i+1] ; i++) {
			if(strcmp(attr[i], "card") == 0) {
				i++;
				config_data->io_props.card = atoi(attr[i]);
			} else if(strcmp(attr[i], "device") == 0) {
				i++;
				config_data->io_props.device = atoi(attr[i]);
			} else if(strcmp(attr[i], "rate") == 0) {
				i++;
				config_data->io_props.rate = atoi(attr[i]);
			} else if(strcmp(attr[i], "channels") == 0) {
				i++;
				switch(atoi(attr[i])) {
					case 1:
						config_data->io_props.channel_mask = AUDIO_CHANNEL_OUT_MONO;
						break;
					case 2:
						config_data->io_props.channel_mask = AUDIO_CHANNEL_OUT_STEREO;
						break;
					case 4:
						config_data->io_props.channel_mask = AUDIO_CHANNEL_OUT_SURROUND;
						break;
					case 6:
						config_data->io_props.channel_mask = AUDIO_CHANNEL_OUT_5POINT1;
						break;
					case 8:
						config_data->io_props.channel_mask = AUDIO_CHANNEL_OUT_7POINT1;
						break;
					default:
						ALOGE("Unknown channel attr: %s", attr[i]);
						break;
				}
			} else if(strcmp(attr[i], "format") == 0) {
				i++;
				if(strcmp(attr[i], "PCM_8") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_8_BIT;
				} else if(strcmp(attr[i], "PCM_16") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_16_BIT;
				} else if(strcmp(attr[i], "PCM_32") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_32_BIT;
				} else if(strcmp(attr[i], "PCM_8_24") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_8_24_BIT;
				} else {
					ALOGE("Unknown format attr: %s", attr[i]);
				}
			} else if(strcmp(attr[i], "period_size") == 0) {
				i++;
				config_data->io_props.period_size = atoi(attr[i]);
			} else if(strcmp(attr[i], "period_count") == 0) {
				i++;
				config_data->io_props.period_count = atoi(attr[i]);
			} else {
				ALOGE("Unknown output attr: %s", attr[i]);
			}
		}
	} else if(strcmp(elem, "input") == 0) {
		config_data->direction = TINYALSA_MIXER_DIRECTION_INPUT;

		for(i=0 ; attr[i] != NULL && attr[i+1] ; i++) {
			if(strcmp(attr[i], "card") == 0) {
				i++;
				config_data->io_props.card = atoi(attr[i]);
			} else if(strcmp(attr[i], "device") == 0) {
				i++;
				config_data->io_props.device = atoi(attr[i]);
			} else if(strcmp(attr[i], "rate") == 0) {
				i++;
				config_data->io_props.rate = atoi(attr[i]);
			} else if(strcmp(attr[i], "channels") == 0) {
				i++;
				switch(atoi(attr[i])) {
					case 1:
						config_data->io_props.channel_mask = AUDIO_CHANNEL_IN_MONO;
						break;
					case 2:
						config_data->io_props.channel_mask = AUDIO_CHANNEL_IN_STEREO;
						break;
					default:
						ALOGE("Unknown channel attr: %s", attr[i]);
						break;
				}
			} else if(strcmp(attr[i], "format") == 0) {
				i++;
				if(strcmp(attr[i], "PCM_8") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_8_BIT;
				} else if(strcmp(attr[i], "PCM_16") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_16_BIT;
				} else if(strcmp(attr[i], "PCM_32") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_32_BIT;
				} else if(strcmp(attr[i], "PCM_8_24") == 0) {
					config_data->io_props.format = AUDIO_FORMAT_PCM_8_24_BIT;
				} else {
					ALOGE("Unknown format attr: %s", attr[i]);
				}
			} else if(strcmp(attr[i], "period_size") == 0) {
				i++;
				config_data->io_props.period_size = atoi(attr[i]);
			} else if(strcmp(attr[i], "period_count") == 0) {
				i++;
				config_data->io_props.period_count = atoi(attr[i]);
			} else {
				ALOGE("Unknown input attr: %s", attr[i]);
			}
		}
	} else if(strcmp(elem, "modem") == 0) {
		config_data->direction = TINYALSA_MIXER_DIRECTION_MODEM;

		for(i=0 ; attr[i] != NULL && attr[i+1] ; i++) {
			if(strcmp(attr[i], "card") == 0) {
				i++;
				config_data->io_props.card = atoi(attr[i]);
			} else if(strcmp(attr[i], "device") == 0) {
				i++;
				config_data->io_props.device = atoi(attr[i]);
			} else {
				ALOGE("Unknown modem attr: %s", attr[i]);
			}
		}
	} else if(strcmp(elem, "device") == 0) {
		for(i=0 ; attr[i] != NULL && attr[i+1] != NULL ; i++) {
			if(strcmp(attr[i], "type") == 0) {
				i++;
				if(config_data->direction == TINYALSA_MIXER_DIRECTION_OUTPUT ||
					config_data->direction == TINYALSA_MIXER_DIRECTION_MODEM) {
					if(strcmp(attr[i], "default") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_DEFAULT;
					} else if(strcmp(attr[i], "earpiece") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_EARPIECE;
					} else if(strcmp(attr[i], "speaker") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_SPEAKER;
					} else if(strcmp(attr[i], "wired-headset") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_WIRED_HEADSET;
					} else if(strcmp(attr[i], "wired-headphone") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
					} else if(strcmp(attr[i], "bt-sco") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_BLUETOOTH_SCO;
					} else if(strcmp(attr[i], "bt-sco-headset") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET;
					} else if(strcmp(attr[i], "bt-sco-carkit") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT;
					} else if(strcmp(attr[i], "bt-a2dp") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP;
					} else if(strcmp(attr[i], "bt-a2dp-headphones") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES;
					} else if(strcmp(attr[i], "bt-a2dp-speaker") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER;
					} else if(strcmp(attr[i], "aux-digital") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_AUX_DIGITAL;
					} else if(strcmp(attr[i], "analog-dock-headset") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET;
					} else if(strcmp(attr[i], "digital-dock-headset") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET;
/*
 * There is now FM support for Qcom devices.  Now that Qcom FM is working,
 * it may be possible to get Samsung FM working...  SAMSUNG_FM_ENABLED
 * is a temp placeholder for the ifdef.  See QCOM_FM_ENABLED for an example
 * in system/core/include/system/audio.h
 */
#ifdef SAMSUNG_FM_ENABLED
					} else if(strcmp(attr[i], "fm") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_OUT_FM;
#endif
					} else {
						ALOGE("Unknown device attr: %s", attr[i]);
					}
				} else if(config_data->direction == TINYALSA_MIXER_DIRECTION_INPUT) {
					if(strcmp(attr[i], "default") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_DEFAULT;
					} else if(strcmp(attr[i], "communication") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_COMMUNICATION;
					} else if(strcmp(attr[i], "ambient") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_AMBIENT;
					} else if(strcmp(attr[i], "builtin-mic") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_BUILTIN_MIC;
					} else if(strcmp(attr[i], "bt-sco-headset") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET;
					} else if(strcmp(attr[i], "wired-headset") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_WIRED_HEADSET;
					} else if(strcmp(attr[i], "aux-digital") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_AUX_DIGITAL;
					} else if(strcmp(attr[i], "voice-call") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_VOICE_CALL;
					} else if(strcmp(attr[i], "back-mic") == 0) {
						config_data->device_props.type = AUDIO_DEVICE_IN_BACK_MIC;
					} else {
						ALOGE("Unknown device attr: %s", attr[i]);
					}
				}
			} else {
				ALOGE("Unknown device attr: %s", attr[i]);
			}

			if(config_data->device_props.type != 0) {
				config_data->device = tinyalsa_mixer_device_alloc();
				memcpy(&config_data->device->props, &config_data->device_props, sizeof(config_data->device_props));
			} else {
				ALOGE("Missing attrs for elem: %s", elem);
			}
		}
	} else if(strcmp(elem, "path") == 0) {
		for(i=0 ; attr[i] != NULL && attr[i+1] != NULL ; i++) {
			if(strcmp(attr[i], "type") == 0) {
				i++;
				if(strcmp(attr[i], "enable") == 0) {
					config_data->list_start = &config_data->device->enable;
				} else if(strcmp(attr[i], "disable") == 0) {
					config_data->list_start = &config_data->device->disable;
				} else {
					ALOGE("Unknown path attr: %s", attr[i]);
				}
			} else {
				ALOGE("Unknown path attr: %s", attr[i]);
			}
		}
	} else if(strcmp(elem, "ctrl") == 0) {
		if(config_data->device != NULL && config_data->list_start != NULL) {
			list = list_head_alloc();
			mixer_data = tinyalsa_mixer_data_alloc();

			mixer_data->type = MIXER_DATA_TYPE_CTRL;
			list->data = (void *) mixer_data;
		} else {
			ALOGE("Missing device/path for elem: %s", elem);
			return;
		}

		for(i=0 ; attr[i] != NULL && attr[i+1] != NULL ; i++) {
			if(strcmp(attr[i], "name") == 0) {
				i++;
				mixer_data->name = strdup((char *) attr[i]);
			} else if(strcmp(attr[i], "attr") == 0) {
				i++;
				mixer_data->attr = strdup((char *) attr[i]);
			} else if(strcmp(attr[i], "value") == 0) {
				i++;
				mixer_data->value = strdup((char *) attr[i]);
			} else {
				ALOGE("Unknown ctrl attr: %s", attr[i]);
			}
		}

		if(mixer_data->name != NULL && mixer_data->value != NULL) {
			if(*config_data->list_start == NULL) {
				*config_data->list_start = list;
			} else {
				config_data->list->next = list;
				list->prev = config_data->list;
			}

			config_data->list = list;
		} else {
			tinyalsa_mixer_data_free(mixer_data);
			list_head_free(list);
		}
	}
}

void tinyalsa_mixer_config_end(void *data, const XML_Char *elem)
{
	struct tinyalsa_mixer_config_data *config_data;
	struct list_head *list_prev;
	struct list_head *list;

	if(data == NULL || elem == NULL)
		return;

	config_data = (struct tinyalsa_mixer_config_data *) data;

	if(strcmp(elem, "output") == 0) {
		memcpy(&config_data->mixer->output.props, &config_data->io_props, sizeof(config_data->io_props));
		memset(&config_data->io_props, 0, sizeof(config_data->io_props));
		config_data->direction = 0;
	} else if(strcmp(elem, "input") == 0) {
		memcpy(&config_data->mixer->input.props, &config_data->io_props, sizeof(config_data->io_props));
		memset(&config_data->io_props, 0, sizeof(config_data->io_props));
		config_data->direction = 0;
	} else if(strcmp(elem, "modem") == 0) {
		memcpy(&config_data->mixer->modem.props, &config_data->io_props, sizeof(config_data->io_props));
		memset(&config_data->io_props, 0, sizeof(config_data->io_props));
		config_data->direction = 0;
	} else if(strcmp(elem, "device") == 0) {
		// direction == 0 will fallback to out
		if(config_data->direction == TINYALSA_MIXER_DIRECTION_OUTPUT) {
			list = list_head_alloc();
			list->data = (void *) config_data->device;

			if(config_data->mixer->output.devices == NULL) {
				config_data->mixer->output.devices = list;
			} else {
				list_prev = config_data->mixer->output.devices;

				while(list_prev->next != NULL)
					list_prev = list_prev->next;

				list_prev->next = list;
				list->prev = list_prev;
			}
		} else if(config_data->direction == TINYALSA_MIXER_DIRECTION_INPUT) {
			list = list_head_alloc();
			list->data = (void *) config_data->device;

			if(config_data->mixer->input.devices == NULL) {
				config_data->mixer->input.devices = list;
			} else {
				list_prev = config_data->mixer->input.devices;

				while(list_prev->next != NULL)
					list_prev = list_prev->next;

				list_prev->next = list;
				list->prev = list_prev;
			}
		} else if(config_data->direction == TINYALSA_MIXER_DIRECTION_MODEM) {
			list = list_head_alloc();
			list->data = (void *) config_data->device;

			if(config_data->mixer->modem.devices == NULL) {
				config_data->mixer->modem.devices = list;
			} else {
				list_prev = config_data->mixer->modem.devices;

				while(list_prev->next != NULL)
					list_prev = list_prev->next;

				list_prev->next = list;
				list->prev = list_prev;
			}
		}

		config_data->device = NULL;
		config_data->list = NULL;
	} else if(strcmp(elem, "path") == 0) {
		config_data->list_start = 0;
		config_data->list = 0;
	}
}

int tinyalsa_mixer_config_parse(struct tinyalsa_mixer *mixer, char *config_file)
{
	struct tinyalsa_mixer_config_data config_data;
	char buf[80];
	XML_Parser p;
	FILE *f;

	int eof = 0;
	int len = 0;

	if(mixer == NULL || config_file == NULL)
		return -1;

	f = fopen(config_file, "r");
	if(!f) {
		ALOGE("Failed to open tinyalsa-audio config file!");
		return -1;
	}

	p = XML_ParserCreate(NULL);
	if(!p) {
		ALOGE("Failed to create XML parser!");
		goto error_file;
	}

	memset(&config_data, 0, sizeof(config_data));
	config_data.mixer = mixer;

	XML_SetUserData(p, &config_data);
	XML_SetElementHandler(p, tinyalsa_mixer_config_start, tinyalsa_mixer_config_end);

	while(!eof) {
		len = fread(buf, 1, sizeof(buf), f);
		if(ferror(f)) {
			ALOGE("Failed to read config file!");
			goto error_xml_parser;
		}

		eof = feof(f);

		if(XML_Parse(p, buf, len, eof) == XML_STATUS_ERROR) {
			ALOGE("Failed to parse line %d: %s",
				(int) XML_GetCurrentLineNumber(p),
				(char *) XML_ErrorString(XML_GetErrorCode(p)));
			goto error_xml_parser;
		}
	}

	XML_ParserFree(p);
	fclose(f);

	return 0;

error_xml_parser:
	XML_ParserFree(p);

error_file:
	fclose(f);

	return -1;
}

/*
 * Route/Directions
 */

int tinyalsa_mixer_set_route_ctrl(struct tinyalsa_mixer *mixer,
	struct tinyalsa_mixer_data *mixer_data)
{
	struct mixer_ctl *ctl;
	int value = 0;
	int type;
	int rc;
	int i;

	if(mixer_data->type != MIXER_DATA_TYPE_CTRL)
		return -1;

	ctl = mixer_get_ctl_by_name(mixer->mixer, mixer_data->name);
	type = mixer_ctl_get_type(ctl);

	ALOGD("Setting %s to %s", mixer_data->name, mixer_data->value);

	switch(type) {
		case MIXER_CTL_TYPE_BOOL:
			value = strcmp(mixer_data->value, "on") == 0 ?
				1 : 0;
			break;
		case MIXER_CTL_TYPE_INT:
			value = atoi(mixer_data->value);
			break;
		case MIXER_CTL_TYPE_BYTE:
			value = atoi(mixer_data->value) & 0xff;
			break;
	}

	if(type == MIXER_CTL_TYPE_BOOL || type == MIXER_CTL_TYPE_INT ||
		type == MIXER_CTL_TYPE_BYTE) {
		for(i=0 ; i < mixer_ctl_get_num_values(ctl) ; i++) {
			rc = mixer_ctl_set_value(ctl, i, value);
			if(rc < 0)
				return -1;
		}
	} else if(type == MIXER_CTL_TYPE_ENUM || type == MIXER_CTL_TYPE_UNKNOWN) {
		rc = mixer_ctl_set_enum_by_string(ctl, mixer_data->value);
		if(rc < 0)
			return -1;
	}

	return 0;
}

int tinyalsa_mixer_set_route_list(struct tinyalsa_mixer *mixer, struct list_head *list)
{
	struct tinyalsa_mixer_data *mixer_data = NULL;
	int rc;

	if(mixer == NULL || mixer->mixer == NULL)
		return -1;

	while(list != NULL) {
		mixer_data = (struct tinyalsa_mixer_data *) list->data;

		if(mixer_data->type == MIXER_DATA_TYPE_CTRL) {
			if(mixer_data->attr != NULL &&
				strcmp(mixer_data->attr, "voice-volume") == 0) {
				ALOGD("Skipping voice volume control");
			} else {
				rc = tinyalsa_mixer_set_route_ctrl(mixer, mixer_data);
				if(rc < 0) {
					ALOGE("Unable to set control!");
					return -1;
				}
			}
		}

		if(list->next != NULL)
			list = list->next;
		else
			break;
	}

	return 0;
}

int tinyalsa_mixer_set_route(struct tinyalsa_mixer *mixer,
	struct tinyalsa_mixer_io *mixer_io, audio_devices_t device)
{
	struct tinyalsa_mixer_device *mixer_device = NULL;
	struct list_head *list = NULL;
	int rc;

	if(mixer == NULL || mixer_io == NULL)
		return -1;

	ALOGD("%s(card=%d,device=%d)++",__func__,mixer_io->props.card,device);

	mixer->mixer = mixer_open(mixer_io->props.card);
	if(mixer->mixer == NULL) {
		ALOGE("Unable to open mixer for card: %d", mixer_io->props.card);
		return -1;
	}


	mixer_device = tinyalsa_mixer_get_device(mixer_io, device);
	if(mixer_device == NULL) {
		ALOGE("Unable to find a matching device: 0x%x", device);
		goto error_mixer;
	}

	// No need to disable and enable the same route
	if(mixer_device == mixer_io->device_current)
		goto exit_mixer;

	if(mixer_io->device_current != NULL) {
		rc = tinyalsa_mixer_set_route_list(mixer, mixer_io->device_current->disable);
		if(rc < 0) {
			ALOGE("Unable to disable current device controls");
			goto error_mixer;
		}
	}

	rc = tinyalsa_mixer_set_route_list(mixer, mixer_device->enable);
	if(rc < 0) {
		ALOGE("Unable to enable device controls");
		goto error_mixer;
	}

	mixer_io->device_current = mixer_device;

exit_mixer:
	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(card=%d,device=%d)--",__func__,mixer_io->props.card,device);

	return 0;

error_mixer:
	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(card=%d,device=%d)-- (MIXER ERROR)",__func__,mixer_io->props.card,device);

	return -1;
}

int tinyalsa_mixer_set_device_volume_with_attr(struct tinyalsa_mixer *mixer,
	enum tinyalsa_mixer_direction direction, audio_devices_t device,
	char *attr, float volume)
{
	struct tinyalsa_mixer_io *mixer_io = NULL;
	struct tinyalsa_mixer_device *mixer_device = NULL;
	struct tinyalsa_mixer_data *mixer_data = NULL;
	struct list_head *list = NULL;
	int value, value_min, value_max, values_count;
	char *value_string = NULL;
	int rc;

	if(mixer == NULL || attr == NULL)
		return -1;

	ALOGD("%s(direction=%d, device=%d, attr=%s, volume=%f)++",__func__,direction,device,attr,volume);

	switch(direction) {
		case TINYALSA_MIXER_DIRECTION_OUTPUT:
			mixer_io = &mixer->output;
			break;
		case TINYALSA_MIXER_DIRECTION_INPUT:
			mixer_io = &mixer->input;
			break;
		case TINYALSA_MIXER_DIRECTION_MODEM:
			mixer_io = &mixer->modem;
			break;
		default:
			ALOGE("Invalid diretion: 0x%x", direction);
			return -1;
	}

	if(!mixer_io->state) {
		ALOGE("Unable to set device for the asked direction: state is %d", mixer_io->state);
		return -1;
	}

	mixer->mixer = mixer_open(mixer_io->props.card);
	if(mixer->mixer == NULL) {
		ALOGE("Unable to open mixer for card: %d", mixer_io->props.card);
		return -1;
	}

	mixer_device = tinyalsa_mixer_get_device(mixer_io, device);
	if(mixer_device == NULL) {
		ALOGE("Unable to find a matching device: 0x%x", device);
		goto error_mixer;
	}

	list = mixer_device->enable;

	mixer_data = tinyalsa_mixer_get_data_with_attr(list, attr);
	if(mixer_data == NULL) {
		ALOGE("Unable to find a matching ctrl with attr: %s", attr);
		goto error_mixer;
	}

	if(mixer_data->value == NULL) {
		ALOGE("Missing mixer data value!");
		goto error_mixer;
	}

	values_count = sscanf(mixer_data->value, "%d-%d", &value_min, &value_max);
	if(values_count != 2) {
		ALOGE("Failed to get mixer data value!");
		goto error_mixer;
	}

	value = (value_max - value_min) * volume + value_min;

	// Ugly workaround because a string value is needed
	value_string = mixer_data->value;
	asprintf(&mixer_data->value, "%d", value);

	rc = tinyalsa_mixer_set_route_ctrl(mixer, mixer_data);
	if(rc < 0) {
		ALOGE("Unable to set ctrl!");
		goto error_data;
	}

	free(mixer_data->value);
	mixer_data->value = value_string;

	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(direction=%d, device=%d, attr=%s, volume=%f)--",__func__,direction,device,attr,volume);

	return 0;

error_data:
	free(mixer_data->value);
	mixer_data->value = value_string;

	ALOGD("%s(direction=%d, device=%d, attr=%s, volume=%f)-- (DATA ERROR)",__func__,direction,device,attr,volume);

error_mixer:
	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(direction=%d, device=%d, attr=%s, volume=%f)-- (MIXER ERROR)",__func__,direction,device,attr,volume);

	return -1;
}

int tinyalsa_mixer_set_device_state_with_attr(struct tinyalsa_mixer *mixer,
	enum tinyalsa_mixer_direction direction, audio_devices_t device,
	char *attr, int state)
{
	struct tinyalsa_mixer_io *mixer_io = NULL;
	struct tinyalsa_mixer_device *mixer_device = NULL;
	struct tinyalsa_mixer_data *mixer_data = NULL;
	struct list_head *list = NULL;
	int rc;

	if(mixer == NULL || attr == NULL)
		return -1;

	state = state >= 1 ? 1 : 0;

	ALOGD("%s(direction=%d, device=%d, attr=%s, state=%d)++",__func__,direction,device,attr,state);

	switch(direction) {
		case TINYALSA_MIXER_DIRECTION_OUTPUT:
			mixer_io = &mixer->output;
			break;
		case TINYALSA_MIXER_DIRECTION_INPUT:
			mixer_io = &mixer->input;
			break;
		case TINYALSA_MIXER_DIRECTION_MODEM:
			mixer_io = &mixer->modem;
			break;
		default:
			ALOGE("Invalid diretion: 0x%x", direction);
			return -1;
	}

	if(!mixer_io->state) {
		ALOGE("Unable to set device for the asked direction: state is %d", mixer_io->state);
		return -1;
	}

	mixer->mixer = mixer_open(mixer_io->props.card);
	if(mixer->mixer == NULL) {
		ALOGE("Unable to open mixer for card: %d", mixer_io->props.card);
		return -1;
	}

	mixer_device = tinyalsa_mixer_get_device(mixer_io, device);
	if(mixer_device == NULL) {
		ALOGE("Unable to find a matching device: 0x%x", device);
		goto error_mixer;
	}

	if(state)
		list = mixer_device->enable;
	else
		list = mixer_device->disable;

	mixer_data = tinyalsa_mixer_get_data_with_attr(list, attr);
	if(mixer_data == NULL) {
		ALOGE("Unable to find a matching ctrl with attr: %s", attr);
		goto error_mixer;
	}

	rc = tinyalsa_mixer_set_route_ctrl(mixer, mixer_data);
	if(rc < 0) {
		ALOGE("Unable to set ctrl!");
		goto error_mixer;
	}

	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(direction=%d, device=%d, attr=%s, state=%d)--",__func__,direction,device,attr,state);

	return 0;

error_mixer:
	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(direction=%d, device=%d, attr=%s, state=%d)-- (MIXER ERROR)",__func__,direction,device,attr,state);

	return -1;
}

int tinyalsa_mixer_set_state(struct tinyalsa_mixer *mixer,
	enum tinyalsa_mixer_direction direction, int state)
{
	struct tinyalsa_mixer_io *mixer_io = NULL;
	struct tinyalsa_mixer_device *mixer_device = NULL;
	struct list_head *list;
	audio_devices_t default_device;
	int rc;

	if(mixer == NULL)
		return -1;

	state = state >= 1 ? 1 : 0;

	ALOGD("%s(direction=%d, state=%d)++",__func__,direction,state);

	switch(direction) {
		case TINYALSA_MIXER_DIRECTION_OUTPUT:
			mixer_io = &mixer->output;
			default_device = AUDIO_DEVICE_OUT_DEFAULT;
			break;
		case TINYALSA_MIXER_DIRECTION_INPUT:
			mixer_io = &mixer->input;
			default_device = AUDIO_DEVICE_IN_DEFAULT;
			break;
		case TINYALSA_MIXER_DIRECTION_MODEM:
			mixer_io = &mixer->modem;
			default_device = AUDIO_DEVICE_OUT_DEFAULT;
			break;
		default:
			ALOGE("Invalid diretion: 0x%x", direction);
			return -1;
	}

	if(mixer_io->state == state) {
		ALOGD("Current state is already: %d", state);
		return 0;
	}

	mixer->mixer = mixer_open(mixer_io->props.card);
	if(mixer->mixer == NULL) {
		ALOGE("Unable to open mixer for card: %d", mixer_io->props.card);
		return -1;
	}

	if(!state && mixer_io->device_current != NULL &&
		mixer_io->device_current->disable != NULL) {
		rc = tinyalsa_mixer_set_route_list(mixer, mixer_io->device_current->disable);
		if(rc < 0) {
			ALOGE("Unable to disable current device controls");
			goto error_mixer;
		}
	}

	mixer_device = tinyalsa_mixer_get_device(mixer_io, default_device);
	if(mixer_device == NULL) {
		ALOGD("Unable to find default device");
		// This is not really an issue
	}

	if(state && mixer_device != NULL && mixer_device->enable != NULL) {
		rc = tinyalsa_mixer_set_route_list(mixer, mixer_device->enable);
		if(rc < 0) {
			ALOGE("Unable to enable default device controls");
			goto error_mixer;
		}
	} else if(!state && mixer_device != NULL) {
		rc = tinyalsa_mixer_set_route_list(mixer, mixer_device->disable);
		if(rc < 0) {
			ALOGE("Unable to disable default device controls");
			goto error_mixer;
		}
	}

	mixer_io->device_current = NULL;
	mixer_io->state = state;

	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(direction=%d, state=%d)--",__func__,direction,state);

	return 0;

error_mixer:
	mixer_close(mixer->mixer);
	mixer->mixer = NULL;

	ALOGD("%s(direction=%d, state=%d)-- (MIXER ERROR)",__func__,direction,state);

	return -1;
}

/*
 * Interface
 */

int tinyalsa_mixer_set_output_state(struct tinyalsa_mixer *mixer, int state)
{
	ALOGD("%s(%d)", __func__, state);

	return tinyalsa_mixer_set_state(mixer, TINYALSA_MIXER_DIRECTION_OUTPUT, state);
}

int tinyalsa_mixer_set_input_state(struct tinyalsa_mixer *mixer, int state)
{
	ALOGD("%s(%d)", __func__, state);

	return tinyalsa_mixer_set_state(mixer, TINYALSA_MIXER_DIRECTION_INPUT, state);
}

int tinyalsa_mixer_set_modem_state(struct tinyalsa_mixer *mixer, int state)
{
	ALOGD("%s(%d)", __func__, state);

	return tinyalsa_mixer_set_state(mixer, TINYALSA_MIXER_DIRECTION_MODEM, state);
}

int tinyalsa_mixer_set_device(struct tinyalsa_mixer *mixer, audio_devices_t device)
{
	int rc;

	ALOGD("%s(%x)", __func__, device);

	if(mixer == NULL)
		return -1;

	if(!audio_is_output_device(device) && !audio_is_input_device(device)) {
		ALOGE("Invalid device: 0x%x", device);
		return -1;
	}


	if(audio_is_output_device(device) && mixer->output.state) {
		rc = tinyalsa_mixer_set_route(mixer, &mixer->output, device);
		if(rc < 0) {
			ALOGE("Unable to set route for output device: 0x%x", device);
			return -1;
		}
	}

	if(audio_is_input_device(device) && mixer->input.state) {
		rc = tinyalsa_mixer_set_route(mixer, &mixer->input, device);
		if(rc < 0) {
			ALOGE("Unable to set route for input device: 0x%x", device);
			return -1;
		}
	}

	if(audio_is_output_device(device) && mixer->modem.state) {
		rc = tinyalsa_mixer_set_route(mixer, &mixer->modem, device);
		if(rc < 0) {
			ALOGE("Unable to set route for modem device: 0x%x", device);
			return -1;
		}
	}

	return 0;
}

int tinyalsa_mixer_set_output_volume(struct tinyalsa_mixer *mixer,
	audio_devices_t device, float volume)
{
	ALOGD("%s(%p, %x, %f)", __func__, mixer, device, volume);

	return tinyalsa_mixer_set_device_volume_with_attr(mixer,
		TINYALSA_MIXER_DIRECTION_OUTPUT, device,
		"output-volume", volume);
}

int tinyalsa_mixer_set_master_volume(struct tinyalsa_mixer *mixer, float volume)
{
	ALOGD("%s(%p, %f)", __func__, mixer, volume);

	return tinyalsa_mixer_set_device_volume_with_attr(mixer,
		TINYALSA_MIXER_DIRECTION_OUTPUT, AUDIO_DEVICE_OUT_DEFAULT, 
		"master-volume", volume);
}

int tinyalsa_mixer_set_mic_mute(struct tinyalsa_mixer *mixer,
	audio_devices_t device, int mute)
{
	ALOGD("%s(%p, %x, %d)", __func__, mixer, device, mute);

	// Mic mute can be set for both input and modem directions
	if(audio_is_input_device(device)) {
		return tinyalsa_mixer_set_device_state_with_attr(mixer,
			TINYALSA_MIXER_DIRECTION_INPUT, device,
			"mic-mute", mute);
	} else if(audio_is_output_device(device)) {
		return tinyalsa_mixer_set_device_state_with_attr(mixer,
			TINYALSA_MIXER_DIRECTION_MODEM, device,
			"mic-mute", mute);
	} else {
		return -1;
	}
}

int tinyalsa_mixer_set_input_gain(struct tinyalsa_mixer *mixer,
	audio_devices_t device, float gain)
{
	ALOGD("%s(%p, %x, %f)", __func__, mixer, device, gain);

	return tinyalsa_mixer_set_device_volume_with_attr(mixer,
		TINYALSA_MIXER_DIRECTION_INPUT, device,
		"input-gain", gain);
}

int tinyalsa_mixer_set_voice_volume(struct tinyalsa_mixer *mixer,
	audio_devices_t device, float volume)
{
	ALOGD("%s(%p, %x, %f)", __func__, mixer, device, volume);

	return tinyalsa_mixer_set_device_volume_with_attr(mixer,
		TINYALSA_MIXER_DIRECTION_MODEM, device,
		"voice-volume", volume);
}

struct tinyalsa_mixer_io_props *tinyalsa_mixer_get_output_props(struct tinyalsa_mixer *mixer)
{
	ALOGD("%s(%p)", __func__, mixer);

	return &(mixer->output.props);
}

struct tinyalsa_mixer_io_props *tinyalsa_mixer_get_input_props(struct tinyalsa_mixer *mixer)
{
	ALOGD("%s(%p)", __func__, mixer);

	return &(mixer->input.props);
}

struct tinyalsa_mixer_io_props *tinyalsa_mixer_get_modem_props(struct tinyalsa_mixer *mixer)
{
	ALOGD("%s(%p)", __func__, mixer);

	return &(mixer->modem.props);
}

void tinyalsa_mixer_close(struct tinyalsa_mixer *mixer)
{
	ALOGD("%s(%p)", __func__, mixer);

	if(mixer == NULL)
		return;

	tinyalsa_mixer_set_output_state(mixer, 0);
	tinyalsa_mixer_set_input_state(mixer, 0);
	tinyalsa_mixer_set_modem_state(mixer, 0);

	tinyalsa_mixer_io_free_devices(&mixer->output);
	tinyalsa_mixer_io_free_devices(&mixer->input);
	tinyalsa_mixer_io_free_devices(&mixer->modem);

	free(mixer);
}

int tinyalsa_mixer_open(struct tinyalsa_mixer **mixer_p, char *config_file)
{
	struct tinyalsa_mixer *mixer = NULL;
	int rc;

	ALOGD("%s(%p, %s)", __func__, mixer_p, config_file);

	if(mixer_p == NULL || config_file == NULL)
		return -1;

	mixer = calloc(1, sizeof(struct tinyalsa_mixer));

	rc = tinyalsa_mixer_config_parse(mixer, config_file);
	if(rc < 0) {
		ALOGE("Unable to parse mixer config!");
		goto error_mixer;
	}

	*mixer_p = mixer;

	return 0;

error_mixer:
	*mixer_p = NULL;

	free(mixer);

	return -1;
}
