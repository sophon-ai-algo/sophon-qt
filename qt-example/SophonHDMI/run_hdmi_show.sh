#!/bin/sh -x
fl2000=$(lsmod | grep fl2000 | awk '{print $1}')

echo $fl2000
if [ "$fl2000" != "fl2000" ]; then
	echo "insmod fl2000"
else
	echo "fl2000 already insmod"
fi

cp -r ./bm_bin /bm_bin
export PATH=$PATH:/system/bin:/bm_bin
export QTDIR=./install/lib #qtsdk在系统上的路径
export QT_QPA_FONTDIR=$QTDIR/fonts 
export QT_QPA_PLATFORM_PLUGIN_PATH=./install/plugins/ 
export LD_LIBRARY_PATH=/system/lib:./install/lib:$LD_LIBRARY_PATH 
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fl2000-0 #framebuffer驱动
export QWS_MOUSE_PROTO=/dev/input/event3
./SophUI
