# QEMU 构建
export CROSS_COMPILE=loongarch32r-linux-gnuf32-; \
export ARCH=loongarch; \
make la32_defconfig; \
make vmlinux -j; \
cp vmlinux ../qemu_work/vmlinux; \
loongarch32r-linux-gnusf-strip vmlinux

# Wired SoC 构建
export CROSS_COMPILE=loongarch32r-linux-gnuf32-; \
export ARCH=loongarch; \
make wired_defconfig; \
make vmlinux -j; \
cp vmlinux ../qemu_work/vmlinux; \
loongarch32r-linux-gnusf-strip vmlinux; \
cp vmlinux /mnt/d/BITSTREAM

export CROSS_COMPILE=loongarch32r-linux-gnuf32-; \
export ARCH=loongarch; \
make menuconfig; \
make savedefconfig

# Wired SoC 联网配置
ip link set eth0 down
ip link set eth0 up