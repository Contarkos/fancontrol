#pragma once

#include <pthread.h>

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*loop_func)(void *);

typedef struct
{
    pthread_t thread;
    pthread_attr_t attr;
    loop_func loop;
} OS_thread_t;

// Fonction pour les GPIO
typedef enum e_os_gpio_func
{
    OS_GPIO_FUNC_IN = 0,
    OS_GPIO_FUNC_OUT = 1,
    OS_GPIO_FUNC_ALT1 = 2,
    OS_GPIO_FUNC_ALT2 = 3,
    OS_GPIO_FUNC_ALT3 = 4
} t_os_gpio_func;

// Init des modules disponibles
int OS_init(void);
int OS_stop(void);

int OS_create_timer(void);

// Gestion des threads
int OS_create_thread(OS_thread_t * p_o_thread,
                     void * args);

int OS_joint_thread(OS_thread_t * p_i_thread, void **retval);

int OS_detach_thread(OS_thread_t * p_i_thread);

// Pour GPIO
int OS_init_gpio(void);
int OS_stop_gpio(void);

int OS_set_gpio(int i_pin, int i_inout);
int OS_write_gpio(int i_pin, int bool_active);

// Pour SPI
int OS_spiOpenPort (int spi_device);
int OS_spiClosePort (int spi_device);
int OS_spiWriteAndRead (int spi_device, unsigned char *data, int length);


#ifdef __cplusplus
}
#endif

