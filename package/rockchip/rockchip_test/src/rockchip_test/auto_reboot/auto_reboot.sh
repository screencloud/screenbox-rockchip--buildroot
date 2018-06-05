#!/bin/sh

delay=10
total=30000
fudev=/dev/sda
CNT=/data/rockchip_test/reboot_cnt

if [ ! -e "/data/rockchip_test" ]; then
	echo "no /data/rockchip_test"
	mkdir /data/rockchip_test
fi

if [ ! -e "/data/rockchip_test/auto_reboot.sh" ]; then
	cp /rockchip_test/auto_reboot/auto_reboot.sh /data/rockchip_test
fi

while true
do

#if [ ! -e "$fudev" ]; then
#    echo "Please insert a U disk to start test!"
#    exit 0
#fi

if [ -e $CNT ]
then
    cnt=`cat $CNT`
else
    echo reset Reboot count.
    echo 0 > $CNT
fi

echo  Reboot after $delay seconds. 

let "cnt=$cnt+1"

if [ $cnt -ge $total ] 
then 
    echo AutoReboot Finisned. 
    echo "off" > $CNT
    echo "do cleaning ..."
    rm -rf /data/rockchip_test/auto_reboot.sh
	rm -f $CNT
    exit 0
fi

echo $cnt > $CNT
echo "current cnt = $cnt"
echo "You can stop reboot by: echo off > /data/rockchip_test/reboot_cnt"
sleep $delay
cnt=`cat $CNT`
if [ $cnt != "off" ]; then
	reboot
else
	echo "Auto reboot is off"
	rm -rf /data/rockchip_test/auto_reboot.sh
	rm -f $CNT
fi
exit 0
done
