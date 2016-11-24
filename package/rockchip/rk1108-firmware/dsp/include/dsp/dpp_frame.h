/*
 *  Copyright (c) 2016 Rockchip Electronics Co. Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DPP_FRAME_H__
#define __DPP_FRAME_H__
#include <sys/time.h>

#include "dpp_buffer.h"

typedef void* DppFrame;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  DPP_FRAME_COPY = 0,
  DPP_FRAME_3DNR,
  DPP_FRAME_ADAS,
} DPP_FRAME_TYPE;

typedef enum {
  DPP_ADAS_LDW = 0, // car line detection
  DPP_ADAS_FCW,     // car block detection
} DPP_ADAS_TYPE;

typedef struct _dsp_ldw_out
{
  unsigned int turnflg; // 0 turn left; 1 run normal; 2 turn right
  int endPoints[4][2];  // [0][0] left; [0][1] right
} dsp_ldw_out;

/*
 * DppFrame interface
 *
 * @dpp_isp_data: ISP data
 * @dpp_dsp_data: DSP data
 * @buf_info:     buffer info form external create
 * @buffer:       cma buffer of dpp management
 * @width:        packet buffer width
 * @height:       packet buffer height
 */
DPP_RET dpp_frame_init(DppFrame *frame);
DPP_RET dpp_frame_deinit(DppFrame frame);
DPP_RET dpp_frame_copy(DppFrame dst, DppFrame src);

void dpp_frame_set_buf_info(DppFrame frame, DppBufferInfo *buf_info);
void dpp_frame_set_buffer(DppFrame frame, DppBuffer buffer);
void dpp_frame_set_width(DppFrame frame, unsigned int width);
void dpp_frame_set_height(DppFrame frame, unsigned int height);
void dpp_frame_set_data(DppFrame frame, void *data);
void dpp_frame_set_type(DppFrame frame, DPP_FRAME_TYPE type);
void dpp_frame_set_adas_out(DppFrame frame, dsp_ldw_out *adas_out);
void dpp_frame_set_pts(DppFrame frame, struct timeval pts);
void dpp_frame_set_noise(DppFrame frame, unsigned int* noise);

DppBufferInfo *dpp_frame_get_buf_info(const DppFrame frame);
DppBuffer dpp_frame_get_buffer(const DppFrame frame);
void dpp_frame_release_buffer(const DppFrame frame);
unsigned int dpp_frame_get_width(const DppFrame frame);
unsigned int dpp_frame_get_height(const DppFrame frame);
void *dpp_frame_get_data(const DppFrame frame);
DPP_FRAME_TYPE dpp_frame_get_type(const DppFrame frame);
dsp_ldw_out *dpp_frame_get_adas_out(const DppFrame frame);
struct timeval dpp_frame_get_pts(DppFrame frame);
void dpp_frame_get_noise(DppFrame frame, unsigned int* noise);

#ifdef __cplusplus
}
#endif

#endif /*__DPP_FRAME_H__*/
