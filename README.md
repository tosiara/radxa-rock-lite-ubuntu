# radxa-rock-lite-ubuntu
Build Ubuntu SD card image for Radxa Rock Lite

Based on the following links:

* https://wiki.radxa.com/Rock/Linux_Mainline
* https://wiki.radxa.com/Rock/U-Boot#For_SD_card
* https://wiki.radxa.com/Rock/make_sd_image
* https://github.com/sgjava/ubuntu-mini#create-ubuntu-root-filesystem-mk808-mk802iv-etc

Kernel version: 4.4.221
Ubuntu version: latest 16.04

## Build

`./build.sh`

This will build 5 files:

* u-boot-sd.img
* parameter.im
* boot.img
* rootfs.ext4
* radxa-rock-lite-kernel4.4-ubuntu16-sdcard.img

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

dd if=radxa-rock-lite-kernel4.4-ubuntu16-sdcard.img of=/dev/disk/by-id/YOURDISK-part1 conv=sync
```
