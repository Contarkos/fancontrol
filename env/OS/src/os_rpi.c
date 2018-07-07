#include "os.h"
#include "os_rpi.h"


struct bcm2835_peripheral gpio = {GPIO_BASE, 0, NULL, NULL};

// Exposes the physical address defined in the passed structure using mmap on /dev/mem
int map_peripheral(struct bcm2835_peripheral *p)
{
#if 0
    // Open /dev/mem
    if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
    {
        printf("Failed to open /dev/mem, try checking permissions.\n");
        return -1;
    }

    p->map = mmap(
            NULL,
            BLOCK_SIZE,
            PROT_READ|PROT_WRITE,
            MAP_SHARED,
            p->mem_fd,      // File descriptor to physical memory virtual file '/dev/mem'
            p->addr_p       // Address in physical map that we want this memory block to expose
            );

    if (p->map == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    p->addr = (volatile unsigned int *)p->map;
#else
    UNUSED_PARAMS(p);
#endif
    return 0;
}

void unmap_peripheral(struct bcm2835_peripheral *p)
{
#if 0
    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
#else
    UNUSED_PARAMS(p);
#endif
}
