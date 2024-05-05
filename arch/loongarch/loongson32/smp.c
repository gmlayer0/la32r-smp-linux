#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/sched/hotplug.h>
#include <linux/sched/task_stack.h>
#include <linux/seq_file.h>
#include <linux/smp.h>
#include <linux/syscore_ops.h>
#include <linux/tracepoint.h>
#include <asm/processor.h>
#include <asm/time.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <loongson.h>

static DEFINE_PER_CPU(int, cpu_state);
DEFINE_PER_CPU_SHARED_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);

#define MAX_CPUS 8

#define STATUS  0x00
#define EN      0x04
#define SET     0x08
#define CLEAR   0x0c
#define MBUF    0x20

extern u32 smp_group[MAX_PACKAGES];
static u32 core_offsets[8] = {0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000};

static volatile void *ipi_set_regs[MAX_CPUS];
static volatile void *ipi_clear_regs[MAX_CPUS];
static volatile void *ipi_status_regs[MAX_CPUS];
static volatile void *ipi_en_regs[MAX_CPUS];
static volatile void *ipi_mailbox_buf[MAX_CPUS];

enum ipi_msg_type {
	IPI_RESCHEDULE,
	IPI_CALL_FUNCTION,
};

static const char *ipi_types[NR_IPI] __tracepoint_string = {
	[IPI_RESCHEDULE] = "Rescheduling interrupts",
	[IPI_CALL_FUNCTION] = "Call Function interrupts",
};

void show_ipi_list(struct seq_file *p, int prec)
{
	unsigned int cpu, i;

	for (i = 0; i < NR_IPI; i++) {
		seq_printf(p, "%*s%u:%s", prec - 1, "IPI", i, prec >= 4 ? " " : "");
		for_each_online_cpu(cpu)
			seq_printf(p, "%10u ", per_cpu(irq_stat, cpu).ipi_irqs[i]);
		seq_printf(p, " LoongArch     %s\n", ipi_types[i]);
	}
}

static u32 ipi_read_clear(int cpu)
{
	u32 action;

	/* Load the ipi register to figure out what we're supposed to do */
	action = xconf_readl(ipi_status_regs[cpu]);
	/* Clear the ipi register to clear the interrupt */
	xconf_writel(action, ipi_clear_regs[cpu]);

	return action;
}

static void ipi_write_action(int cpu, u32 action)
{
	xconf_writel((u32)action, ipi_set_regs[cpu]);
}

static void ipi_regaddrs_init(void)
{
	int i, core;
	for (i = 0; i < MAX_CPUS; i++) {
		core = i;
		ipi_set_regs[i] = (void *)
			(smp_group[0] + core_offsets[core] + SET);
		ipi_clear_regs[i] = (void *)
			(smp_group[0] + core_offsets[core] + CLEAR);
		ipi_status_regs[i] = (void *)
			(smp_group[0] + core_offsets[core] + STATUS);
		ipi_en_regs[i] = (void *)
			(smp_group[0] + core_offsets[core] + EN);
		ipi_mailbox_buf[i] = (void *)
			(smp_group[0] + core_offsets[core] + MBUF);
	}
}

/*
 * Simple enough, just poke the appropriate ipi register
 */
static void loongson3_send_ipi_single(int cpu, unsigned int action)
{
	ipi_write_action(cpu_logical_map(cpu), (u32)action);
}

static void loongson3_send_ipi_mask(const struct cpumask *mask, unsigned int action)
{
	unsigned int i;

	for_each_cpu(i, mask)
		ipi_write_action(cpu_logical_map(i), (u32)action);
}

void loongson3_ipi_interrupt(int irq)
{
	unsigned int action;
	unsigned int cpu = smp_processor_id();

	action = ipi_read_clear(cpu_logical_map(cpu));

	smp_mb();

	if (action & SMP_RESCHEDULE) {
		scheduler_ipi();
		per_cpu(irq_stat, cpu).ipi_irqs[IPI_RESCHEDULE]++;
	}

	if (action & SMP_CALL_FUNCTION) {
		irq_enter();
		generic_smp_call_function_interrupt();
		per_cpu(irq_stat, cpu).ipi_irqs[IPI_CALL_FUNCTION]++;
		irq_exit();
	}
}

