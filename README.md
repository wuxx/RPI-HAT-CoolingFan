# RPI-HAT-CoolingFan
* [产品介绍](#产品介绍) 
* [产品特点](#产品特点)
* [使用步骤](#使用步骤)
* [产品链接](#产品链接)
* [FAQ](#faq)
    
# 产品介绍
这是是缪斯实验室推出的针对树莓派4解决其散热问题的散热风扇扩展板，数码管显示温度及IP地址，带温控，硬件/软件开源。由于树莓派4的使用定位为小型计算机，然而单板上并无任何散热措施，一旦系统负载过大（如打开较多网页、播放视频等），树莓派温度则会立即上升至80度左右，由于CPU自带温度监控，一旦温度过大，则会降频运行，甚至直接卡住或者死机，严重影响可用性。一般可使用散热片或者散热风扇的形式进行降温，使用散热风扇进行主动降温的效果是最好的，经过实测，使用散热风扇扩展板在系统高负载时可降低温度20-30度，可有效提升系统性能，改善使用体验。


# 产品特点
- 数码管实时显示温度和IP地址
- 温度门限控制，超过门限则开启散热风扇
- 硬件开源 
- 软件开源，可根据自己需求修改参数

# 使用步骤
仓库目录下文件如下：  
```
pi@raspberrypi:~/oss/RPI-HAT-CoolingFan $ ls  
coolingfan-daemon  LICENSE  main.c  main.o  Makefile  README.md  
```
只需将可执行文件coolingfan-daemon拷贝至系统启动脚本中，即可实现开机自动运行。  
推荐加至如下位置：  
```
/etc/xdg/lxsession/LXDE-pi/sshpwd.sh  
```
举例如下：  
```
#!/bin/bash

export TEXTDOMAIN=pprompt

. gettext.sh

/home/pi/oss/RPI-HAT-CoolingFan/coolingfan-daemon

if [ -e /run/sshwarn ] ; then
    zenity --warning --width=400 --text="$(gettext "SSH is enabled and the default password for the 'pi' user has not been changed.\n\nThis is a security risk - please login as the 'pi' user and run Raspberry Pi Configuration to set a new password.")"
fi
```

# 产品链接
[树莓派扩展板-散热风扇](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-21349689053.3.4f8d20f8MryK8Q&id=596673065140)

# FAQ

stress:
stress --cpu 8 --io 4 --vm 2 --vm-bytes 128M --timeout 100s

