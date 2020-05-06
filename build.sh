#!/bin/bash

echo "Install arm compiler"
sudo apt-get install git build-essential gcc-arm-linux-gnueabihf lzop libncurses5-dev libssl-dev bc

if [ -d "./build" ]
then
	echo "Build dir exists, please remove it before proceeding"
	exit 1
fi

maindir=`pwd`

echo "mkdir build"
mkdir $maindir/build

echo "Compile mkbootimg"
cd $maindir/rockchip-mkbootimg
make

echo "Compile kernel"
cd $maindir/kernel
mkdir modules
cp $maindir/defconfig/rockchip_defconfig arch/arm/configs/rockchip_defconfig
cp $maindir/dts/rk3188-radxarock-lite.dts arch/arm/boot/dts/rk3188-radxarock.dts

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
export INSTALL_MOD_PATH=./modules

make rockchip_defconfig
make zImage dtbs
make modules
make modules_install

cp arch/arm/boot/zImage $maindir/build/
cp arch/arm/boot/dts/rk3188-radxarock.dtb $maindir/build/

echo "Compress initrd"
cd $maindir/initrd
#cp ../kernel/modules/lib/modules/4.4.221 lib/modules/ -r
find . ! -path "./.git*"  | cpio -H newc  -ov > $maindir/build/initrd.img

cd $maindir/build
gzip initrd.img
cat zImage rk3188-radxarock.dtb > zImage-dtb
$maindir/rockchip-mkbootimg/mkbootimg --kernel zImage-dtb --ramdisk initrd.img.gz -o boot.img

if [ ! -d "$maindir/arm-eabi-4.6" ]
then
	echo "Download u-boot required toolchain"
	mkdir $maindir/arm-eabi-4.6
	cd $maindir/arm-eabi-4.6
	wget https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/+archive/refs/heads/kitkat-release.tar.gz
	tar xvfz kitkat-release.tar.gz
fi

echo "Compile u-boot"
cd $maindir/u-boot-rockchip
export ARCH=arm
export CROSS_COMPILE=$maindir/arm-eabi-4.6/bin/arm-eabi-
make rk30xx
./pack-sd.sh
cp u-boot-sd.img $maindir/build/

echo "Compile rkcrc"
cd $maindir/rkflashtool
gcc -o $maindir/build/rkcrc rkcrc.c

echo "Make parameter image"
cd $maindir/parameter
$maindir/build/rkcrc -p parameter_linux_sd $maindir/build/parameter.img

export LC_ALL=C

dd if=/dev/zero of=$maindir/build/rootfs.ext4 count=2097152
mkfs.ext4 $maindir/build/rootfs.ext4
sudo mkdir /mnt/tmp
sudo mount $maindir/build/rootfs.ext4 /mnt/tmp
sudo qemu-debootstrap --verbose --variant=minbase --arch=armhf --include=vim xenial /mnt/tmp http://ports.ubuntu.com/ubuntu-ports

sudo mount -t proc proc /mnt/tmp/proc
sudo mount -t sysfs sysfs /mnt/tmp/sys
sudo mount -o bind /dev /mnt/tmp/dev
sudo mount -t devpts devpts /mnt/tmp/dev/pts

sudo chroot /mnt/tmp /bin/bash -x <<'EOF'
DEBIAN_FRONTEND=noninteractive apt-get -yq install language-pack-en-base software-properties-common
apt-add-repository restricted
apt-add-repository universe
apt-add-repository multiverse
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get -yq install sudo isc-dhcp-client udev netbase ifupdown iproute openssh-server iputils-ping wget net-tools wireless-tools wpasupplicant ntpdate ntp less tzdata console-common module-init-tools usbutils
echo "ubuntu" | tee /etc/hostname
echo "127.0.0.1 ubuntu" | tee -a /etc/hosts
echo "auto eth0" | tee -a /etc/network/interfaces
echo "    iface eth0 inet dhcp" | tee -a /etc/network/interfaces
adduser ubuntu --gecos "First Last,RoomNumber,WorkPhone,HomePhone" --disabled-password
echo "ubuntu:linux" | chpasswd
gpasswd -a ubuntu sudo
EOF

sudo mkdir -p /mnt/tmp/lib/modules
sudo cp $maindir/kernel/modules/lib/modules/* /mnt/tmp/lib/modules/ -r

sudo umount /mnt/tmp/{proc,sys,dev/pts,dev,}

export IMAGENAME="radxa-rock-lite-kernel3.18-ubuntu16-sdcard.img"
echo "Make sdcard image $IMAGENAME"
cd $maindir/build
dd if=/dev/zero of=$IMAGENAME bs=1M count=1200

export START_SECTOR=131072
fdisk $IMAGENAME  << EOF
n
p
1
$START_SECTOR

w
EOF

dd if=u-boot-sd.img of=$IMAGENAME seek=64                 conv=notrunc
dd if=parameter.img of=$IMAGENAME seek=$((0x2000))        conv=notrunc
dd if=boot.img      of=$IMAGENAME seek=$((0x2000+0x2000)) conv=notrunc
dd if=rootfs.ext4   of=$IMAGENAME seek=$START_SECTOR      conv=notrunc

echo "Done"

