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

#define BOOT_ANIMATION_CONFIG_FILE "/mnt/bootanimation/boot.conf"

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
struct plane_prop {
    int crtc_id;
    int fb_id;
    int src_x;
    int src_y;
    int src_w;
    int src_h;
    int crtc_x;
    int crtc_y;
    int crtc_w;
    int crtc_h;
};

typedef struct
{
    void * decoderHandle;
    char *outAddr;
    int phyAddr;
} HwJpegOutputBuf;

typedef struct
{
    struct armsoc_bo* bo;
}SfJpegOutputBuf;

#define JPEG_OUTPUT_BUF_CNT 6
#define MAX_OUTPUT_RSV_CNT 4

// ---------------------------------------------------------------------------
static int m_next_frame_time = 16;
static int m_thread_run = 0;
int tmpCnt=0;
int bufCnt = JPEG_OUTPUT_BUF_CNT;
static int mImageCount = 0;
char mBootAnimPath[128];
SfJpegOutputBuf mSfJpegBufInfo[MAX_OUTPUT_RSV_CNT];

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

static bool drmmode_getproperty(int drmFd, uint32_t obj_id, uint32_t obj_type, struct mdrm_mode_modeinfo* drm_mode)
{
    int property_id=-1;
    bool found = false;
    drmModeObjectPropertiesPtr props;

    props = drmModeObjectGetProperties(drmFd, obj_id, obj_type);
    for (int i = 0; !found && (size_t)i < props->count_props; ++i) {
        drmModePropertyBlobPtr blob;
        drmModePropertyPtr p = drmModeGetProperty(drmFd, props->props[i]);
        int value;

        if (!strcmp(p->name, "MODE_ID")) {
            struct mdrm_mode_modeinfo* mCurMode;
            found = true;
            if (!drm_property_type_is(p, DRM_MODE_PROP_BLOB)) {
                printf("%s:line=%d,is not blob",__FUNCTION__,__LINE__);
                drmModeFreeProperty(p);
                drmModeFreeObjectProperties(props);
                return false;
            }
            if (!p->count_blobs)
                value = props->prop_values[i];
            else
                value = p->blob_ids[0];
            blob = drmModeGetPropertyBlob(drmFd, value);
            if (!blob) {
                printf("%s:line=%d, blob is null\n",__FUNCTION__,__LINE__);
                drmModeFreeProperty(p);
                drmModeFreeObjectProperties(props);
                return false;
            }

            float vfresh;
            mCurMode = (struct mdrm_mode_modeinfo *)blob->data;
            memcpy(drm_mode, mCurMode, sizeof(*drm_mode));
            if (drm_mode->flags & DRM_MODE_FLAG_INTERLACE)
                vfresh = drm_mode->clock *2/ (float)(drm_mode->vtotal * drm_mode->htotal) * 1000.0f;
            else
                vfresh = drm_mode->clock / (float)(drm_mode->vtotal * drm_mode->htotal) * 1000.0f;
            //printf("drmmode_getproperty w=%d h=%d \n", drm_mode->hdisplay, drm_mode->vdisplay);
            drmModeFreePropertyBlob(blob);
        }
        drmModeFreeProperty(p);
    }
    drmModeFreeObjectProperties(props);
    return found;
}

static int drm_get_curActiveResolution(int drm_fd, int* width, int* height){
    struct mdrm_mode_modeinfo drm_mode;
    int ret=-1;

    if (g_drm_encoder != NULL) {
        ret = drmmode_getproperty(drm_fd, g_drm_encoder->crtc_id, DRM_MODE_OBJECT_CRTC, &drm_mode);
        if (ret == true) {
            *width = drm_mode.hdisplay;
            *height = drm_mode.vdisplay;
        } else {
            *width = 1280;
            *height = 720;
        }
    }
    return ret;
}

