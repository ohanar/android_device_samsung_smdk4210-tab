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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/ioctl.h>

#define LOG_TAG "Yamaha-MC1N2-Audio"
#include <cutils/log.h>

#include <system/audio.h>

#include <yamaha-mc1n2-audio.h>

struct yamaha_mc1n2_audio_pdata *yamaha_mc1n2_audio_platforms[] = {
	&smdk4210_pdata,
};

int yamaha_mc1n2_audio_platforms_count = sizeof(yamaha_mc1n2_audio_platforms) /
	sizeof(struct yamaha_mc1n2_audio_pdata *);

/*
 * IOCTL
 */

int yamaha_mc1n2_audio_ioctl(struct yamaha_mc1n2_audio_pdata *pdata,
	int command, struct mc1n2_ctrl_args *hw_ctrl)
{
	char *hw_node = NULL;
	int hw_fd = -1;
	int rc = -1;

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	hw_node = pdata->ops->hw_node;
	if(hw_node == NULL) {
		ALOGE("%s: error, missing hw_node!", __func__);
		return -1;
	}

	if(pdata->ops->hw_fd <= 0) {
		hw_fd = open(hw_node, O_RDWR);
		if(hw_fd < 0) {
			ALOGE("%s: error, unable to open hw_node (fd is %d)!", __func__, hw_fd);
			return -1;
		}

		pdata->ops->hw_fd = hw_fd;
	}

	rc = ioctl(pdata->ops->hw_fd, command, hw_ctrl);
	if(rc < 0) {
		ALOGE("%s: error, ioctl on hw_node failed (rc is %d)!", __func__, rc);
		return -1;
	}

	return rc;
}

int yamaha_mc1n2_audio_ioctl_set_ctrl(struct yamaha_mc1n2_audio_pdata *pdata,
	unsigned long command, void *data, unsigned long update_info)
{
	struct mc1n2_ctrl_args hw_ctrl;

	if(pdata == NULL)
		return -1;

	memset(&hw_ctrl, 0, sizeof(hw_ctrl));
	hw_ctrl.dCmd = command;
	hw_ctrl.pvPrm = data;
	hw_ctrl.dPrm = update_info;

	return yamaha_mc1n2_audio_ioctl(pdata, MC1N2_IOCTL_SET_CTRL, &hw_ctrl);
}

int yamaha_mc1n2_audio_ioctl_notify(struct yamaha_mc1n2_audio_pdata *pdata,
	unsigned long command)
{
	struct mc1n2_ctrl_args hw_ctrl;

	if(pdata == NULL)
		return -1;

	memset(&hw_ctrl, 0, sizeof(hw_ctrl));
	hw_ctrl.dCmd = command;

	return yamaha_mc1n2_audio_ioctl(pdata, MC1N2_IOCTL_NOTIFY, &hw_ctrl);
}

/*
 * Routines
 */

