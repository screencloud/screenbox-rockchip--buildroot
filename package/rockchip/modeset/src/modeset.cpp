/*
 * Copyright (C) 2007 The Android Open Source Project
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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h> 
#include <string> 
#include <jpeglib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

#include "drm_common.h"
using namespace std;


struct mdrm_mode_modeinfo {
    __u32 clock;
    __u16 hdisplay;
    __u16 hsync_start;
    __u16 hsync_end;
    __u16 htotal;
    __u16 hskew;
    __u16 vdisplay;
    __u16 vsync_start;
    __u16 vsync_end;
    __u16 vtotal;
    __u16 vscan;

    __u32 vrefresh;

    __u32 flags;
    __u32 type;
    char name[32];
};
struct bo
{
	int fd;
	void *ptr;
	size_t size;
	size_t offset;
	size_t pitch;
	unsigned handle;
};

    struct armsoc_bo *
bo_create_dumb(int fd, unsigned int width, unsigned int height, unsigned int bpp)
{
    struct drm_mode_create_dumb arg;
    struct armsoc_bo *bo;
    int ret;

    bo = (struct armsoc_bo *)calloc(1, sizeof(*bo));
    if (bo == NULL) {
        fprintf(stderr, "failed to allocate buffer object\n");
        return NULL;
    }

    memset(&arg, 0, sizeof(arg));
    arg.bpp = bpp;
    arg.width = width;
    arg.height = height;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
    if (ret) {
        fprintf(stderr, "failed to create dumb buffer: %s\n",
                strerror(errno));
        free(bo);
        return NULL;
    }

    bo->fd = fd;
    bo->handle = arg.handle;
    bo->size = arg.size;
    bo->pitch = arg.pitch;
    bo->width = width;
    bo->height = height;
    return bo;
}

int bo_map(struct armsoc_bo *bo)
{
    struct drm_mode_map_dumb arg;
    void *map;
    int ret;

    memset(&arg, 0, sizeof(arg));
    arg.handle = bo->handle;

    ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
    if (ret)
        return ret;

    /*map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
      bo->fd, arg.offset);*/
    drmMap(bo->fd, arg.offset, bo->size, &map);
    if (map == MAP_FAILED)
        return -EINVAL;

    bo->ptr = map;


    return 0;
}

void bo_unmap(struct armsoc_bo *bo)
{
    if (!bo->ptr)
        return;

    //drm_munmap(bo->ptr, bo->size);
    drmUnmap(bo->ptr, bo->size);
    bo->ptr = NULL;
}

