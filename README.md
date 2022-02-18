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

