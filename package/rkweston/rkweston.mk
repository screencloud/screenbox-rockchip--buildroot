################################################################################
#
# weston
#
################################################################################
RKWESTON_VERSION = master
RKWESTON_SITE = https://github.com/qiuen/rk-weston2.0.git
RKWESTON_SITE_METHOD = git
#WESTON_SOURCE = 
RKWESTON_LICENSE = MIT
RKWESTON_LICENSE_FILES = COPYING

RKWESTON_DEPENDENCIES = host-pkgconf wayland wayland-protocols \
	libxkbcommon pixman libpng jpeg mtdev udev cairo libinput libhdmiset\
	$(if $(BR2_PACKAGE_WEBP),webp)

RKWESTON_CONF_OPTS = \
	--with-dtddir=$(STAGING_DIR)/usr/share/wayland \
	--disable-headless-compositor \
	--disable-colord \
	--disable-devdocs \
	--disable-setuid-install

RKWESTON_MAKE_OPTS = \
	WAYLAND_PROTOCOLS_DATADIR=$(STAGING_DIR)/usr/share/wayland-protocols

# Uses VIDIOC_EXPBUF, only available from 3.8+
ifeq ($(BR2_TOOLCHAIN_HEADERS_AT_LEAST_3_8),)
RKWESTON_CONF_OPTS += --disable-simple-dmabuf-v4l-client
endif

ifeq ($(BR2_PACKAGE_DBUS),y)
RKWESTON_CONF_OPTS += --enable-dbus
RKWESTON_DEPENDENCIES += dbus
else
RKWESTON_CONF_OPTS += --disable-dbus
endif

# weston-launch must be u+s root in order to work properly
ifeq ($(BR2_PACKAGE_LINUX_PAM),y)
define WESTON_PERMISSIONS
	/usr/bin/weston-launch f 4755 0 0 - - - - -
endef
define WESTON_USERS
	- - weston-launch -1 - - - - Weston launcher group
endef
RKWESTON_CONF_OPTS += --enable-weston-launch
RKWESTON_DEPENDENCIES += linux-pam
else
RKWESTON_CONF_OPTS += --disable-weston-launch
endif

# Needs wayland-egl, which normally only mesa provides
ifeq ($(BR2_PACKAGE_MESA3D_OPENGL_EGL)$(BR2_PACKAGE_MESA3D_OPENGL_ES),yy)
RKWESTON_CONF_OPTS += --enable-egl
RKWESTON_DEPENDENCIES += libegl
else
RKWESTON_CONF_OPTS += \
	--disable-egl \
	--disable-simple-egl-clients
endif

ifeq ($(BR2_PACKAGE_LIBUNWIND),y)
RKWESTON_DEPENDENCIES += libunwind
else
RKWESTON_CONF_OPTS += --disable-libunwind
endif

ifeq ($(BR2_PACKAGE_RKWESTON_RDP),y)
RKWESTON_DEPENDENCIES += freerdp
RKWESTON_CONF_OPTS += --enable-rdp-compositor
else
RKWESTON_CONF_OPTS += --disable-rdp-compositor
endif

ifeq ($(BR2_PACKAGE_RKWESTON_FBDEV),y)
RKWESTON_CONF_OPTS += \
	--enable-fbdev-compositor \
	WESTON_NATIVE_BACKEND=fbdev-backend.so
else
RKWESTON_CONF_OPTS += --disable-fbdev-compositor
endif

ifeq ($(BR2_PACKAGE_RKWESTON_DRM),y)
RKWESTON_CONF_OPTS += \
	--enable-drm-compositor \
	WESTON_NATIVE_BACKEND=drm-backend.so
RKWESTON_DEPENDENCIES += libdrm
else
RKWESTON_CONF_OPTS += --disable-drm-compositor
endif

ifeq ($(BR2_PACKAGE_RKWESTON_X11),y)
RKWESTON_CONF_OPTS += \
	--enable-x11-compositor \
	WESTON_NATIVE_BACKEND=x11-backend.so
RKWESTON_DEPENDENCIES += libxcb xlib_libX11
else
RKWESTON_CONF_OPTS += --disable-x11-compositor
endif

ifeq ($(BR2_PACKAGE_RKWESTON_XWAYLAND),y)
RKWESTON_CONF_OPTS += --enable-xwayland
RKWESTON_DEPENDENCIES += cairo libepoxy libxcb xlib_libX11 xlib_libXcursor
else
RKWESTON_CONF_OPTS += --disable-xwayland
endif

ifeq ($(BR2_PACKAGE_LIBVA),y)
RKWESTON_CONF_OPTS += --enable-vaapi-recorder
RKWESTON_DEPENDENCIES += libva
else
RKWESTON_CONF_OPTS += --disable-vaapi-recorder
endif

ifeq ($(BR2_PACKAGE_LCMS2),y)
RKWESTON_CONF_OPTS += --enable-lcms
RKWESTON_DEPENDENCIES += lcms2
else
RKWESTON_CONF_OPTS += --disable-lcms
endif

ifeq ($(BR2_PACKAGE_SYSTEMD),y)
RKWESTON_CONF_OPTS += --enable-systemd-login --enable-systemd-notify
RKWESTON_DEPENDENCIES += systemd
else
RKWESTON_CONF_OPTS += --disable-systemd-login --disable-systemd-notify
endif

ifeq ($(BR2_PACKAGE_LIBXML2),y)
RKWESTON_CONF_OPTS += --enable-junit-xml
RKWESTON_DEPENDENCIES += libxml2
else
RKWESTON_CONF_OPTS += --disable-junit-xml
endif

ifeq ($(BR2_PACKAGE_RKWESTON_DEMO_CLIENTS),y)
RKWESTON_CONF_OPTS += --enable-demo-clients-install
else
WESTON_CONF_OPTS += --disable-demo-clients-install
endif

$(eval $(autotools-package))