static void loongson3_init_secondary(void)
{
	unsigned int cpu = smp_processor_id();
	unsigned int imask = ECFGF_PC | ECFGF_TIMER | ECFGF_IPI/* | ECFGF_IP1 | ECFGF_IP0 */; // 别的也没接

	/* Set interrupt mask, but don't enable */
	change_csr_ecfg(ECFG0_IM, imask);

	xconf_writel(0xffffffff, ipi_en_regs[cpu_logical_map(cpu)]);

#ifdef CONFIG_NUMA
	numa_add_cpu(cpu);
#endif
	per_cpu(cpu_state, cpu) = CPU_ONLINE;
	cpu_set_core(&cpu_data[cpu], cpu_logical_map(cpu));
	cpu_set_cluster(&cpu_data[cpu], 0);
	cpu_data[cpu].package =0;
}

static void loongson3_smp_finish(void)
{
	int cpu = smp_processor_id();

	local_irq_enable();

	xconf_writel(0, ipi_mailbox_buf[cpu_logical_map(cpu)]+0x0);

	pr_info("CPU#%d finished\n", smp_processor_id());
}

static void __init loongson3_smp_setup(void)
{
	ipi_regaddrs_init();

	xconf_writel(0xffffffff, ipi_en_regs[cpu_logical_map(0)]);

	pr_info("Detected %i available CPU(s)\n", loongson_sysconf.nr_cpus);

	cpu_set_core(&cpu_data[0], cpu_logical_map(0));
	cpu_set_cluster(&cpu_data[0], 0);
	cpu_data[0].package = 0;
}

static void __init loongson3_prepare_cpus(unsigned int max_cpus)
{
	int i = 0;

	for (i = 0; i < loongson_sysconf.nr_cpus; i++) {
		set_cpu_present(i, true);

		xconf_writel(0, ipi_mailbox_buf[__cpu_logical_map[i]]+0x0);
	}

	per_cpu(cpu_state, smp_processor_id()) = CPU_ONLINE;
}

/*
 * Setup the PC, SP, and TP of a secondary processor and start it running!
 */
static int loongson3_boot_secondary(int cpu, struct task_struct *idle)
{
	unsigned long startargs[4];

	pr_info("Booting CPU#%d...\n", cpu);

	/* startargs[] are initial PC, SP and TP for secondary CPU */
	startargs[0] = (unsigned long)&smpboot_entry;
	startargs[1] = (unsigned long)__KSTK_TOS(idle);
	startargs[2] = (unsigned long)task_thread_info(idle);
	startargs[3] = 0;

	pr_info("CPU#%d, func_pc=%lx, sp=%lx, tp=%lx\n",
			cpu, startargs[0], startargs[1], startargs[2]);
	xconf_writel(startargs[3], ipi_mailbox_buf[cpu_logical_map(cpu)]+0x18);
	xconf_writel(startargs[2], ipi_mailbox_buf[cpu_logical_map(cpu)]+0x10);
	xconf_writel(startargs[1], ipi_mailbox_buf[cpu_logical_map(cpu)]+0x8);
	xconf_writel(startargs[0], ipi_mailbox_buf[cpu_logical_map(cpu)]+0x0);
	xconf_writel(0xffffffff, ipi_en_regs[cpu_logical_map(cpu)]);
    xconf_writel(4, ipi_set_regs[cpu_logical_map(cpu)]);

	return 0;
}

const struct plat_smp_ops loongson3_smp_ops = {
	.send_ipi_single = loongson3_send_ipi_single,
	.send_ipi_mask = loongson3_send_ipi_mask,
	.smp_setup = loongson3_smp_setup,
	.prepare_cpus = loongson3_prepare_cpus,
	.boot_secondary = loongson3_boot_secondary,
	.init_secondary = loongson3_init_secondary,
	.smp_finish = loongson3_smp_finish,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable = loongson3_cpu_disable,
	.cpu_die = loongson3_cpu_die,
#endif
};

