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
	if (pending & 0x800)
		do_IRQ(LOONGSON_TIMER_IRQ);
#ifndef CONFIG_BX_SOC
	if (pending & 0x20)
		do_IRQ(4);
	if (pending & 0x4)
		do_IRQ(LOONGSON_GMAC_IRQ) ; //in fact , it's for ehternet
	if (pending & 0x8)
		do_IRQ(LOONGSON_UART_IRQ);
#else
	else if (pending & 0x4)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 2); 	/* IP2 */
	else if (pending & 0x8)
		do_IRQ(LOONGSON_CPU_IRQ_BASE + 3); 	/* IP2 */
#endif
}

asmlinkage void plat_irq_dispatch(int irq)
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

	set_csr_ecfg(ECFGF_IP0 | ECFGF_IP1 |ECFGF_IP2 | ECFGF_IP3| ECFGF_IPI | ECFGF_PC);
}