static int ctx_drm_display(int drm_fd, struct armsoc_bo* bo, int x, int y)
{
    drmModePlaneResPtr plane_res;
    drmModePlanePtr plane;
    drmModeResPtr res;
    drmModeCrtcPtr crtc = NULL;
    drmModeObjectPropertiesPtr props;
    drmModeAtomicReq *req;
    struct plane_prop plane_prop;
    int i, ret;
    int found_crtc = 0;
    int32_t flags = 0;
    uint32_t gem_handle;
    int crtc_w = 1280;
    int crtc_h = 720;

    if (g_drm_resources == NULL) {
        res = drmModeGetResources(drm_fd);
        if (!res) {
            printf("Failed to get resources: %s\n",
                    strerror(errno));
            return -ENODEV;
        }
    } else {
        res = g_drm_resources;
    }
    if (g_drm_encoder == NULL) {
        drm_update(drm_fd);
        if (g_drm_encoder == NULL)
            return 0;
    }
    /*
     * Found active crtc.
     */
    for (i = 0; i < res->count_crtcs; ++i) {
        uint32_t j;
        crtc = drmModeGetCrtc(drm_fd, res->crtcs[i]);
        if (!crtc) {
            printf("Could not get crtc %u: %s\n",
                    res->crtcs[i], strerror(errno));
            continue;
        }
        props = drmModeObjectGetProperties(drm_fd, crtc->crtc_id,
                DRM_MODE_OBJECT_CRTC);
        if (!props) {
            printf("failed to found props crtc[%d] %s\n",
                    crtc->crtc_id, strerror(errno));
            continue;
        }
        for (j = 0; j < props->count_props; j++) {
            drmModePropertyPtr prop;
            prop = drmModeGetProperty(drm_fd, props->props[j]);
            if (!strcmp(prop->name, "ACTIVE")) {
                if (props->prop_values[j]) {
                    //printf("found active crtc %d\n", crtc->crtc_id);
                    found_crtc = 1;
                    drmModeFreeProperty(prop);
                    break;
                }
            }
            drmModeFreeProperty(prop);
        }
        if (props)
            drmModeFreeObjectProperties(props);
        if (found_crtc)
            break;
        drmModeFreeCrtc(crtc);
    }

    if (!crtc) {
        if (props)
            drmModeFreeObjectProperties(props);
        printf("failed to find usable crtc props\n");
        return -ENODEV;
    }

    plane_res = drmModeGetPlaneResources(drm_fd);
    for (uint32_t i = 0; i < plane_res->count_planes; ++i) {
        bool foundPlane=false;
        plane = drmModeGetPlane(drm_fd, plane_res->planes[i]);
        //printf(" plan[%d].count_formats 0x%x : \n", i, plane->count_formats);
        for (uint32_t j = 0; j < plane->count_formats; j++) {
            //printf("  format %d : \n", plane->formats[j]);
            if (plane->formats[j] == bo->format) {
                //printf("found RGB layer ************\n");
                foundPlane = true;
                break;
            }
        }
        if (foundPlane)
            break;
        drmModeFreePlane(plane);
    }

    //plane = drmModeGetPlane(drm_fd, plane_res->planes[1]);
    props = drmModeObjectGetProperties(drm_fd, plane->plane_id,
            DRM_MODE_OBJECT_PLANE);

    if (!props) {
        printf("failed to found props plane[%d] %s\n",
                plane->plane_id, strerror(errno));
        if (plane)
            drmModeFreePlane(plane);
        if (plane_res)
            drmModeFreePlaneResources(plane_res);
        if (props)
            drmModeFreeObjectProperties(props);
        if (crtc)
            drmModeFreeCrtc(crtc);
        return -ENODEV;
    }
    memset(&plane_prop, 0, sizeof(struct plane_prop));
    for (i = 0; i < props->count_props; i++) {
        drmModePropertyPtr prop;
        prop = drmModeGetProperty(drm_fd, props->props[i]);
        if (!strcmp(prop->name, "CRTC_ID"))
            plane_prop.crtc_id = prop->prop_id;
        else if (!strcmp(prop->name, "FB_ID"))
            plane_prop.fb_id = prop->prop_id;
        else if (!strcmp(prop->name, "SRC_X"))
            plane_prop.src_x = prop->prop_id;
        else if (!strcmp(prop->name, "SRC_Y"))
            plane_prop.src_y = prop->prop_id;
        else if (!strcmp(prop->name, "SRC_W"))
            plane_prop.src_w = prop->prop_id;
        else if (!strcmp(prop->name, "SRC_H"))
            plane_prop.src_h = prop->prop_id;
        else if (!strcmp(prop->name, "CRTC_X"))
            plane_prop.crtc_x = prop->prop_id;
        else if (!strcmp(prop->name, "CRTC_Y"))
            plane_prop.crtc_y = prop->prop_id;
        else if (!strcmp(prop->name, "CRTC_W"))
            plane_prop.crtc_w = prop->prop_id;
        else if (!strcmp(prop->name, "CRTC_H"))
            plane_prop.crtc_h = prop->prop_id;

        drmModeFreeProperty(prop);
        //xf86DrvMsg(-1, X_WARNING, "prop[%d] = %d\n", i, prop->prop_id);
    }
    drm_get_curActiveResolution(drm_fd, &crtc_w, &crtc_h);
    req = drmModeAtomicAlloc();
#define DRM_ATOMIC_ADD_PROP(object_id, prop_id, value) \
    ret = drmModeAtomicAddProperty(req, object_id, prop_id, value); \
    if (ret < 0) \
    printf("Failed to add prop[%d] to [%d]", value, object_id);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.crtc_id, crtc->crtc_id);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.fb_id, bo->fb_id);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.src_x, x);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.src_y, y);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.src_w, bo->width << 16);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.src_h, bo->height << 16);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.crtc_x, 0);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.crtc_y, 0);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.crtc_w, crtc_w);
    DRM_ATOMIC_ADD_PROP(plane->plane_id, plane_prop.crtc_h, crtc_h);

    flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
