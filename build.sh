#!/bin/bash

echo "Install arm compiler"
sudo apt-get install git build-essential gcc-arm-linux-gnueabihf lzop libncurses5-dev libssl-dev bc

if [ -d "./build" ]
then
	echo "Build dir exists, please remove it before proceeding"
	exit 1
fi

echo "mkdir build"
mkdir build

echo "Compile mkbootimg"
cd rockchip-mkbootimg
make
cd ..

echo "Compile kernel"
cd kernel
mkdir modules
cp ../defconfig/rockchip_defconfig arch/arm/configs/rockchip_defconfig
cp ../dts/rk3188-radxarock-lite.dts arch/arm/boot/dts/rk3188-radxarock.dts

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
export INSTALL_MOD_PATH=./modules

make rockchip_defconfig
make zImage dtbs
make modules
make modules_install

cp arch/arm/boot/zImage ../build/
cp arch/arm/boot/dts/rk3188-radxarock.dtb ../build/

echo "Compress initrd"
cd ../initrd
#cp ../kernel/modules/lib/modules/4.4.221 lib/modules/ -r
find . ! -path "./.git*"  | cpio -H newc  -ov > ../build/initrd.img

cd ../build
gzip initrd.img
cat zImage rk3188-radxarock.dtb > zImage-dtb
../rockchip-mkbootimg/mkbootimg --kernel zImage-dtb --ramdisk initrd.img.gz -o boot.img

cd ..

if [ ! -d "./arm-eabi-4.6" ]
then
	echo "Download u-boot required toolchain"
	mkdir arm-eabi-4.6
	cd arm-eabi-4.6
	wget https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/+archive/refs/heads/kitkat-release.tar.gz
	tar xvfz kitkat-release.tar.gz
	cd ..
fi

echo "Compile u-boot"
cd u-boot-rockchip
export ARCH=arm
export CROSS_COMPILE=`pwd`/../arm-eabi-4.6/bin/arm-eabi-
make rk30xx
./pack-sd.sh
cp u-boot-sd.img ../build/

echo "Compile rkcrc"
cd ../rkflashtool
gcc -o ../build/rkcrc rkcrc.c

echo "Make parameter image"
cd ../parameter
../build/rkcrc -p parameter_linux_sd ../build/parameter.img  

echo "Done"

