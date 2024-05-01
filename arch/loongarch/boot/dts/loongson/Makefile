# SPDX-License-Identifier: GPL-2.0
dtb-$(CONFIG_LS_SOC) += loongson32_ls.dtb
dtb-$(CONFIG_BX_SOC) += loongson32_bx.dtb
dtb-$(CONFIG_WIRED_SOC) += loongson32_wired.dtb
obj-y				+= $(patsubst %.dtb, %.dtb.o, $(dtb-y))
