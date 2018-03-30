#! /bin/sh

#Parsing the SN from the cmdline
cd /
cmdline=`cat /proc/cmdline`
result=$(echo $cmdline | grep "serialno=")
if [[ "$result" == "" ]];then
        echo "#WARINING:Parsing the SN from the cmdline failed!"
		echo "          Use default sn:123456789"
        snstr=123456789
else
	fstr=`echo ${cmdline##*serialno=}`
	snstr=`echo ${fstr%% skip_initramfs*}`
fi

#USB gadget configfs (adb function)
cd /
if [ ! -d "/config" ];then
	mkdir config
fi

mount -t configfs none /config

#create directory "g1"£¬create gadget template£º
mkdir /config/usb_gadget/g1

#define VendorID and ProductID
echo 0x2207 > /config/usb_gadget/g1/idVendor
echo 0x0310 > /config/usb_gadget/g1/bcdDevice
echo 0x0200 > /config/usb_gadget/g1/bcdUSB

#Instantiation ID
mkdir /config/usb_gadget/g1/strings/0x409

#write serialnumber,manufacturer,product
echo $snstr > /config/usb_gadget/g1/strings/0x409/serialnumber
echo "rockchip"   > /config/usb_gadget/g1/strings/0x409/manufacturer
echo "rk3229-echo" > /config/usb_gadget/g1/strings/0x409/product

#Create adb function
mkdir /config/usb_gadget/g1/functions/ffs.adb

#USB Configure
mkdir /config/usb_gadget/g1/configs/b.1
mkdir /config/usb_gadget/g1/configs/b.1/strings/0x409
echo 0x1       >  /config/usb_gadget/g1/os_desc/b_vendor_code
echo "MSFT100" >  /config/usb_gadget/g1/os_desc/qw_sign
echo  500      >  /config/usb_gadget/g1/configs/b.1/MaxPower
ln -s  /config/usb_gadget/g1/configs/b.1 /config/usb_gadget/g1/os_desc/b.1

#adb create and configure
mkdir /dev/usb-ffs
mkdir /dev/usb-ffs/adb
mount -t functionfs adb /dev/usb-ffs/adb -o uid=2000,gid=2000

#ln misc parttion.
rm -f /dev/PartNo 
mkdir /dev/PartNo              
#if [ -f /dev/mmcblk0p11 ]; then               
        ln -s /dev/mmcblk0p11 /dev/PartNo/misc
#else                                          
#        ln -s /dev/mmcblk1p11 /dev/PartNo/misc
#fi

echo_pcbatest_server &

#Start auto test items.
#echo_auto_test echo_wlan_test &
echo_auto_test echo_bt_test &
echo_auto_test echo_ddr_test &
echo_auto_test echo_emmc_test &
echo_auto_test echo_rtc_test &

#waiting for kernel config usb USB_SDP_CHARGER(6.6s)
#sleep 7
source /data/adb.sh
echo_uevent_detect &
