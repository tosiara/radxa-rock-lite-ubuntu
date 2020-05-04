# radxa-rock-lite-ubuntu
Build Ubuntu SD card image for Radxa Rock Lite

Based on the following links:

* https://wiki.radxa.com/Rock/Linux_Mainline
* https://wiki.radxa.com/Rock/U-Boot#For_SD_card
* https://wiki.radxa.com/Rock/make_sd_image

Kernel version: 4.4.221
Ubuntu version: latest 16.04

## Build

`./build.sh`

This will build 3 files:

* u-boot-sd.img
* parameter.im
* boot.img

## Root FS

* https://github.com/sgjava/ubuntu-mini#create-ubuntu-root-filesystem-mk808-mk802iv-etc

## Prepare SD card and flash
```
dd if=/dev/zero of=/dev/disk/by-id/YOURDISK bs=1M count=1
export START_SECTOR=131072

fdisk /dev/disk/by-id/YOURDISK  << EOF
n
p
1
$START_SECTOR

w
EOF

dd if=u-boot-sd.img of=/dev/disk/by-id/YOURDISK conv=sync seek=64 
dd if=parameter.img of=/dev/disk/by-id/YOURDISK conv=sync seek=$((0x2000))
dd if=boot.img of=/dev/disk/by-id/YOURDISK conv=sync seek=$((0x2000+0x2000))
dd if=rootfs.ext4 of=/dev/disk/by-id/YOURDISK-part1 conv=sync
```
