#!/bin/bash

echo "Install arm compiler"
sudo apt-get install git build-essential gcc-arm-linux-gnueabihf lzop libncurses5-dev libssl-dev bc
sudo -k

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
	mkdir arm-eabi-4.6
	cd arm-eabi-4.6
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

echo "Done"

