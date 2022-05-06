# sophon-qt

.
├── doc

│   └── AIBOX-qt配置.pdf  ----qt creator configure document

├── LICENSE

├── qt-example

│   ├── SophonHDMI        ----qt app, run script and qt runtime lib for se5. 

│   ├── SophonHDMI.tgz    ----SophonHDMI tgz

│   ├── SophUI     -----qt app example source code

│   └── SophUI2    -----qt app example source code 

├── qt-lib

│   ├── qt5.14-ubuntu16.04.tgz   ----x86 cross complie lib for ubuntu16.04 version

│   ├── qt5.14-ubuntu18.04.tgz   ----x86 cross complie lib for ubuntu18.04 version

│   └── qtbase-5.14.tgz          ----x86 cross complie lib for ubuntu20.04 version

└── README.md


Q: how to complie qt app?

A: read AIBOX-qt配置.pdf

Q: how to run qt app into SE5?

A: cp SophonHDMI.tgz to SE5. 
   1.tar -zxf SophonHDMI.tgz 
   2.cd SophonHDMI
   3.sudo -s; ./run_hdmi_show.sh

Q：hdmi no output, no error log (kenerl log)?

A: hdmi only output 1080p 60hz，don't use vga transform，becasuse vga maybe not support 1080p.  

Q: ERROR "Failed to open framebuffer /dev/fl2000-0 (No such file or directory)   linuxfb:Failed to initialize screen"
A:  首先使用 lsmod 命令检查 fl2000驱动是否加载。如果没有加载使用 sudo insmod /system/data/fl2000.ko手动加载
