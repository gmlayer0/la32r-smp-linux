// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020-2021 Loongson Technology Corporation Limited
 */
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irqchip.h>
#include <linux/irqdomain.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/stddef.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/loongarchregs.h>
#include <loongson.h>

void mach_irq_dispatch(unsigned int pending)
{
	if (pending & 0x4)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 2);
	if (pending & 0x8)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 3);
	if (pending & 0x10)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 4);
	if (pending & 0x20)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 5);
	if (pending & 0x40)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 6);
	if (pending & 0x80)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 7);
	if (pending & 0x100)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 8);
	if (pending & 0x200)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 9);
	// if (pending & 0x400)
	// 	do_IRQ(LOONGSON_CPU_IRQ_BASE + 10); // Performance Counter
	if (pending & 0x800)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 11);
	if (pending & 0x1000)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 12); // IPI
	// if (pending & 0x4)
	// 	do_IRQ(LOONGSON_GMAC_IRQ) ; //in fact , it's for ethernet
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending;
	pending = read_csr_estat() & read_csr_ecfg();
	/* machine-specific plat_irq_dispatch */
	mach_irq_dispatch(pending);
}

void __init setup_IRQ(void)
{
	irqchip_init();
}

void __init arch_init_irq(void)
{
	clear_csr_ecfg(ECFG0_IM);
	clear_csr_estat(ESTATF_IP);

	setup_IRQ();

	set_csr_ecfg(ECFGF_IP0 | ECFGF_IP1 | ECFGF_IP2 | ECFGF_IP3 | ECFGF_IP4 |
	             ECFGF_IP5 | ECFGF_IP6 | ECFGF_IP7 | ECFGF_PC  | ECFGF_TIMER |
				 ECFGF_IPI);
}