int yamaha_mc1n2_audio_init(struct yamaha_mc1n2_audio_pdata *pdata)
{
	struct yamaha_mc1n2_audio_params_init *params = NULL;
	int rc = -1;

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	params = pdata->ops->params.init;
	if(params == NULL)
		return -1;

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_DAC,
		&params->dac_info, 0x07);
	if(rc < 0) {
		ALOGE("SET_DAC IOCTL failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_ADC,
		&params->adc_info, 0x07);
	if(rc < 0) {
		ALOGE("SET_ADC IOCTL failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_SP,
		&params->sp_info, 0x00);
	if(rc < 0) {
		ALOGE("SET_SP IOCTL failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_PDM,
		&params->pdm_info, 0x7f);
	if(rc < 0) {
		ALOGE("SET_PDM IOCTL failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_DNG,
		&params->dng_info, 0x3f3f3f);
	if(rc < 0) {
		ALOGE("SET_DNG IOCTL failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_SYSEQ,
		&params->syseq_info, 0x03);
	if(rc < 0) {
		ALOGE("SET_SYSEQ IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

struct yamaha_mc1n2_audio_params_route *
	yamaha_mc1n2_audio_params_route_find(struct yamaha_mc1n2_audio_pdata *pdata,
	audio_devices_t device, enum yamaha_mc1n2_audio_direction direction)
{
	struct yamaha_mc1n2_audio_params_route *params = NULL;
	int params_count = 0;
	int i;

	if(pdata == NULL || pdata->ops == NULL)
		return NULL;

	ALOGD("(%s): device = %d, direction = %d",__func__,device,direction);

	params = pdata->ops->params.routes;
	params_count = pdata->ops->params.routes_count;
	if(params == NULL || params_count <= 0)
		return NULL;

	for(i=0 ; i < params_count ; i++) {
		if(params[i].device == device && params[i].direction == direction)
			return &params[i];
	}

	return NULL;
}

int yamaha_mc1n2_audio_params_route_simple_array_merge(int length,
	unsigned char *array_src, unsigned char *array_dst)
{
	int i;

	if(length <= 0 || array_src == NULL || array_dst == NULL)
		return -1;

	for(i=0 ; i < length ; i++) {
		if(array_dst[i] == 0)
			array_dst[i] = array_src[i];
	}

	return 0;
}

int yamaha_mc1n2_audio_params_route_path_array_merge(int length,
	unsigned char *array_src, unsigned char *array_dst)
{
	int i, j;
	unsigned char v;

	if(length <= 0 || array_src == NULL || array_dst == NULL)
		return -1;

	for(i=0 ; i < length ; i++) {
		if(array_dst[i] == 0) {
			array_dst[i] = array_src[i];
		} else {
			v = array_src[i];

			for(j=0; j < 8 ; j++) {
				if((1 << j) & array_dst[i]) {
					if(j % 2 == 0)
						v &= ~(1 << (j+1));
					else
						v &= ~(1 << (j-1));

					v |= (1 << j);
				}
			}

			array_dst[i] = v;
		}
	}

	return 0;
}

int yamaha_mc1n2_audio_params_route_merge(
	struct yamaha_mc1n2_audio_params_route *params_src,
	struct yamaha_mc1n2_audio_params_route *params_dst)
{
	if(params_src == NULL || params_dst == NULL)
		return -1;

	// ae_info
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->ae_info.bOnOff),
		&params_src->ae_info.bOnOff, &params_dst->ae_info.bOnOff);
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->ae_info.abBex),
		params_src->ae_info.abBex, params_dst->ae_info.abBex);
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->ae_info.abWide),
		params_src->ae_info.abWide, params_dst->ae_info.abWide);
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->ae_info.abDrc),
		params_src->ae_info.abDrc, params_dst->ae_info.abDrc);
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->ae_info.abEq5),
		params_src->ae_info.abEq5, params_dst->ae_info.abEq5);
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->ae_info.abEq3),
		params_src->ae_info.abEq3, params_dst->ae_info.abEq3);

	// path_info
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asHpOut[0].abSrcOnOff),
		params_src->path_info.asHpOut[0].abSrcOnOff, params_dst->path_info.asHpOut[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asHpOut[1].abSrcOnOff),
		params_src->path_info.asHpOut[1].abSrcOnOff, params_dst->path_info.asHpOut[1].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asSpOut[0].abSrcOnOff),
		params_src->path_info.asSpOut[0].abSrcOnOff, params_dst->path_info.asSpOut[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asSpOut[1].abSrcOnOff),
		params_src->path_info.asSpOut[1].abSrcOnOff, params_dst->path_info.asSpOut[1].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asRcOut[0].abSrcOnOff),
		params_src->path_info.asRcOut[0].abSrcOnOff, params_dst->path_info.asRcOut[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asLout1[0].abSrcOnOff),
		params_src->path_info.asLout1[0].abSrcOnOff, params_dst->path_info.asLout1[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asLout1[1].abSrcOnOff),
		params_src->path_info.asLout1[1].abSrcOnOff, params_dst->path_info.asLout1[1].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asLout2[0].abSrcOnOff),
		params_src->path_info.asLout2[0].abSrcOnOff, params_dst->path_info.asLout2[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asLout2[1].abSrcOnOff),
		params_src->path_info.asLout2[1].abSrcOnOff, params_dst->path_info.asLout2[1].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asPeak[0].abSrcOnOff),
		params_src->path_info.asPeak[0].abSrcOnOff, params_dst->path_info.asPeak[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asDit0[0].abSrcOnOff),
		params_src->path_info.asDit0[0].abSrcOnOff, params_dst->path_info.asDit0[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asDit1[0].abSrcOnOff),
		params_src->path_info.asDit1[0].abSrcOnOff, params_dst->path_info.asDit1[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asDit2[0].abSrcOnOff),
		params_src->path_info.asDit2[0].abSrcOnOff, params_dst->path_info.asDit2[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asDac[0].abSrcOnOff),
		params_src->path_info.asDac[0].abSrcOnOff, params_dst->path_info.asDac[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asDac[1].abSrcOnOff),
		params_src->path_info.asDac[1].abSrcOnOff, params_dst->path_info.asDac[1].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asAe[0].abSrcOnOff),
		params_src->path_info.asAe[0].abSrcOnOff, params_dst->path_info.asAe[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asCdsp[0].abSrcOnOff),
		params_src->path_info.asCdsp[0].abSrcOnOff, params_dst->path_info.asCdsp[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asCdsp[1].abSrcOnOff),
		params_src->path_info.asCdsp[1].abSrcOnOff, params_dst->path_info.asCdsp[1].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asCdsp[2].abSrcOnOff),
		params_src->path_info.asCdsp[2].abSrcOnOff, params_dst->path_info.asCdsp[2].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asCdsp[3].abSrcOnOff),
		params_src->path_info.asCdsp[3].abSrcOnOff, params_dst->path_info.asCdsp[3].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asAdc0[0].abSrcOnOff),
		params_src->path_info.asAdc0[0].abSrcOnOff, params_dst->path_info.asAdc0[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asAdc0[1].abSrcOnOff),
		params_src->path_info.asAdc0[1].abSrcOnOff, params_dst->path_info.asAdc0[1].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asAdc1[0].abSrcOnOff),
		params_src->path_info.asAdc1[0].abSrcOnOff, params_dst->path_info.asAdc1[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asMix[0].abSrcOnOff),
		params_src->path_info.asMix[0].abSrcOnOff, params_dst->path_info.asMix[0].abSrcOnOff);
	yamaha_mc1n2_audio_params_route_path_array_merge(sizeof(params_src->path_info.asBias[0].abSrcOnOff),
		params_src->path_info.asBias[0].abSrcOnOff, params_dst->path_info.asBias[0].abSrcOnOff);

	// dac_info
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->dac_info.bMasterSwap),
		&params_src->dac_info.bMasterSwap, &params_dst->dac_info.bMasterSwap);
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->dac_info.bVoiceSwap),
		&params_src->dac_info.bVoiceSwap, &params_dst->dac_info.bVoiceSwap);
	yamaha_mc1n2_audio_params_route_simple_array_merge(sizeof(params_src->dac_info.bDcCut),
		&params_src->dac_info.bDcCut, &params_dst->dac_info.bDcCut);

	return 0;
}

