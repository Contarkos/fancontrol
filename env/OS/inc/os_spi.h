#ifndef OS_SPI_H_
#define OS_SPI_H_

#include "os_rpi.h"

/* Defines */
#define OS_FILE_SPI0    "/dev/spidev0.0"
#define OS_FILE_SPI1    "/dev/spidev0.1"

/* Struct */
typedef struct
{
    char filename[OS_MAX_LENGTH_FILENAME];
    int fd;
    unsigned int speed;
    unsigned char mode;
    unsigned char bits_per_word;
    t_os_spi_device id;
} t_os_spi_struct;

/* Internal struct */
typedef struct
{
    struct bcm2835_peripheral device;
    t_os_spi_register *map;
    unsigned int speed;
    unsigned char mode;
    unsigned char bits_per_word;
    t_os_spi_device id;
    t_uint32 mosi_pin;
    t_os_gpio_func mosi_func;
    t_uint32 miso_pin;
    t_os_gpio_func miso_func;
    t_uint32 sclk_pin;
    t_os_gpio_func sclk_func;
    t_uint32 ce0_pin;
    t_os_gpio_func ce0_func;
    t_uint32 ce1_pin;
    t_os_gpio_func ce1_func;
    OS_mutex_t mutex;
    base_bool is_init;
} t_os_spi_struct_dev;

/* Local functions */
t_os_spi_struct* os_spi_get_device(t_os_spi_device i_device);

#endif /* OS_SPI_H_ */

