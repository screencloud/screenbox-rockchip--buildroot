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

#ifndef __DPI_H__
#define __DPI_H__

#include "dpp_frame.h"
#include "dpp_packet.h"

typedef enum {
  DPP_CTX_DSP,
  DPP_CTX_BUTT,
} DppCtxType;

typedef enum {
  DPP_FUN_COPY,
  DPP_FUN_3DNR,
  DPP_FUN_ADAS,
  DPP_FUN_BUTT,
} DppFunType;

typedef enum {
  DPP_CXT_IDLE  = 1000,
  DPP_CXT_RUNNING,

  DPP_CXT_BUTT,
} DppCxtStatus;

typedef enum {
  DPP_CMD_BASE,
  DPP_CMD_SET_FUN_TYPE,
  DPP_CMD_GET_STATUS,
  DPP_CMD_RESET,

  DPI_CMD_BUTT,
} DpiCmd;

typedef void *DppCtx;
typedef void *DppParam;

/*
 * mpp main work function set
 * size     : DppApi structure size
 * version  : Dpp svn revision
 *
 * put_packet : send raw stream packet to ceva, async interface.
 * get_frame  : get output frame from process task, sync interface.
 *
 */
typedef struct DppApi_t {
  unsigned int size;
  unsigned int version;

  // async interface
  DPP_RET (*put_packet)(DppCtx ctx, DppPacket packet_in);

  // sync interface
  DPP_RET (*get_frame)(DppCtx ctx, DppFrame *frame_out);

  DPP_RET (*control)(DppCtx ctx, DpiCmd cmd, DppParam param);

  unsigned int reserv[16];
} DppApi;


#define FRAME_OUT_BUFF_LIMIT 3

#ifdef __cplusplus
extern "C" {
#endif

/*
 * dpp interface work flow
 *
 * 1. dpp_create : Create empty context structure and dpi function pointers with context type and type(such as DSP).
 * 2. dpp_destory: Destroy dpp context and free both context and dpi structure
 */
DPP_RET dpp_create(DppCtx *ctx, DppApi **dpi, DppCtxType type);
DPP_RET dpp_destroy(DppCtx ctx);
DPP_RET dpp_start(DppCtx ctx);
DPP_RET dpp_stop(DppCtx ctx);

// type format function
DPP_RET dpp_check_support_format(DppCtxType type);
void    dpp_show_support_format();

#ifdef __cplusplus
}
#endif

#endif /*__DPI_H__*/