void bo_destroy(struct armsoc_bo *bo)
{
    struct drm_mode_destroy_dumb arg;
    int ret;

    if (bo->fb_id) {
        drmModeRmFB (bo->fd, bo->fb_id);
        bo->fb_id = 0;
    }

    memset(&arg, 0, sizeof(arg));
    arg.handle = bo->handle;

    ret = drmIoctl(bo->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
    if (ret)
        fprintf(stderr, "failed to destroy dumb buffer: %s fd=0x%x\n",
                strerror(errno), bo->fd);

    //free(bo);
}

struct armsoc_bo *
bo_create(int fd, unsigned int format,
        unsigned int width, unsigned int height)
{
    unsigned int virtual_height;
    struct armsoc_bo *bo;
    unsigned int bpp;
    int ret;

    switch (format) {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YVU420:
            bpp = 8;
            break;

        case DRM_FORMAT_ARGB4444:
        case DRM_FORMAT_XRGB4444:
        case DRM_FORMAT_ABGR4444:
        case DRM_FORMAT_XBGR4444:
        case DRM_FORMAT_RGBA4444:
        case DRM_FORMAT_RGBX4444:
        case DRM_FORMAT_BGRA4444:
        case DRM_FORMAT_BGRX4444:
        case DRM_FORMAT_ARGB1555:
        case DRM_FORMAT_XRGB1555:
        case DRM_FORMAT_ABGR1555:
        case DRM_FORMAT_XBGR1555:
        case DRM_FORMAT_RGBA5551:
        case DRM_FORMAT_RGBX5551:
        case DRM_FORMAT_BGRA5551:
        case DRM_FORMAT_BGRX5551:
        case DRM_FORMAT_RGB565:
        case DRM_FORMAT_BGR565:
        case DRM_FORMAT_UYVY:
        case DRM_FORMAT_VYUY:
        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            bpp = 16;
            break;

        case DRM_FORMAT_BGR888:
        case DRM_FORMAT_RGB888:
            bpp = 24;
            break;

        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_XBGR8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_RGBX8888:
        case DRM_FORMAT_BGRA8888:
        case DRM_FORMAT_BGRX8888:
        case DRM_FORMAT_ARGB2101010:
        case DRM_FORMAT_XRGB2101010:
        case DRM_FORMAT_ABGR2101010:
        case DRM_FORMAT_XBGR2101010:
        case DRM_FORMAT_RGBA1010102:
        case DRM_FORMAT_RGBX1010102:
        case DRM_FORMAT_BGRA1010102:
        case DRM_FORMAT_BGRX1010102:
            bpp = 32;
            break;

        default:
            fprintf(stderr, "unsupported format 0x%08x\n",  format);
            return NULL;
    }

    switch (format) {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YVU420:
            virtual_height = height * 3 / 2;
            break;

        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
            virtual_height = height * 2;
            break;

        default:
            virtual_height = height;
            break;
    }

    bo = bo_create_dumb(fd, width, virtual_height, bpp);
    bo->format = format;
    if (!bo)
        return NULL;

    ret = bo_map(bo);
    if (ret) {
        fprintf(stderr, "failed to map buffer: %s\n",
                strerror(-errno));
        bo_destroy(bo);
        return NULL;
    }

    /* just testing a limited # of formats to test single
     * and multi-planar path.. would be nice to add more..
     */
    //bo_unmap(bo);

    return bo;
}

static int drm_init(int drmFd) {
    int i, ret;

    ret = drmSetClientCap(drmFd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    if (ret) {
        printf( "Failed to set atomic cap %s \n", strerror(errno));
        return ret;
    }

    ret = drmSetClientCap(drmFd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret) {
        printf( "Failed to set atomic cap %s \n", strerror(errno));
        return ret;
    }


    if (!drm_get_resources(drmFd))
        return -1;

    if (!drm_get_connector(drmFd))
        return -1;

    if (!drm_get_encoder(drmFd))
        return -1;
}

static int
drmmode_getproperty(int drmFd, uint32_t obj_id, uint32_t obj_type, const char *prop_name)
{
    int property_id=-1;
    bool found = false;
    drmModeObjectPropertiesPtr props;

    props = drmModeObjectGetProperties(drmFd, obj_id, obj_type);
    for (int i = 0; !found && (size_t)i < props->count_props; ++i) {
        drmModePropertyPtr p = drmModeGetProperty(drmFd, props->props[i]);
        if (!strcmp(p->name, prop_name)) {
            property_id = p->prop_id;
            found = TRUE;
			break;
        }
        drmModeFreeProperty(p);
    }

    drmModeFreeObjectProperties(props);

    return property_id;
}

static int CreatePropertyBlob(void *data, size_t length,
        uint32_t *blob_id, int drm_fd) {
    struct drm_mode_create_blob create_blob;
    int ret=-1;
    memset(&create_blob, 0, sizeof(create_blob));
    create_blob.length = length;
    create_blob.data = (__u32)data;

    ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATEPROPBLOB, &create_blob);
    if (ret) {
        return ret;
    }
    *blob_id = create_blob.blob_id;
    return 0;
}

static int resolve_connectors(int drm_fd, drmModeRes *res, uint32_t *con_ids)
{
	drmModeConnector *connector;
	unsigned int i;
	uint32_t id;
	char *endp;

	for (i = 0; (int)i < res->count_connectors; i++)
	{
	   drmModeConnectorPtr conn = drmModeGetConnector(
			 drm_fd, res->connectors[i]);
	
	   if (conn)
	   {
		  bool connected = conn->connection == DRM_MODE_CONNECTED;
		  con_ids[i] = conn->connector_id;
		  printf("con_ids[%d]=%d \n", i, con_ids[i]);
		  drmModeFreeConnector(conn);
	   }
	}

	return 0;
}

static void setMode(int drmFd, int modeid){
	uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
	unsigned int fb_id;
	struct armsoc_bo *bo;
	unsigned int i;
	unsigned int j;
	int ret, x;
	uint32_t num_cons;
	drmModeAtomicReq *req;
	uint32_t blob_id=0;
	int property_crtc_id,mode_property_id,active_property_id;
	drmModeCrtcPtr crtc;
	drmModeModeInfo *drm_mode = NULL;
	drmModeEncoder *drm_encoder  = NULL;
	drmModeRes *g_resources = g_drm_resources;

	num_cons = g_resources->count_connectors;

	if (modeid < g_drm_connector->count_modes)
		drm_mode = &g_drm_connector->modes[modeid];
	else
		drm_mode = &g_drm_connector->modes[0];
	if (g_resources->count_crtcs > 1) {
		if (g_drm_connector->encoder_id == 0 && 
		    g_drm_connector->connector_type == DRM_MODE_CONNECTOR_HDMIA)
			crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[0]);
		else if(g_drm_connector->encoder_id == 0)
			crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[1]);
		else
			crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[0]);
	} else {
		crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[0]);;
	}

	req = drmModeAtomicAlloc();
	#define DRM_ATOMIC_ADD_PROP(object_id, prop_id, value) \
		ret = drmModeAtomicAddProperty(req, object_id, prop_id, value); \
		printf("object[%d] = %d\n", object_id, value); \
		if (ret < 0) \
		printf("Failed to add prop[%d] to [%d]", value, object_id);

	if (modeid < g_drm_connector->count_modes)
		g_drm_mode = &g_drm_connector->modes[modeid];
	else
		g_drm_mode = &g_drm_connector->modes[0];

	g_connector_id = g_drm_connector->connector_id;

	ret = CreatePropertyBlob(g_drm_mode, sizeof(*g_drm_mode), &blob_id, drmFd);
    property_crtc_id = drmmode_getproperty(drmFd, g_connector_id, DRM_MODE_OBJECT_CONNECTOR, "CRTC_ID");
    DRM_ATOMIC_ADD_PROP(g_connector_id, property_crtc_id, crtc->crtc_id);
    mode_property_id = drmmode_getproperty(drmFd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC, "MODE_ID");
    DRM_ATOMIC_ADD_PROP(crtc->crtc_id, mode_property_id, blob_id);
    active_property_id = drmmode_getproperty(drmFd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC, "ACTIVE");
    DRM_ATOMIC_ADD_PROP(crtc->crtc_id, active_property_id, 1);
	ret = drmModeAtomicCommit(drmFd, req, DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_PAGE_FLIP_EVENT, NULL);
    drmModeAtomicFree(req);
	if (crtc)
        drmModeFreeCrtc(crtc);
}

