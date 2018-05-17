# vim: syntax=cmake
cmake_minimum_required(VERSION 2.8.11)

include_directories(.)
include_directories("/home/asx/linux/3128h/external/libjpeg_hw/codec")
include_directories("/home/asx/linux/3128h/external/libjpeg_hw/codec/src_dec")
include_directories("/home/asx/linux/3128h/external/libjpeg_hw/codec/inc")
include_directories("/home/asx/linux/3128h/buildroot/output/host/usr/arm-rockchip-linux-gnueabihf/sysroot/usr/include"
					"/home/asx/linux/3128h/buildroot/output/host/usr/arm-rockchip-linux-gnueabihf/sysroot/usr/include/libdrm")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

link_libraries("libdrm.so")
link_libraries("libjpeg.so")
link_libraries("libpthread.so")
#link_libraries("libjpegdec.so")
#link_libraries("libcutils.so")
set(BOOT_SET_H
	drm_common.h
)
add_executable(bootanimation
	drm_common.cpp
    bootanimation_main.cpp
    )

INSTALL(TARGETS bootanimation RUNTIME DESTINATION bin)