#if 1
    ret = drmModeAtomicCommit(drm_fd, req, flags, NULL);
    if (ret)
        printf("atomic: couldn't commit new state: %s\n", strerror(errno));
    drmModeAtomicFree(req);
#else
    if (drmModeSetPlane(drm_fd, plane->plane_id,
                crtc->crtc_id, bo->fb_id, 0,
                0, 0, crtc_w, crtc_w,  0, 0, bo->width<<16, bo->height<<16)){
        printf("failed to enable plane: %s\n",
                strerror(errno));
    }
#endif

    if (plane)
        drmModeFreePlane(plane);
    if (plane_res)
        drmModeFreePlaneResources(plane_res);
    if (props)
        drmModeFreeObjectProperties(props);
    if (crtc)
        drmModeFreeCrtc(crtc);
    return ret;
}

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

int bo_destroy(struct armsoc_bo *bo)
{
    struct drm_mode_destroy_dumb arg;
    struct drm_gem_close data;
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

    memset(&data, 0, sizeof(data));
    /*
       data.handle = bo->handle;
       ret = drmIoctl(bo->fd, DRM_IOCTL_GEM_CLOSE, &data);
       if (ret)
       return -errno;*/
    return ret;
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

static void jpeg_get_displayinfo(char* path, int* width, int* height)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* infile=NULL;
#ifdef TIME_DEBUG
    struct timeval start,end;
    gettimeofday(&start, NULL);
#endif

    if ((infile = fopen(path, "rb")) == NULL)
    {
        *width = 0;
        *height = 0;
        //printf("fopen fail path=%s\n", path);
        return;
    }
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, true);
    jpeg_calc_output_dimensions(&cinfo);
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    jpeg_destroy_decompress (&cinfo);
#ifdef TIME_DEBUG
    gettimeofday(&end, NULL);
    printf("jpeg_get_displayinfo usetime=%d************4\n", (1000000 *(end.tv_sec - start.tv_sec) + (end.tv_usec-start.tv_usec))/1000);
#endif
    if (infile)
        fclose(infile);
}

static int jpeg_sf_decode(char* path, char* output)
{
    struct jpeg_decompress_struct cinfo;  
    struct jpeg_error_mgr jerr; 
    JSAMPARRAY lineBuf;// = (JSAMPIMAGE)malloc(sizeof(JSAMPARRAY)*1);;
    int bytesPerPix;
#ifdef TIME_DEBUG
    struct timeval start,end;
    gettimeofday(&start, NULL);
#endif

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    FILE* infile;
    int outSize=0;

    if ((infile = fopen(path, "rb")) == NULL)
    {
        perror("fopen fail");
        return 0;
    }
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, true);
    //cinfo.out_color_space = JCS_YCbCr;
    //cinfo.raw_data_out = true;
    jpeg_start_decompress(&cinfo);
