#include <types.h>
#include <config.h>
#include <string.h>
#include <arm.h>
#include <console.h>
#include <param.h>
#include <vm.h>
#include <mm.h>
#include <clock.h>
#include <timer.h>
#include <trap.h>
#include <proc.h>
#include <emmc.h>
#include <mbox.h>
#include <irq.h>
#include <ds3231.h>
#include <random.h>
#include <i2c.h>
#include <usb.h>
#include <net/net.h>
#include <vfs.h>
#include <driver.h>

extern struct driver console_driver;
extern struct driver tty_driver;
extern struct driver sd_driver;

struct driver *drivers[] = {
    &console_driver,
    &tty_driver,
    &sd_driver,
    NULL
};

extern struct mount_ops v6_mount_ops;
extern struct mount_ops procfs_mount_ops;

struct mount_ops *filesystems[] = {
    &v6_mount_ops,
    &procfs_mount_ops,
    NULL
};

device_t root_dev = DEVV6;

/*
 * Keep it in data segment by explicitly initializing by zero,
 * since we have `-fno-zero-initialized-in-bss` in Makefile.
 */
static struct {
    int cnt;
    struct spinlock lock;
} mp = { 0 };

void
main()
{
    extern char edata[], end[];
    acquire(&mp.lock);
    if (mp.cnt++ == 0) {
        memset(edata, 0, end - edata);
        i2c_init(DS3231_I2C_DIV);
        irq_init();
        console_preinit();
        mm_init();
        clock_init();
        rand_init();
        proc_init();
        for (int i = 0; drivers[i]; i++)
            drivers[i]->init();
        init_vfs();
        for (int i = 0; filesystems[i]; i++)
            filesystems[i]->init();
        int err = vfs_mount(NULL, "/", root_dev, &v6_mount_ops, 0, 0);
        if (err < 0)
            error("vfs_mount error: %d", err);
#if 0
        usb_init();
        net_init();
        net_run();
#endif
        user_init();
        //kthread_created(kthread_read_ether);

        // Tests
#if 0
        mbox_test();
        mm_test();
        vm_test();
#endif
    }
    release(&mp.lock);

    timer_init();
    trap_init();
    info("cpu %d init finished", cpuid());

    scheduler();

    panic("scheduler return.\n");
}
