#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/fs.h>

#define PROC_NAME "riskmonitor"

/**
 * Obtém estatísticas de I/O com verificações adicionais.
 */
static u64 get_io_usage(struct task_struct *task) {
    if (!task)
        return 0;

    u64 io_bytes = task->ioac.read_bytes + task->ioac.write_bytes;

    // Se ainda for 0, tenta capturar estatísticas alternativas
    if (io_bytes == 0) {
        io_bytes = task->acct_rss_mem1 + task->acct_vm_mem1;
    }

    return io_bytes;
}

static int riskmonitor_show(struct seq_file *m, void *v) {
    struct task_struct *task;

    for_each_process(task) {
        u64 cpu_time = task->se.sum_exec_runtime;
        u64 io_bytes = get_io_usage(task);

        // Nova lógica de avaliação de risco
        const char *risk;
        if (cpu_time > 80000000 || io_bytes > 15000000)
            risk = "Alto";
        else if (cpu_time > 40000000 || io_bytes > 7500000)
            risk = "Médio";
        else
            risk = "Baixo";

        seq_printf(m, "PID: %d | CPU: %llu | IO: %llu | Risco: %s\n",
                   task->pid, cpu_time, io_bytes, risk);
    }

    return 0;
}

static int riskmonitor_open(struct inode *inode, struct file *file) {
    return single_open(file, riskmonitor_show, NULL);
}

static const struct proc_ops riskmonitor_fops = {
    .proc_open    = riskmonitor_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init riskmonitor_init(void) {
    proc_create(PROC_NAME, 0, NULL, &riskmonitor_fops);
    printk(KERN_INFO "[riskmonitor] Módulo carregado com sucesso.\n");
    return 0;
}

static void __exit riskmonitor_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "[riskmonitor] Módulo descarregado.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pedro");
MODULE_DESCRIPTION("Monitor de risco de processos via uso de CPU e IO atualizado para kernels modernos");
MODULE_VERSION("2.7");

module_init(riskmonitor_init);
module_exit(riskmonitor_exit);