static void set_crtc_mode(int drmFd, int modeid)
{
	uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
	unsigned int fb_id;
	struct armsoc_bo *bo;
	unsigned int i;
	unsigned int j;
	int ret, x;
	uint32_t* con_ids;
	uint32_t num_cons;
	drmModeCrtcPtr crtc;
	drmModeModeInfo *drm_mode = NULL;
	drmModeEncoder *drm_encoder  = NULL;
	drmModeRes *g_resources = g_drm_resources;

	num_cons = g_resources->count_connectors;
	con_ids = (uint32_t*)calloc(num_cons, sizeof(*con_ids));
	resolve_connectors(drmFd, g_resources, con_ids);

	if (modeid < g_drm_connector->count_modes)
		drm_mode = &g_drm_connector->modes[modeid];
	else
		drm_mode = &g_drm_connector->modes[0];
	if (g_resources->count_crtcs > 1) {
		if (g_drm_connector->encoder_id == 0 && 
		    g_drm_connector->connector_type == DRM_MODE_CONNECTOR_HDMIA)
			crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[0]);
		else if(g_drm_connector->encoder_id == 0)
			crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[1]);
		else
			crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[0]);
	} else {
		crtc = drmModeGetCrtc(drmFd, g_resources->crtcs[0]);;
	}

	bo = bo_create(drmFd, DRM_FORMAT_ARGB8888, drm_mode->hdisplay,
		       drm_mode->vdisplay);
	offsets[0] = 0;
	handles[0] = bo->handle;
	pitches[0] = bo->pitch;

	ret = drmModeAddFB2(drmFd, drm_mode->hdisplay, drm_mode->vdisplay,
			    DRM_FORMAT_ARGB8888, handles, pitches, offsets, &fb_id, 0);
	if (ret) {
		fprintf(stderr, "failed to add fb (%ux%u): %s\n",
			drm_mode->hdisplay, drm_mode->vdisplay, strerror(errno));
		return;
	}

	ret = drmModeSetCrtc(drmFd, crtc->crtc_id, fb_id,
			     0, 0, con_ids, num_cons,
			     drm_mode);

	/* XXX: Actually check if this is needed */
	drmModeDirtyFB(drmFd, fb_id, NULL, 0);
	x += drm_mode->hdisplay;

	if (ret) {
		fprintf(stderr, "failed to set mode: %s\n", strerror(errno));
		return;
	}
}

int main(int argc, char** argv)
{
    int                     mThreadStatus;
    pthread_t				thread_id;
    int drm_fd = 0;

	int modeid=0;
	if (argc > 1)
		modeid = atoi(argv[1]);

	printf("argc=%d modeid=%d\n", argc, modeid);
    drm_fd = open("/dev/dri/card0", O_RDWR);
    if (drm_fd < 0)
        printf("Failed to open dri 0 =%s\n", strerror(errno));
    drm_init(drm_fd);
#if 0
	if (g_drm_encoder != NULL)
		setMode(drm_fd, modeid);
	else
		set_crtc_mode(drm_fd, modeid);
#else
	setMode(drm_fd, modeid);	
#endif
	drm_free();

    return 0;
}
