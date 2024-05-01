# QEMU 构建
export CROSS_COMPILE=loongarch32r-linux-gnuf32-; \
export ARCH=loongarch; \
make la32_defconfig; \
make vmlinux -j; \
loongarch32r-linux-gnusf-strip vmlinux; \
cp vmlinux ../qemu_work/vmlinux

export CROSS_COMPILE=loongarch32r-linux-gnuf32-; \
export ARCH=loongarch; \
make wired_defconfig; \
make vmlinux -j; \
cp vmlinux ../qemu_work/vmlinux; \
loongarch32r-linux-gnusf-strip vmlinux

export CROSS_COMPILE=loongarch32r-linux-gnuf32-; \
export ARCH=loongarch; \
make menuconfig; \
make savedefconfig