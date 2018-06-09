#include <stdio.h>

#include <asm-generic/mman.h>
#include <asm-generic/types.h>
#include <asm-generic/stat.h>

#include <unistd.h>

#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x2000000) // GPIO controller

#define BLOCK_SIZE              (4*1024)

// IO Access
struct bcm2835_peripheral
{
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

extern struct bcm2835_peripheral gpio;
