
// Global includes
#include "wiringPi.h"
#include <unistd.h>
#include <fcntl.h>

// Local includes
#include "os.h"
#include "os_rpi.h"

struct bcm2835_peripheral gpio = {GPIO_BASE, 0, NULL, NULL};

// Exposes the physical address defined in the passed structure using mmap on /dev/mem
int map_peripheral(struct bcm2835_peripheral *p)
{
    int ret = 0;
#if 1
    // Open /dev/mem
    if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
    {
        printf("Failed to open /dev/mem, try checking permissions.\n");
        ret = -1;
    }
    else
    {
        // On va mapper le composant mémoire
        p->map = mmap(
                NULL,
                BLOCK_SIZE,
                PROT_READ|PROT_WRITE,
                MAP_SHARED,
                p->mem_fd,      // File descriptor to physical memory virtual file '/dev/mem'
                p->addr_p       // Address in physical map that we want this memory block to expose
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

void unmap_peripheral(struct bcm2835_peripheral *p)
{
#if 1
    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
#else
    UNUSED_PARAMS(p);
#endif
}

int OS_init_gpio(void)
{
    int ret = 0;

    // Mapping du fichier mémoire
    ret += map_peripheral(&gpio);

    if (0 != ret)
    {
        printf("OS : Erreur à l'init des GPIO, code : %d\n", ret);
    }
    else
    {
        printf("OS : Init GPIO ok\n");
    }

    return ret;
}

int OS_stop_gpio(void)
{
    int ret = 0;

    // Demapping du gpio
    unmap_peripheral(&gpio);

    return ret;
}

// Choix de la direction pour une GPIO
int OS_set_gpio(int i_pin, int i_inout)
{
    int ret = 0;

    if (i_pin <= GPIO_MAX_NB || i_pin >= 0)
    {
        if (i_inout)
        {
            OUT_GPIO(i_pin);
        }
        else
        {
            INP_GPIO(i_pin);
        }
    }
    else
    {
        printf("OS : Erreur numéro de pin GPIO : %d\n", i_pin);
        ret = -1;
    }

    return ret;
}

// Ecriture dans une GPIO avec wiringPi
int OS_write_gpio(int i_pin, int bool_active)
{
    int ret = 0;

    if (i_pin <= GPIO_MAX_NB || i_pin >= 0)
    {
        if (bool_active)
        {
            GPIO_SET(i_pin);
        }
        else
        {
            GPIO_CLR(i_pin);
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}
