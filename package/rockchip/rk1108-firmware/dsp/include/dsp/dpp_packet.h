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

#ifndef __DPP_PACKET_H__
#define __DPP_PACKET_H__
#include <sys/time.h>

#include "dpp_buffer.h"

typedef void* DppPacket;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DppPacket interface
 *
 * packet struct include:
 * @buf_info    buffer info form external create
 * @buffer      cma buffer of dpp management
 * @width       packet buffer width
 * @height      packet buffer height
 * @release_cb  packet buffer release callback
 */
DPP_RET dpp_packet_init(DppPacket *packet, unsigned int width, unsigned int height);
DPP_RET dpp_packet_set_cb(DppPacket packet, void (*release_cb)(void *));
DPP_RET dpp_packet_release(DppPacket packet);
DPP_RET dpp_packet_copy_init(DppPacket *packet, const DppPacket src);
DPP_RET dpp_packet_deinit(DppPacket packet);

void dpp_packet_set_buf_info(DppPacket packet, DppBufferInfo *buf_info);
void dpp_packet_set_buffer(DppPacket packet, DppBuffer buffer);
void dpp_packet_set_width(DppPacket packet, unsigned int width);
void dpp_packet_set_height(DppPacket packet, unsigned int height);
void dpp_packet_set_params(DppPacket packet, void *params, unsigned int size);
void dpp_packet_set_ldw_type(DppPacket packet, unsigned int type);
void dpp_packet_set_data(DppPacket packet, void *data);
void dpp_packet_set_pts(DppPacket packet, struct timeval pts);

DppBufferInfo *dpp_packet_get_buf_info(const DppPacket packet);
DppBuffer dpp_packet_get_buffer(const DppPacket packet);
unsigned int dpp_packet_get_width(const DppPacket packet);
unsigned int dpp_packet_get_height(const DppPacket packet);
unsigned int dpp_packet_get_ldw_type(const DppPacket packet);
unsigned int dpp_packet_get_buf_size(const DppPacket packet);
void *dpp_packet_get_data(const DppPacket packet);
DPP_RET dpp_packet_get_params(const DppPacket packet, void **params, unsigned int *size);
struct timeval dpp_packet_get_pts(const DppPacket packet);

#ifdef __cplusplus
}
#endif

#endif /*__DPP_PACKET_H__*/
