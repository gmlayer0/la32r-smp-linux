#!/bin/bash

export CROSS_COMPILE=~/install-32-glibc-loongarch-novec-reduce-linux-5-14/bin/loongarch32-linux-gnu-
export ARCH=loongarch
OUT=la_build

echo "----------------output ${OUT}----------------"

make menuconfig O=${OUT}
make vmlinux -j  O=${OUT} 2>&1 | tee -a build_error.log
