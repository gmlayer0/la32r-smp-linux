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

void mach_irq_dispatch(unsigned int pending)
{
        if (pending & 0x800)
		do_IRQ(LOONGSON_TIMER_IRQ);
	if (pending & 0x20)
		do_IRQ(4);
	if (pending & 0x4)
		do_IRQ(LOONGSON_BRIDGE_IRQ) ; //in fact , it's for ehternet
	if (pending & 0x8)
		do_IRQ(LOONGSON_LINTC_IRQ);
}

asmlinkage void plat_irq_dispatch(void)
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
}

static inline void mask_loongson_irq(struct irq_data *d) { }
static inline void unmask_loongson_irq(struct irq_data *d) { }

static struct irq_chip loongson_irq_chip = {
	.name           = "Loongson",
	.irq_ack        = mask_loongson_irq,
	.irq_mask       = mask_loongson_irq,
	.irq_mask_ack   = mask_loongson_irq,
	.irq_unmask     = unmask_loongson_irq,
	.irq_eoi        = unmask_loongson_irq,
};


void __init arch_init_irq(void)
{
	clear_csr_ecfg(ECFG0_IM);
	clear_csr_estat(ESTATF_IP);

	setup_IRQ();
	irq_set_chip_and_handler(LOONGSON_LINTC_IRQ,
			&loongson_irq_chip, handle_percpu_irq);

	set_csr_ecfg(ECFGF_IP0 | ECFGF_IP1 |ECFGF_IP2 | ECFGF_IP3| ECFGF_IPI | ECFGF_PC);
}
