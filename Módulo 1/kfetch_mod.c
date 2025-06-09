/*
 * kfetch_mod.c - Módulo de kernel que exibe informações do sistema.
 */
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/utsname.h>
#include <linux/cpumask.h>
#include <asm/processor.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GRUPO 6");
MODULE_DESCRIPTION("KFETCH_MODULE - Exibe informações do sistema");

#define KFETCH_RELEASE (1 << 0)
#define KFETCH_NUM_CPUS (1 << 1)
#define KFETCH_CPU_MODEL (1 << 2)
#define KFETCH_MEM (1 << 3)
#define KFETCH_UPTIME (1 << 4)
#define KFETCH_NUM_PROCS (1 << 5)
#define KFETCH_FULL_INFO ((1 << 6) - 1)

// --- Protótipos e estruturas ---
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release};

static int major;
static struct class *kfetch_class = NULL;
static struct device *kfetch_device = NULL;

// ---- Buffer de saída e seu tamanho ----
static char kernel_buffer[1024];
static int kernel_buffer_len;

// ASCII Art
const char *ascii_art =
    "\x1b[33m"
    "\n"
    "                    ('(=--.\n"
    "      .             ;  o  _\\\\\n"
    "     (`(/,         / `/'`\n"
    "  `-\\\\` '.______/`  \\\\\n"
    "    \\\\`'-      -=`\\\\  )\n"
    "        `={  -= = _/  /\n"
    "        `\\\\  .-'   /`\n"
    "        {`-,__.'===,_\n"
    "        //`        `'\\\\\n"
    "    _.;'            '\"\n"
    "     `\\\\==.  ~Kweh! \n"
    "\x1b[0m";

// ---- Função de leitura ----
static ssize_t device_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off)
{
    int bytes_to_copy = 0;

    if (*off >= kernel_buffer_len)
    {
        return 0;
    }

    bytes_to_copy = kernel_buffer_len - *off;
    if (bytes_to_copy > len)
    {
        bytes_to_copy = len;
    }

    if (copy_to_user(user_buf, kernel_buffer + *off, bytes_to_copy) != 0)
    {
        return -EFAULT;
    }

    *off += bytes_to_copy;
    return bytes_to_copy;
}

static int __init my_init(void)
{
    struct sysinfo mem_info;
    unsigned long total_mem_mb;
    unsigned long free_mem_mb;
    struct task_struct *task;
    int process_count = 0;
    struct timespec64 uptime_sec;
    size_t vet = 0;
    size_t i, len;
    printk(KERN_INFO "kfetch_mod: Iniciando o módulo.\n");

    si_meminfo(&mem_info);
    ktime_get_boottime_ts64(&uptime_sec);
    total_mem_mb = (mem_info.totalram * PAGE_SIZE) / (1024 * 1024);
    free_mem_mb = (mem_info.freeram * PAGE_SIZE) / (1024 * 1024);

    for_each_process(task)
    {
        process_count++;
    }

    vet += scnprintf(kernel_buffer, sizeof(kernel_buffer),
                     "%s\n"
                     "Hostname: %s\n"
                     "Kernel: %s\n"
                     "CPU Model: %s\n"
                     "CPUs: %d\n"
                     "Memória: %lu / %lu MB\n"
                     "Processos: %d\n"
                     "Uptime: %ld minutos\n",
                     ascii_art,
                     init_utsname()->nodename,
                     init_utsname()->release,
                     cpu_data(0).x86_model_id,
                     num_online_cpus(),
                     free_mem_mb,
                     total_mem_mb,
                     process_count,
                     uptime_sec.tv_sec / 60); // em minutos sue burro

    kernel_buffer_len = strlen(kernel_buffer);

    major = register_chrdev(0, "kfetch_mod", &fops);
    if (major < 0)
    {
        printk(KERN_ALERT "kfetch_mod: Erro ao registrar dispositivo: %d\n", major);
        return major;
    }

    kfetch_class = class_create("kfetch_class");
    if (IS_ERR(kfetch_class))
    {
        unregister_chrdev(major, "kfetch_mod");
        return PTR_ERR(kfetch_class);
    }

    kfetch_device = device_create(kfetch_class, NULL, MKDEV(major, 0), NULL, "kfetch_mod");
    if (IS_ERR(kfetch_device))
    {
        class_destroy(kfetch_class);
        unregister_chrdev(major, "kfetch_mod");
        return PTR_ERR(kfetch_device);
    }

    printk(KERN_INFO "kfetch_mod: Dispositivo 'kfetch_mod' criado em /dev.\n");
    return 0;
}

static void __exit my_exit(void)
{
    device_destroy(kfetch_class, MKDEV(major, 0));
    class_destroy(kfetch_class);
    unregister_chrdev(major, "kfetch_mod");
    printk(KERN_INFO "kfetch_mod: Módulo removido.\n");
}

static int device_open(struct inode *inode, struct file *file) { return 0; }
static int device_release(struct inode *inode, struct file *file) { return 0; }
static ssize_t device_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) { return len; }

module_init(my_init);
module_exit(my_exit);