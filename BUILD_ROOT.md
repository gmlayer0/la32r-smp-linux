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
loongarch32r-linux-gnuf32-strip vmlinux; \
cp vmlinux /mnt/d/BITSTREAM

# Wired SoC 构建-LA32
export CROSS_COMPILE=loongarch32-linux-gnuf32-; \
export ARCH=loongarch; \
make wired_defconfig; \
make vmlinux -j; \
cp vmlinux ../qemu_work/vmlinux; \
loongarch32-linux-gnuf32-strip vmlinux; \
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

# 代码量统计

git ls-files | xargs cat 2>/dev/null | wc -l; \
cd kernel; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd include; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd mm; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd lib; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd drivers; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd arch; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd fs; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd net; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd tools; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -; \
cd Documentation; pwd; git ls-files | xargs cat 2>/dev/null | wc -l; cd -
