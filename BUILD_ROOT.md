# QEMU 构建
export CROSS_COMPILE=loongarch32r-linux-gnusf-; \
export ARCH=loongarch; \
make la32_defconfig; \
make vmlinux -j; \
loongarch32r-linux-gnusf-strip vmlinux; \
cp vmlinux ../qemu_work/vmlinux

export CROSS_COMPILE=loongarch32r-linux-gnusf-; \
export ARCH=loongarch; \
make menuconfig; \
make savedefconfig