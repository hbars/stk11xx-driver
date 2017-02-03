stk11xx-driver

The syntek USB 2.0 video camera driver for DC-1125 ans STK-1135.
Build with latest kernels. I'm try with 4.8.x.

The driver supports several webcam models :
- 0x174F:0xA311 : Quiet good supported (developper's model)
- 0x174F:0xA821 : Supported (only the video stream)
- 0x174F:0x6A31 : Supported (only the video stream)
- 0x174F:0x6A33 : Supported (only the video stream) Syntek Web Cam - Asus F3SA, F9J, F9S (my model)
- 0x174F:0x6A51 : Supported (only the video stream)
- 0x174F:0x6A54 : Supported (only the video stream)
- 0x174F:0x6D51 : Supported (only the video stream)
- 0x05E1:0x0500 :
- 0x05E1:0x0501 : Like '0x174F:0xA311' (it's the same model)

sudo apt install ctags
make -f Makefile.standalone driver
sudo mkdir -p /lib/modules/`uname -r`/kernel/drivers/media/usb/stk11xx
sudo install -m 644 -o 0 -g 0 stk11xx.ko /lib/modules/`uname -r`/kernel/drivers/media/usb/stk11xx
sudo depmod -a
sudo modprobe stk11xx

Injoy.

ps: Have a same problems whith skype.
