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

# 安装头文件
export CROSS_COMPILE=loongarch32r-linux-gnuf32-; \
export ARCH=loongarch; \
make la32_defconfig; \
make headers_install INSTALL_HDR_PATH=/home/dofingert/os/busyroot/la32_toolchain/install/sysroot/usr

# Wired SoC 联网配置
ip link set eth0 down
ip link set eth0 up