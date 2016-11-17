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

#ifndef __DPP_ERR_H__
#define __DPP_ERR_H__

typedef enum {
	DPP_SUCCESS                 = 0,
	DPP_OK                      = 0,
	
	DPP_NOK                     = -1,
	DPP_ERR_UNKNOW              = -2,
	DPP_ERR_NULL_PTR            = -3,
	DPP_ERR_MALLOC              = -4,
	DPP_ERR_INIT                = -5,    
	DPP_ERR_DEV_OPEN_FAIL       = -6,
	DPP_ERR_DEV_DEQUEUE		    = -7,
	
} DPP_RET;

#endif /*__DPP_ERR_H__*/
