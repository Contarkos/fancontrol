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
/*                       Variables globales                          */
/*********************************************************************/


/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

/* Init de toutes les fonctions nécessaires au Rpi */
int OS_init(void)
{
    int ret = 0;

    /* Init des GPIO */
    ret += os_init_gpio();

    /* Init des PWM */
    ret += os_init_pwm();

    /* Init de la CLOCK */
    ret += os_init_clock();

    /* Init COM */

    /* Init des timers */
    ret += os_init_timer();

    return ret;
}

/* Arret de toutes les fonctions du Rpi */
int OS_stop(void)
{
    int ret = 0;

    /* Stop des GPIO */
    ret += os_stop_gpio();

    /* Stop PWM */
    ret += os_stop_pwm();

    /* Stop CLOCK */
    ret += os_stop_clock();

    /* Stop timer */
    ret += os_end_timer();

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
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
        /* On va mapper le composant mémoire */
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
            perror("mmap");
            ret = -2;
        }
        else
        {
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

