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

struct acpi_madt_lio_pic *acpi_liointc;
struct acpi_madt_eio_pic *acpi_eiointc;
struct acpi_madt_ht_pic *acpi_htintc;
struct acpi_madt_lpc_pic *acpi_pchlpc;
struct acpi_madt_msi_pic *acpi_pchmsi;
struct acpi_madt_bio_pic *acpi_pchpic[MAX_PCH_PICS];

struct fwnode_handle *acpi_liointc_handle;
struct fwnode_handle *acpi_msidomain_handle;
struct fwnode_handle *acpi_picdomain_handle[MAX_PCH_PICS];

#ifdef CONFIG_BX_SOC // TODO: move BaiXin Interrupt Controller code to /drivers/irqchip
#define LS1X_IRQ_BASE			LOONGSON_CPU_IRQ_BASE + 13
#define LS1X_IRQ(n, x)			(LS1X_IRQ_BASE + (n << 5) + (x))

#define LS1X_INTC_REG(n, x) \
		((void __iomem *)(0x9fd01040 + (n * 0x18) + (x)))

#define LS1X_INTC_INTISR(n)		LS1X_INTC_REG(n, 0x0)
#define LS1X_INTC_INTIEN(n)		LS1X_INTC_REG(n, 0x4)
#define LS1X_INTC_INTSET(n)		LS1X_INTC_REG(n, 0x8)
#define LS1X_INTC_INTCLR(n)		LS1X_INTC_REG(n, 0xc)
#define LS1X_INTC_INTPOL(n)		LS1X_INTC_REG(n, 0x10)
#define LS1X_INTC_INTEDGE(n)		LS1X_INTC_REG(n, 0x14)

static void ls1x_irq_ack(struct irq_data *d)
{
	unsigned int bit = (d->irq - LS1X_IRQ_BASE) & 0x1f;
	unsigned int n = (d->irq - LS1X_IRQ_BASE) >> 5;

//	printk("========== %s====\n", __func__);
	__raw_writel(__raw_readl(LS1X_INTC_INTCLR(n))
			| (1 << bit), LS1X_INTC_INTCLR(n));
}

static void ls1x_irq_mask(struct irq_data *d)
{
	unsigned int bit = (d->irq - LS1X_IRQ_BASE) & 0x1f;
	unsigned int n = (d->irq - LS1X_IRQ_BASE) >> 5;

//	printk("========== %s====\n", __func__);
	__raw_writel(__raw_readl(LS1X_INTC_INTIEN(n))
			& ~(1 << bit), LS1X_INTC_INTIEN(n));
}

static void ls1x_irq_mask_ack(struct irq_data *d)
{
	unsigned int bit = (d->irq - LS1X_IRQ_BASE) & 0x1f;
	unsigned int n = (d->irq - LS1X_IRQ_BASE) >> 5;

//	printk("========== %s====\n", __func__);
	__raw_writel(__raw_readl(LS1X_INTC_INTIEN(n))
			& ~(1 << bit), LS1X_INTC_INTIEN(n));
	__raw_writel(__raw_readl(LS1X_INTC_INTCLR(n))
			| (1 << bit), LS1X_INTC_INTCLR(n));
}

static void ls1x_irq_unmask(struct irq_data *d)
{
	unsigned int bit = (d->irq - LS1X_IRQ_BASE) & 0x1f;
	unsigned int n = (d->irq - LS1X_IRQ_BASE) >> 5;

//	printk("========== %s====\n", __func__);
	__raw_writel(__raw_readl(LS1X_INTC_INTIEN(n))
			| (1 << bit), LS1X_INTC_INTIEN(n));
}

