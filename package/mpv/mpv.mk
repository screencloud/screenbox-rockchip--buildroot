################################################################################
#
# mpv
#
################################################################################

MPV_VERSION = 0.28.2
MPV_SITE = https://github.com/mpv-player/mpv/archive
MPV_SOURCE = v$(MPV_VERSION).tar.gz
MPV_DEPENDENCIES = \
	host-pkgconf ffmpeg3 zlib libgles libegl libdrm lua luainterpreter\
	$(if $(BR2_PACKAGE_LIBICONV),libiconv)
MPV_LICENSE = GPL-2.0+
MPV_LICENSE_FILES = LICENSE

MPV_NEEDS_EXTERNAL_WAF = YES

# Some of these options need testing and/or tweaks
MPV_CONF_OPTS = \
	--prefix=/usr \
	--disable-libsmbclient \
	--disable-apple-remote \
	--enable-drmprime \
	--enable-drm \
	--enable-gbm \
	--enable-egl-drm \
	--enable-lua

$(eval $(waf-package))