#if 0
    printf("cinfo.image_width=%d cinfo.image_height=%d cinfo.jpeg_color_space=%d output-%p\n", 
            cinfo.image_width, cinfo.image_height, cinfo.jpeg_color_space, output);
    printf("cinfo.output_width=%d cinfo.output_height=%d cinfo.bytesPerPix=%d\n", 
            cinfo.output_width, cinfo.output_height, cinfo.output_components);
#endif
    outSize = cinfo.output_width * cinfo.output_height*3;
    bytesPerPix = cinfo.output_components;
    if(output == NULL)
    {
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        return -2;
    }
    lineBuf = cinfo.mem->alloc_sarray ((j_common_ptr) &cinfo, JPOOL_IMAGE, (cinfo.output_width * bytesPerPix), 1);
    if (3 == bytesPerPix) {
        int y;
        for (y = 0; y < cinfo.output_height; ++y) {
            jpeg_read_scanlines (&cinfo, lineBuf, 1);
            memcpy ((output + y * cinfo.output_width * 3),lineBuf[0],3 * cinfo.output_width);
        }
    } else if (1 == bytesPerPix) {
        unsigned int col;
        int lineOffset = (cinfo.output_width * 3);
        int lineBufIndex;
        int x ;
        int y;

        for (y = 0; y < cinfo.output_height; ++y) {
            jpeg_read_scanlines (&cinfo, lineBuf, 1);

            lineBufIndex = 0;
            for (x = 0; x < lineOffset; ++x) {
                col = lineBuf[0][lineBufIndex];

                output[(lineOffset * y) + x] = col;
                ++x;
                output[(lineOffset * y) + x] = col;
                ++x;
                output[(lineOffset * y) + x] = col;

                ++lineBufIndex;
            }   
        }
    } else {
        printf ("Error: the number of color channels is %d.	This program only handles 1 or 3\n", bytesPerPix);
        return -1;
    }

    jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);
    fclose (infile);
#ifdef TIME_DEBUG
    gettimeofday(&end, NULL);
    printf("jpeg_sf_decode usetime=%d************4\n", (1000000 *(end.tv_sec - start.tv_sec) + (end.tv_usec-start.tv_usec))/1000);
#endif
#if 0
    infile = fopen("/mnt/sdcard/result.yuv", "wb");
    printf("errno = %s \n", strerror(errno));
    if (infile) {
        fwrite(output, outSize, 1, infile);
        fclose(infile);
    }
#endif
    return 0;
}