static int ls1x_irq_settype(struct irq_data *d, unsigned int type)
{
	unsigned int bit = (d->irq - LS1X_IRQ_BASE) & 0x1f;
	unsigned int n = (d->irq - LS1X_IRQ_BASE) >> 5;

	printk("========== %s====\n", __func__);
	switch (type) {
	case IRQ_TYPE_LEVEL_HIGH:
		__raw_writel(__raw_readl(LS1X_INTC_INTPOL(n))
			| (1 << bit), LS1X_INTC_INTPOL(n));
		__raw_writel(__raw_readl(LS1X_INTC_INTEDGE(n))
			& ~(1 << bit), LS1X_INTC_INTEDGE(n));
		break;
	case IRQ_TYPE_LEVEL_LOW:
		__raw_writel(__raw_readl(LS1X_INTC_INTPOL(n))
			& ~(1 << bit), LS1X_INTC_INTPOL(n));
		__raw_writel(__raw_readl(LS1X_INTC_INTEDGE(n))
			& ~(1 << bit), LS1X_INTC_INTEDGE(n));
		break;
	case IRQ_TYPE_EDGE_RISING:
		__raw_writel(__raw_readl(LS1X_INTC_INTPOL(n))
			| (1 << bit), LS1X_INTC_INTPOL(n));
		__raw_writel(__raw_readl(LS1X_INTC_INTEDGE(n))
			| (1 << bit), LS1X_INTC_INTEDGE(n));
		break;
	case IRQ_TYPE_EDGE_FALLING:
		__raw_writel(__raw_readl(LS1X_INTC_INTPOL(n))
			& ~(1 << bit), LS1X_INTC_INTPOL(n));
		__raw_writel(__raw_readl(LS1X_INTC_INTEDGE(n))
			| (1 << bit), LS1X_INTC_INTEDGE(n));
		break;
	case IRQ_TYPE_EDGE_BOTH:
		__raw_writel(__raw_readl(LS1X_INTC_INTPOL(n))
			& ~(1 << bit), LS1X_INTC_INTPOL(n));
		__raw_writel(__raw_readl(LS1X_INTC_INTEDGE(n))
			| (1 << bit), LS1X_INTC_INTEDGE(n));
		break;
	case IRQ_TYPE_NONE:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct irq_chip ls1x_irq_chip = {
	.name		= "LS1X-INTC",
	.irq_ack	= ls1x_irq_ack,
	.irq_mask	= ls1x_irq_mask,
	.irq_mask_ack	= ls1x_irq_mask_ack,
	.irq_unmask	= ls1x_irq_unmask,
	.irq_set_type   = ls1x_irq_settype,
};

#define INTN_NEW 2
#define NR_IRQS_NEW 64
static void __init ls1x_irq_init(int base)
{
	int n;

	/* Disable interrupts and clear pending,
	 * setup all IRQs as high level triggered
	 */
	for (n = 0; n < INTN_NEW; n++) {
		__raw_writel(0x0, LS1X_INTC_INTIEN(n));
		__raw_writel(0xffffffff, LS1X_INTC_INTCLR(n));
		__raw_writel(0xffffffff, LS1X_INTC_INTPOL(n));
		//__raw_writel(0x1, LS1X_INTC_INTPOL(n));
		/* set DMA0, DMA1 and DMA2 to edge trigger */
		//__raw_writel(n ? 0x0 : 0xe001, LS1X_INTC_INTEDGE(n));			//todo!!
		__raw_writel(n ? 0x0 : 0x0, LS1X_INTC_INTEDGE(n));			//todo!!
		if (0 == n)
			__raw_writel(0x1, LS1X_INTC_INTIEN(n));
		if (1 == n)
			__raw_writel(0x8, LS1X_INTC_INTIEN(n));
	}

	for (n = base; n < NR_IRQS_NEW; n++) {
		irq_set_chip_and_handler(n, &ls1x_irq_chip,
					 handle_level_irq);
	}

/* todo */
//	setup_irq(INT0_IRQ, &cascade_irqaction);
//	setup_irq(INT1_IRQ, &cascade_irqaction);
//	setup_percpu_irq(LOONGSON_CPU_IRQ_BASE + 2, &cascade_irqaction);
//	setup_percpu_irq(LOONGSON_CPU_IRQ_BASE + 3, &cascade_irqaction);


}

static void ls1x_irq_dispatch(int n)
{
	u32 int_status, irq;
	//printk("####ls1x interrupt :n=%d\n", n);
	/* Get pending sources, masked by current enables */
	int_status = __raw_readl(LS1X_INTC_INTISR(n)) &
			__raw_readl(LS1X_INTC_INTIEN(n));

	if (int_status) {
		irq = LS1X_IRQ(n, __ffs(int_status));
	//	printk("========== %s====irq is %d\n", __func__, irq);
		do_IRQ(irq);
	}
}
#endif

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
	if (pending & 0x4) {
		ls1x_irq_dispatch(0); 	/* IP2 */
	}
    else if (pending & 0x8) {
		ls1x_irq_dispatch(1); /* IP3 */
	}
#endif
}

asmlinkage void plat_irq_dispatch(int irq)
{
	unsigned int pending;
	pending = read_csr_estat() & read_csr_ecfg();
	/* machine-specific plat_irq_dispatch */
	mach_irq_dispatch(pending);
}

int find_pch_pic(u32 gsi)
{
	int i, start, end;

	/* Find the PCH_PIC that manages this GSI. */
	for (i = 0; i < loongson_sysconf.nr_pch_pics; i++) {
		struct acpi_madt_bio_pic *irq_cfg = acpi_pchpic[i];

		start = irq_cfg->gsi_base;
		end   = irq_cfg->gsi_base + irq_cfg->size;
		if (gsi >= start && gsi < end)
			return i;
	}

	pr_err("ERROR: Unable to locate PCH_PIC for GSI %d\n", gsi);
	return -1;
}

void __init setup_IRQ(void)
{
	irqchip_init();
#ifdef CONFIG_BX_SOC
	ls1x_irq_init(LS1X_IRQ_BASE);
#endif
}

void __init arch_init_irq(void)
{
	clear_csr_ecfg(ECFG0_IM);
	clear_csr_estat(ESTATF_IP);

	setup_IRQ();

	set_csr_ecfg(ECFGF_IP0 | ECFGF_IP1 |ECFGF_IP2 | ECFGF_IP3| ECFGF_IPI | ECFGF_PC);
}
