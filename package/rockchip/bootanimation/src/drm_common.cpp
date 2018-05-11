#include "drm_common.h"
#include <unistd.h>
#include <string.h>
#include <string>
#include <stdio.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
struct pollfd g_drm_fds;

uint32_t g_connector_id               = 0;
int g_drm_fd                          = 0;
uint32_t g_crtc_id                    = 0;

static drmModeCrtc *g_orig_crtc       = NULL;

drmModeRes *g_drm_resources    = NULL;
drmModeConnector *g_drm_connector     = NULL;
drmModeModeInfo *g_drm_mode           = NULL;
drmModeEncoder *g_drm_encoder  = NULL;
drmEventContext g_drm_evctx;

/* Restore the original CRTC. */
void drm_restore_crtc(void)
{
   if (!g_orig_crtc)
      return;

   drmModeSetCrtc(g_drm_fd, g_orig_crtc->crtc_id,
         g_orig_crtc->buffer_id,
         g_orig_crtc->x,
         g_orig_crtc->y,
         &g_connector_id, 1, &g_orig_crtc->mode);

   drmModeFreeCrtc(g_orig_crtc);
   g_orig_crtc = NULL;
}

bool drm_get_resources(int fd)
{
   g_drm_resources = drmModeGetResources(fd);
   if (!g_drm_resources)
   {
      printf("[DRM]: Couldn't get device resources.\n");
      return false;
   }

   return true;
}

bool drm_get_connector(int fd)
{
   unsigned i;
   unsigned monitor_index = 0;
   /* Enumerate all connectors. */

   printf("[DRM]: Found %d connectors.\n", g_drm_resources->count_connectors);

   for (i = 0; (int)i < g_drm_resources->count_connectors; i++)
   {
      drmModeConnectorPtr conn = drmModeGetConnector(
            fd, g_drm_resources->connectors[i]);

      if (conn)
      {
         bool connected = conn->connection == DRM_MODE_CONNECTED;
         printf("[DRM]: Connector %d connected: %s\n", i, connected ? "yes" : "no");
         printf("[DRM]: Connector %d has %d modes.\n", i, conn->count_modes);
         if (connected && conn->count_modes > 0)
         {
            monitor_index++;
            printf("[DRM]: Connector %d assigned to monitor index: #%u.\n", i, monitor_index);
         }
         drmModeFreeConnector(conn);
      }
   }

   monitor_index = 0;

   for (i = 0; (int)i < g_drm_resources->count_connectors; i++)
   {
      g_drm_connector = drmModeGetConnector(fd,
            g_drm_resources->connectors[i]);

      if (!g_drm_connector)
         continue;
      if (g_drm_connector->connection == DRM_MODE_CONNECTED
            && g_drm_connector->count_modes > 0)
      {
         monitor_index++;
		 break;
      }

      drmModeFreeConnector(g_drm_connector);
      g_drm_connector = NULL;
   }

   if (!g_drm_connector)
   {
      printf("[DRM]: Couldn't get device connector.\n");
      return false;
   }
   return true;
}

bool drm_get_encoder(int fd)
{
   unsigned i;

   for (i = 0; (int)i < g_drm_resources->count_encoders; i++)
   {
      g_drm_encoder = drmModeGetEncoder(fd, g_drm_resources->encoders[i]);

      if (!g_drm_encoder)
         continue;

      if (g_drm_encoder->encoder_id == g_drm_connector->encoder_id)
         break;

      drmModeFreeEncoder(g_drm_encoder);
      g_drm_encoder = NULL;
   }

   if (!g_drm_encoder)
   {
      printf("[DRM]: Couldn't find DRM encoder.\n");
      return false;
   }

   for (i = 0; (int)i < g_drm_connector->count_modes; i++)
   {
      printf("[DRM]: Mode %d: (%s) %d x %d, %u Hz\n",
            i,
            g_drm_connector->modes[i].name,
            g_drm_connector->modes[i].hdisplay,
            g_drm_connector->modes[i].vdisplay,
            g_drm_connector->modes[i].vrefresh);
   }

   return true;
}

void drm_update(int fd)
{
	unsigned i;
	for (i = 0; (int)i < g_drm_resources->count_connectors; i++)
	{
	   g_drm_connector = drmModeGetConnector(fd,
			 g_drm_resources->connectors[i]);
	
	   if (!g_drm_connector)
		  continue;
	   if (g_drm_connector->connection == DRM_MODE_CONNECTED
			 && g_drm_connector->count_modes > 0)
	   {
		  break;
	   }
	   drmModeFreeConnector(g_drm_connector);
	   g_drm_connector = NULL;
	}

	if (!g_drm_connector)
	{
	   printf("[DRM]: Couldn't get device connector.\n");
	   return;
	}
	
	for (i = 0; (int)i < g_drm_resources->count_encoders; i++)
	{
	   g_drm_encoder = drmModeGetEncoder(fd, g_drm_resources->encoders[i]);
	
	   if (!g_drm_encoder)
		  continue;
	
	   if (g_drm_encoder->encoder_id == g_drm_connector->encoder_id)
		  break;
	
	   drmModeFreeEncoder(g_drm_encoder);
	   g_drm_encoder = NULL;
	}
}

void drm_free(void)
{
   if (g_drm_encoder)
      drmModeFreeEncoder(g_drm_encoder);
   if (g_drm_connector)
      drmModeFreeConnector(g_drm_connector);
   if (g_drm_resources)
      drmModeFreeResources(g_drm_resources);

   memset(&g_drm_fds,     0, sizeof(struct pollfd));
   memset(&g_drm_evctx,   0, sizeof(drmEventContext));

   g_drm_encoder      = NULL;
   g_drm_connector    = NULL;
   g_drm_resources    = NULL;
}
