#!/bin/sh
### file: stress_test_dvfs.sh
### author: xxx@rock-chips.com
### function: cpu dvfs stress test
### date: 20180409

echo "**********************stress dvfs test****************************"

url=$1
echo "url: $url"

#bootid=`ps |grep bootanimation | busybox awk '{ print $2 }'`
killall  bootanimation

mpv -vo=gpu -gpu-context=drm -hwdec=rkmpp -cache-secs=10 -demuxer-seekable-cache=no $url

bootanimation &



