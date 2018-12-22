/*
 * Basic Linux Kernel module using GPIO interrupts.
 *
 * Author:
 *     Interrupt handling part - Stefan Wendler (devnull@kaltpost.de)
 *  Device part :
 *     Copyright (C) 2013, Jack Whitham
 *     Copyright (C) 2009-2010, University of York
 *     Copyright (C) 2004-2006, Advanced Micro Devices, Inc.
 *
 *  Modified by Contarkos for GPIO interrupts counting
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/poll.h>

#define GPIO_FOR_RX_SIGNAL    23
#define DEV_NAME              "gpio_ui0"
#define BUFFER_SZ             512

/* Last Interrupt timestamp */
static struct timespec  lastIrq_time;
static unsigned long    lastDelta[BUFFER_SZ];

static int  pRead;
static int  pWrite;
static int  wasOverflow;

// Semaphore de blocage
static DECLARE_WAIT_QUEUE_HEAD(_queue_read);

/* Define GPIOs for RX signal */
static struct gpio signals[] = {
    { GPIO_FOR_RX_SIGNAL, GPIOF_IN, "RX Signal" },    // Rx signal
};

/* Later on, the assigned IRQ numbers for the buttons are stored here */
static int rx_irqs[] = { -1 };

/*
 * The interrupt service routine called on every pin status change
 */
static irqreturn_t rx_isr(int irq, void *data)
{
    struct timespec current_time;
    struct timespec delta;
    unsigned long ns;

    // Calcul du temps depuis la dernière interruption
    getnstimeofday(&current_time);

    delta = timespec_sub(current_time, lastIrq_time);
    ns = ( (long long)delta.tv_sec * 1000000 ) + ( delta.tv_nsec/1000 );

    getnstimeofday(&lastIrq_time);

    // Ajout de la donnée dans le tableau
    lastDelta[pWrite] = ns;

    // Hack pour éviter le modulo parce que BUFFER_SZ est une puissance de 2
    pWrite = ( pWrite + 1 ) & (BUFFER_SZ-1);
    if (pWrite == pRead)
    {
        // overflow
        pRead = ( pRead + 1 ) & (BUFFER_SZ-1);
        if ( wasOverflow == 0 )
        {
            printk(KERN_ERR "KISR TIME - Buffer Overflow - IRQ will be missed");
            wasOverflow = 1;
        }
    }
    else
    {
        wasOverflow = 0;
    }

    // Reveil des queues
    wake_up(&(_queue_read));

    return IRQ_HANDLED;
}


static int kisr_open(struct inode *inode, struct file *file)
{
   return nonseekable_open(inode, file);
}

static int kisr_release(struct inode *inode, struct file *file)
{
   return 0;
}

static ssize_t kisr_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *pos)
{
   return -EINVAL;
}

/*
 * returns one of the line with the time between two IRQs
 * return 0 : end of reading
 * return >0 : size
 * return -EFAULT : error
 */
static ssize_t kisr_read(struct file *file, char __user *buf,
                          size_t count, loff_t *pos)
{
    int _ret = 0, _count = 0;
    int _error_count;

    // Buffer overflow
    if ( pRead == pWrite )
    {
        _ret = wait_event_interruptible(_queue_read, (pRead != pWrite));

        if (_ret != 0)
        {
            return -ERESTARTSYS;
        }
    }

    _count = sizeof(unsigned long);
    _error_count = copy_to_user(buf, &(lastDelta[pRead]), _count);

    if ( _error_count != 0 )
    {
        printk(KERN_ERR "KISR Time - Error writing to char device");
        return -EFAULT;
    }
    pRead = (pRead + 1) & (BUFFER_SZ-1);

    return _count;
}

static unsigned int kisr_poll(struct file *file, poll_table *table)
{
    unsigned int mask = 0;

    // Attente du signal de nouvelle donnée
    poll_wait(file, &(_queue_read), table);

    if ( pRead != pWrite )
    {
        // Data normale dispo
        mask |= (POLLIN | POLLRDNORM);
    }

    printk(KERN_INFO "KISR poll - mask = %d\n", mask);
    return mask;
}

static struct file_operations kisr_fops = {
   .owner = THIS_MODULE,
   .open = kisr_open,
   .read = kisr_read,
   .write = kisr_write,
   .release = kisr_release,
   .poll = kisr_poll,
};

static struct miscdevice kisr_misc_device = {
   .minor = MISC_DYNAMIC_MINOR,
   .name = DEV_NAME,
   .fops = &kisr_fops,
};

/*
 * Module init function
 */
static int __init kisr_init(void)
{
    int ret = 0;
    printk(KERN_INFO "%s\n", __func__);

    // INITIALIZE IRQ TIME AND Queue Management
    getnstimeofday(&lastIrq_time);
    pRead = 0;
    pWrite = 0;
    wasOverflow = 0;

    // Init de la wait queue
    init_waitqueue_head(&(_queue_read));

    // register GPIO PIN in use
    ret = gpio_request_array(signals, ARRAY_SIZE(signals));

    if (ret)
    {
        printk(KERN_ERR "KISR GPIO - Unable to request GPIOs for RX Signals: %d\n", ret);
        goto fail2;
    }

    // Register IRQ for this GPIO
    ret = gpio_to_irq(signals[0].gpio);
    if(ret < 0)
    {
        printk(KERN_ERR "KISR GPIO - Unable to request IRQ: %d\n", ret);
        goto fail2;
    }
    rx_irqs[0] = ret;
    printk(KERN_INFO "KISR GPIO - Successfully requested RX IRQ # %d\n", rx_irqs[0]);
    ret = request_irq(rx_irqs[0], rx_isr, IRQF_TRIGGER_RISING , "rfrpi#rx", NULL);

    if(ret)
    {
        printk(KERN_ERR "KISR GPIO - Unable to request IRQ: %d\n", ret);
        goto fail3;
    }

    // Register a character device for communication with user space
    misc_register(&kisr_misc_device);

    return 0;

    // cleanup what has been setup so far
fail3:
    free_irq(rx_irqs[0], NULL);

fail2:
    gpio_free_array(signals, ARRAY_SIZE(signals));
    return ret;
}

/**
 * Module exit function
 */
static void __exit kisr_exit(void)
{
   printk(KERN_INFO "%s\n", __func__);

   // Deregister device
   misc_deregister(&kisr_misc_device);

   // free irqs
   free_irq(rx_irqs[0], NULL);

   // unregister
   gpio_free_array(signals, ARRAY_SIZE(signals));
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Contarkos");
MODULE_DESCRIPTION("Linux Kernel Module for gpio interrupts counting");

module_init(kisr_init);
module_exit(kisr_exit);
