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
```
echo "Make rootfs"
maindir=`pwd`

mkdir $maindir/build/rootfs
cd $maindir/build/rootfs
dd if=/dev/zero of=rootfs.ext4 count=2097152
mkfs.ext4 rootfs.ext4
sudo mkdir /mnt/tmp
sudo mount rootfs.ext4 /mnt/tmp
sudo qemu-debootstrap --verbose --variant=minbase --arch=armhf --include=vim xenial /mnt/tmp http://ports.ubuntu.com/ubuntu-ports > install.log 2>&1
sudo mount -t proc proc /mnt/tmp/proc
sudo mount -t sysfs sysfs /mnt/tmp/sys
sudo mount -o bind /dev /mnt/tmp/dev
sudo mount -t devpts devpts /mnt/tmp/dev/pts
sudo chroot /mnt/tmp
####
# when you get root prompt run those commands:
#apt-get -y install language-pack-en-base software-properties-common
#apt-add-repository restricted
#apt-add-repository universe
#apt-add-repository multiverse
#apt-get update
#apt-get -y install sudo isc-dhcp-client udev netbase ifupdown iproute openssh-server iputils-ping wget net-tools wireless-tools wpasupplicant ntpdate ntp less tzdata console-common module-init-tools
#echo "ubuntu" | tee /etc/hostname
#echo "127.0.0.1 ubuntu" | tee -a /etc/hosts
#echo "auto eth0" | tee -a /etc/network/interfaces
#echo "    iface eth0 inet dhcp" | tee -a /etc/network/interfaces
#adduser ubuntu
#gpasswd -a ubuntu sudo
#exit
####
sudo umount /mnt/tmp/{proc,sys,dev/pts,dev,}
echo "Done"
```

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
