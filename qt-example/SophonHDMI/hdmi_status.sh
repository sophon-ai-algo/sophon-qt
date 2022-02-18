#!/bin/sh

count=0
service_state="init"
while true
do

	STATUS=$(cat /sys/devices/platform/5fb80000.pcie/pci0000\:00/0000\:00\:00.0/0000\:01\:00.0/usb2/2-1/2-1.3/2-1.3\:1.2/hdmi_status/status)
	if [ $? -eq 1 ]; then
		STATUS="0"
	fi	
	#echo $STATUS
	if [ $STATUS = "0" ]; then
		#delay stop service for avoid quickly hot-plug
        	count=$(($count + 1))
		if [ $count -gt 60 ] && [ $service_state != "inactive" ]; then
			echo "stop hdmi service"
			systemctl stop SophonHDMI.service
			service_state="inactive"
			count=0
		fi
	else
		count=0
		if [ $service_state != "active" ]; then
			echo "start hdmi service"
			systemctl start SophonHDMI.service
			service_state="active"
		fi
	fi

    sleep 1 
done