void* bootAnimation(void* parm)
{
    int* drm_fd = (int *)parm;
    int mFrameNum = 0;

    do {
        int pathSize = 128;
        char* picPath = (char*)calloc(1, pathSize);
        int displayWidth=0,displayHeight=0;
        uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
        for (int j=0;j<mImageCount;j++) {
            memset(picPath, 0, pathSize);
            displayWidth=displayHeight=0;
            sprintf(picPath, "%s/%d.jpg",mBootAnimPath,j);
            jpeg_get_displayinfo(picPath, &displayWidth, &displayHeight);
            if (displayWidth ==0 || displayHeight == 0){
                sprintf(picPath, "%s/%d.jpeg",mBootAnimPath,j);
                jpeg_get_displayinfo(picPath, &displayWidth, &displayHeight);
            }

            if (displayWidth !=0 && displayHeight != 0) {
                struct armsoc_bo* bo=NULL;
                struct armsoc_bo* tmpBo=NULL;

                tmpBo = mSfJpegBufInfo[MAX_OUTPUT_RSV_CNT-1].bo;
                if (tmpBo && tmpBo->fd != 0 && tmpBo->ptr != NULL) {
                    bo_unmap(tmpBo);
                    bo_destroy(tmpBo);
                    free(tmpBo);
                    mSfJpegBufInfo[MAX_OUTPUT_RSV_CNT-1].bo = NULL;
                    tmpBo = NULL;
                }
                bo = bo_create(*drm_fd, DRM_FORMAT_RGB888, displayWidth, displayHeight);
                jpeg_sf_decode(picPath, (char*)bo->ptr);
                offsets[0] = 0;
                handles[0] = bo->handle;
                pitches[0] = bo->pitch;
                if (drmModeAddFB2(*drm_fd, bo->width, bo->height, DRM_FORMAT_RGB888,
                            handles, pitches, offsets, &bo->fb_id, 0)) {
                    printf( "failed to add fb: %s\n", strerror(errno));
                    break;
                }
                ctx_drm_display(*drm_fd, bo, 0, 0);
                mSfJpegBufInfo[0].bo = bo;
                for (int tmp=MAX_OUTPUT_RSV_CNT-1;tmp>0;tmp--) {
                    mSfJpegBufInfo[tmp].bo = mSfJpegBufInfo[tmp-1].bo;
                }
                mSfJpegBufInfo[0].bo = NULL;

            }
            usleep(m_next_frame_time * 1000);
        }
        if (picPath) {
            free(picPath);
            picPath = NULL;
        }
    } while (m_thread_run);

    for (int i=0;i<MAX_OUTPUT_RSV_CNT;i++) {
        if (mSfJpegBufInfo[i].bo->fd != 0 && mSfJpegBufInfo[i].bo->ptr != NULL) {
            printf("release drm res:i=%d fd=0x%x fb_id=0x%x ptr=%p addr=%p\n", i, mSfJpegBufInfo[i].bo->fd, mSfJpegBufInfo[i].bo->fb_id, mSfJpegBufInfo[i].bo->ptr, mSfJpegBufInfo[i].bo);
            bo_unmap(mSfJpegBufInfo[i].bo);
            bo_destroy((mSfJpegBufInfo[i].bo));
            mSfJpegBufInfo[i].bo = NULL;
        }
    }
    drm_free();
    return 0;
}

int main(int argc, char** argv)
{
    int                     mThreadStatus;
    pthread_t				thread_id;
    int drm_fd = 0;
    int ret=-1;
    FILE* cfg_file = fopen(BOOT_ANIMATION_CONFIG_FILE, "rb");
    int mFps=50;
    if (argc > 1)
        mFps = atoi(argv[1]);

    m_next_frame_time = 1000 / mFps;
    drm_fd = open("/dev/dri/card0", O_RDWR);
    if (drm_fd < 0)
        printf("Failed to open dri 0 \n");
    ret = drmSetClientCap(drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    if (ret)
        printf("[KMS]: can't set UNIVERSAL PLANES cap.\n");

    ret = drmSetClientCap(drm_fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret)
    {
        /*If this happens, check kernel support and kernel parameters 
         * (add i915.nuclear_pageflip=y to the kernel boot line for example) */
        printf ("[KMS]: can't set ATOMIC caps: %s\n", strerror(errno));
    }
    drm_init(drm_fd);

    if (cfg_file) {
        char imageCnt[32];
        char path[128];
        if (!feof(cfg_file)) {
            fgets(imageCnt,32,cfg_file);
            fgets(path,128,cfg_file);
        }
        sscanf(imageCnt, "cnt=%d", &mImageCount);
        sscanf(path, "path=%s", mBootAnimPath);
        printf("cfg: imageCnt=%s path=%s mImageCount=%d mBootAnimPath=%s", imageCnt, path, mImageCount, mBootAnimPath);
        sprintf(mBootAnimPath, "%s", path);
        fclose(cfg_file);
        if (mImageCount <= 0)
            mImageCount = 4;
    } else {
        mImageCount = 4;
        char* tmp = "/media/bootanimation";
        sprintf(mBootAnimPath, "%s", tmp);
    }
    m_thread_run = true;
    mThreadStatus = pthread_create(&thread_id, NULL, bootAnimation, &drm_fd);
    while(1){
        usleep(200*1000);
    }
    close(drm_fd);
    return 0;
}