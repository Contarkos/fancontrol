/* Global includes */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/


/*********************************************************************/
/*                         API Functions                             */
/*********************************************************************/

/* Init for all RPi peripherals */
int OS_init(void)
{
    int ret = 0;

    /* Init des GPIO */
    ret += os_init_gpio();

    /* Init des PWM */
    ret += os_init_pwm();

    /* Init de la CLOCK */
    ret += os_init_clock();

    /* Init for I2C peripheral */
    ret += os_init_i2c();

    /* Init COM */

    /* Init for timers */
    ret += os_init_timer();

    return ret;
}

/* Properly close all peripherals for the RPi */
int OS_stop(void)
{
    int ret = 0;

    /* Stop for GPIO */
    ret += os_stop_gpio();

    /* Stop PWM */
    ret += os_stop_pwm();

    /* Stop CLOCK */
    ret += os_stop_clock();

    /* Stop I2C */
    ret += os_stop_i2c();

    /* Stop timer */
    ret += os_end_timer();

    return ret;
}

/*********************************************************************/
/*                        Local functions                            */
/*********************************************************************/

/* Exposes the physical address defined in the passed structure using mmap on /dev/mem */
int os_map_peripheral(struct bcm2835_peripheral *p)
{
    int ret = 0;
#if 1
    /* Open /dev/mem */
    if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
    {
        LOG_ERR("OS : Failed to open /dev/mem, try checking permissions.");
        ret = -1;
    }
    else
    {
        /* Let's map the region we need in memory */
        p->map = mmap(
                NULL,
                BLOCK_SIZE,
                PROT_READ|PROT_WRITE,
                MAP_SHARED,
                p->mem_fd,      /* File descriptor to physical memory virtual file '/dev/mem' */
                p->addr_p       /* Address in physical map that we want this memory block to expose */
                );

        if (p->map == MAP_FAILED)
        {
            LOG_ERR("OS : mmap failed, errno = %d", errno);
            ret = -2;
        }
        else
        {
            LOG_INF2("OS : peripheral @%#lx successfully mapped @%p", p->addr_p, p->map);
            p->addr = (volatile unsigned int *)p->map;
        }

    }
#else
    UNUSED_PARAMS(p);
#endif
    return ret;
}

/* Close the memory mapping */
void os_unmap_peripheral(struct bcm2835_peripheral *p)
{
#if 1
    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
#else
    UNUSED_PARAMS(p);
#endif
}

