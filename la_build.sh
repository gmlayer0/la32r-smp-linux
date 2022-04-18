#!/bin/bash

export CROSS_COMPILE=~/install-32-glibc-loongarch-novec-reduce-linux-5-14/bin/loongarch32-linux-gnu-
export ARCH=loongarch
OUT=la_build

if [ ! -d la_build ] ;then
    mkdir la_build
    make la32_defconfig O=${OUT}
fi

echo "----------------output ${OUT}----------------"

make menuconfig O=${OUT}
make vmlinux -j  O=${OUT} 2>&1 | tee -a build_error.log
