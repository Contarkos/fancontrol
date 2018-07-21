#pragma once

#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

// Adresse des zones de la mémoire pour BCM2835. A verifier pour 2708
#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000) // GPIO controller

#define BLOCK_SIZE              (4*1024)

// Macros d'acces aux registres des GPIO
#define INP_GPIO(g)             *( gpio.addr + ((g)/10) ) &= (unsigned int) ~( 7 << ( ((g)%10)*3 ) )
#define OUT_GPIO(g)        {\
                                INP_GPIO(g);\
                                *( gpio.addr + ((g)/10) ) |= (unsigned int)  ( 1 << ( ((g)%10)*3 ) );\
                           } // Toujours utiliser INP avant OUT

#define SET_GPIO_ALT(g,a)  {\
                                 INP_GPIO(g);\
                                 *(gpio.addr + ((g)/10) ) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3));\
                           }

// Macros de base pour modifier les valeurs
#define GPIO_SET_REGISTER        *( gpio.addr + 7  )
#define GPIO_SET(g)              GPIO_SET_REGISTER = (unsigned int) 1 << (g) // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR_REGISTER        *( gpio.addr + 10 )
#define GPIO_CLR(g)              GPIO_CLR_REGISTER = (unsigned int) 1 << (g) // clears bits which are 1 ignores bits which are 0
 
#define GPIO_READ_REGISTER       *( gpio.addr + 13 )
#define GPIO_READ(g)            ( (GPIO_READ_REGISTER & (1 << (g))) >> (g) )

#define GPIO_MAX_NB             25

// IO Access
struct bcm2835_peripheral
{
    __off_t addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

extern struct bcm2835_peripheral gpio;
