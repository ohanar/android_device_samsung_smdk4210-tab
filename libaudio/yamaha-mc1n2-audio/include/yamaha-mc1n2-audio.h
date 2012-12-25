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

#ifndef YAMAHA_MC1N2_AUDIO_H
#define YAMAHA_MC1N2_AUDIO_H

#include <system/audio.h>

#include "mc1n2.h"

enum yamaha_mc1n2_audio_direction {
	YAMAHA_MC1N2_AUDIO_DIRECTION_OUTPUT,
	YAMAHA_MC1N2_AUDIO_DIRECTION_INPUT,
	YAMAHA_MC1N2_AUDIO_DIRECTION_MODEM,
	YAMAHA_MC1N2_AUDIO_DIRECTION_MAX
};

struct yamaha_mc1n2_audio_params_init {
	MCDRV_AE_INFO ae_info;
	MCDRV_PATH_INFO path_info;
	MCDRV_DAC_INFO dac_info;
	MCDRV_ADC_INFO adc_info;
	MCDRV_SP_INFO sp_info;
	MCDRV_PDM_INFO pdm_info;
	MCDRV_DNG_INFO dng_info;
	MCDRV_SYSEQ_INFO syseq_info;
};

struct yamaha_mc1n2_audio_params_route {
	audio_devices_t device;
	enum yamaha_mc1n2_audio_direction direction;

	MCDRV_AE_INFO ae_info;
	MCDRV_PATH_INFO path_info;
	MCDRV_DAC_INFO dac_info;
};

struct yamaha_mc1n2_audio_params {
	struct yamaha_mc1n2_audio_params_init *init;
	struct yamaha_mc1n2_audio_params_route *routes;
	int routes_count;
};

struct yamaha_mc1n2_audio_device_ops {
	char *hw_node;
	int hw_fd;
	struct yamaha_mc1n2_audio_params params;
};

struct yamaha_mc1n2_audio_pdata {
	char *name;
	struct yamaha_mc1n2_audio_device_ops *ops;

	audio_devices_t output_device;
	audio_devices_t input_device;

	int output_state;
	int input_state;
	int modem_state;
};

/*
 * Platforms
 */

extern struct yamaha_mc1n2_audio_pdata smdk4210_pdata;

/*
 * Functions
 */

// IOCTL
int yamaha_mc1n2_audio_ioctl(struct yamaha_mc1n2_audio_pdata *pdata,
	int command, struct mc1n2_ctrl_args *hw_ctrl);
int yamaha_mc1n2_audio_ioctl_set_ctrl(struct yamaha_mc1n2_audio_pdata *pdata,
	unsigned long command, void *data, unsigned long update_info);
int yamaha_mc1n2_audio_ioctl_notify(struct yamaha_mc1n2_audio_pdata *pdata,
	unsigned long command);

// Routines
int yamaha_mc1n2_audio_init(struct yamaha_mc1n2_audio_pdata *pdata);
int yamaha_mc1n2_audio_output_start(struct yamaha_mc1n2_audio_pdata *pdata);
int yamaha_mc1n2_audio_output_stop(struct yamaha_mc1n2_audio_pdata *pdata);
int yamaha_mc1n2_audio_input_start(struct yamaha_mc1n2_audio_pdata *pdata);
int yamaha_mc1n2_audio_input_stop(struct yamaha_mc1n2_audio_pdata *pdata);
int yamaha_mc1n2_audio_modem_start(struct yamaha_mc1n2_audio_pdata *pdata);
int yamaha_mc1n2_audio_modem_stop(struct yamaha_mc1n2_audio_pdata *pdata);

// Values configuration
int yamaha_mc1n2_audio_set_route(struct yamaha_mc1n2_audio_pdata *pdata,
	audio_devices_t device);
char *yamaha_mc1n2_audio_get_hw_node(struct yamaha_mc1n2_audio_pdata *pdata);

// Init/Deinit
int yamaha_mc1n2_audio_start(struct yamaha_mc1n2_audio_pdata **pdata_p,
	char *device_name);
int yamaha_mc1n2_audio_stop(struct yamaha_mc1n2_audio_pdata *pdata);

#endif