int yamaha_mc1n2_audio_route_start(struct yamaha_mc1n2_audio_pdata *pdata)
{
	struct yamaha_mc1n2_audio_params_route *params_route = NULL;
	struct yamaha_mc1n2_audio_params_init *params_init = NULL;
	struct yamaha_mc1n2_audio_params_route *params = NULL;
	struct yamaha_mc1n2_audio_params_route params_src;
	struct yamaha_mc1n2_audio_params_route params_dst;

	int rc;

	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	params_init = pdata->ops->params.init;
	if(params_init == NULL)
		return -1;

	// Copy the init params
	memcpy(&params_src.ae_info, &params_init->ae_info, sizeof(params_src.ae_info));
	memcpy(&params_src.path_info, &params_init->path_info, sizeof(params_src.path_info));
	memcpy(&params_src.dac_info, &params_init->dac_info, sizeof(params_src.dac_info));

output_merge:
	if(pdata->output_state) {
		params_route = yamaha_mc1n2_audio_params_route_find(pdata,
			pdata->output_device, YAMAHA_MC1N2_AUDIO_DIRECTION_OUTPUT);
		if(params_route == NULL)
			goto input_merge;

		memcpy(&params_dst, params_route, sizeof(params_dst));
		yamaha_mc1n2_audio_params_route_merge(&params_src, &params_dst);
		memcpy(&params_src, &params_dst, sizeof(params_src));
	}

input_merge:
	if(pdata->input_state) {
		params_route = yamaha_mc1n2_audio_params_route_find(pdata,
			pdata->input_device, YAMAHA_MC1N2_AUDIO_DIRECTION_INPUT);
		if(params_route == NULL)
			goto modem_merge;

		memcpy(&params_dst, params_route, sizeof(params_dst));
		yamaha_mc1n2_audio_params_route_merge(&params_src, &params_dst);
		memcpy(&params_src, &params_dst, sizeof(params_src));
	}

modem_merge:
	if(pdata->modem_state) {
		params_route = yamaha_mc1n2_audio_params_route_find(pdata,
			pdata->output_device, YAMAHA_MC1N2_AUDIO_DIRECTION_MODEM);
		if(params_route == NULL)
			goto route_start;

		memcpy(&params_dst, params_route, sizeof(params_dst));
		yamaha_mc1n2_audio_params_route_merge(&params_src, &params_dst);
		memcpy(&params_src, &params_dst, sizeof(params_src));
	}

route_start:
	params = &(params_src);

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_AUDIOENGINE,
		&params->ae_info, 0x0f);
	if(rc < 0) {
		ALOGE("SET_AUDIOENGINE IOCTL failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_PATH,
		&params->path_info, 0x00);
	if(rc < 0) {
		ALOGE("SET_PATH IOCTL failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_set_ctrl(pdata, MCDRV_SET_DAC,
		&params->dac_info, 0x07);
	if(rc < 0) {
		ALOGE("SET_DAC IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

int yamaha_mc1n2_audio_output_start(struct yamaha_mc1n2_audio_pdata *pdata)
{
	int rc;

	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	pdata->output_state = 1;

	rc = yamaha_mc1n2_audio_route_start(pdata);
	if(rc < 0) {
		ALOGE("Route start failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_notify(pdata, MCDRV_NOTIFY_MEDIA_PLAY_START);
	if(rc < 0) {
		ALOGE("NOTIFY_MEDIA_PLAY_START IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

int yamaha_mc1n2_audio_output_stop(struct yamaha_mc1n2_audio_pdata *pdata)
{
	int rc;

	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	pdata->output_state = 0;

	rc = yamaha_mc1n2_audio_route_start(pdata);
	if(rc < 0) {
		ALOGE("Route start failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_notify(pdata, MCDRV_NOTIFY_MEDIA_PLAY_STOP);
	if(rc < 0) {
		ALOGE("NOTIFY_MEDIA_PLAY_STOP IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

int yamaha_mc1n2_audio_input_start(struct yamaha_mc1n2_audio_pdata *pdata)
{
	int rc;

	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	pdata->input_state = 1;

	rc = yamaha_mc1n2_audio_route_start(pdata);
	if(rc < 0) {
		ALOGE("Route start failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_notify(pdata, MCDRV_NOTIFY_VOICE_REC_START);
	if(rc < 0) {
		ALOGE("NOTIFY_VOICE_REC_START IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

int yamaha_mc1n2_audio_input_stop(struct yamaha_mc1n2_audio_pdata *pdata)
{
	int rc;

	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	pdata->input_state = 0;

	rc = yamaha_mc1n2_audio_route_start(pdata);
	if(rc < 0) {
		ALOGE("Route start failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_notify(pdata, MCDRV_NOTIFY_VOICE_REC_STOP);
	if(rc < 0) {
		ALOGE("NOTIFY_VOICE_REC_START IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

int yamaha_mc1n2_audio_modem_start(struct yamaha_mc1n2_audio_pdata *pdata)
{
	int rc;

	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	pdata->modem_state = 1;

	rc = yamaha_mc1n2_audio_route_start(pdata);
	if(rc < 0) {
		ALOGE("Route start failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_notify(pdata, MCDRV_NOTIFY_CALL_START);
	if(rc < 0) {
		ALOGE("NOTIFY_CALL_START IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

int yamaha_mc1n2_audio_modem_stop(struct yamaha_mc1n2_audio_pdata *pdata)
{
	int rc;

	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	pdata->modem_state = 0;

	rc = yamaha_mc1n2_audio_route_start(pdata);
	if(rc < 0) {
		ALOGE("Route start failed, aborting!");
		return -1;
	}

	rc = yamaha_mc1n2_audio_ioctl_notify(pdata, MCDRV_NOTIFY_CALL_STOP);
	if(rc < 0) {
		ALOGE("NOTIFY_CALL_START IOCTL failed, aborting!");
		return -1;
	}

	return 0;
}

/*
 * Values configuration
 */

int yamaha_mc1n2_audio_set_route(struct yamaha_mc1n2_audio_pdata *pdata,
	audio_devices_t device)
{
	int changed = 0;

	ALOGD("%s(%x)", __func__, device);

	if(pdata == NULL)
		return -1;

	if(audio_is_output_device(device) && pdata->output_device != device) {
		pdata->output_device = device;
		changed = 1;
	} else if(audio_is_input_device(device) && pdata->input_device != device) {
		pdata->input_device = device;
		changed = 1;
	}

	if(changed && (pdata->output_state || pdata->input_state || pdata->modem_state))
		return yamaha_mc1n2_audio_route_start(pdata);

	return 0;
}

char *yamaha_mc1n2_audio_get_hw_node(struct yamaha_mc1n2_audio_pdata *pdata)
{
	if(pdata == NULL)
		return NULL;

	return pdata->ops->hw_node;
}

/*
 * Init/Deinit
 */

struct yamaha_mc1n2_audio_pdata *yamaha_mc1n2_audio_platform_get(
	char *device_name)
{
	int i;

	if(device_name == NULL)
		return NULL;

	ALOGD("Found %d registered platforms",
		yamaha_mc1n2_audio_platforms_count);

	for(i=0 ; i < yamaha_mc1n2_audio_platforms_count ; i++) {
		if(yamaha_mc1n2_audio_platforms[i] != NULL &&
			yamaha_mc1n2_audio_platforms[i]->name != NULL) {
			if(strcmp(yamaha_mc1n2_audio_platforms[i]->name, device_name) == 0) {
				return yamaha_mc1n2_audio_platforms[i];
			}
		}
	}

	return NULL;
}


int yamaha_mc1n2_audio_start(struct yamaha_mc1n2_audio_pdata **pdata_p,
	char *device_name)
{
	struct yamaha_mc1n2_audio_pdata *pdata = NULL;
	int rc;

	ALOGD("%s(%s)", __func__, device_name);

	if(pdata_p == NULL || device_name == NULL)
		return -1;

	pdata = yamaha_mc1n2_audio_platform_get(device_name);
	if(pdata == NULL || pdata->ops == NULL) {
		ALOGE("Unable to find requested platform: %s", device_name);
		return -1;
	}

	pdata->ops->hw_fd = -1;

	*pdata_p = pdata;

	return 0;
}

int yamaha_mc1n2_audio_stop(struct yamaha_mc1n2_audio_pdata *pdata)
{
	ALOGD("%s()", __func__);

	if(pdata == NULL || pdata->ops == NULL)
		return -1;

	if(pdata->ops->hw_fd >= 0) {
		close(pdata->ops->hw_fd);
	}

	return 0;
}
