#ifndef OS_I2C_H_
#define OS_I2C_H_

#include "os_rpi.h"

/* Defines */
#define OS_FILE_I2C0            "/dev/i2c-0"
#define OS_FILE_I2C1            "/dev/i2c-1"

#define OS_MAX_I2C_ADDRESSES    128

#define OS_I2C_DEFAULT_CLOCK    100000U /* Default is 100kHz on RPi */

#define OS_I2C_CDIV_100KHZ      1500U   /* Divider value to get a 100kHz clock */
#define OS_I2C_CDIV_400KHZ      375U    /* Divider value to get a 400kHz clock */

/* Struct */
typedef struct
{
    char filename[OS_MAX_LENGTH_FILENAME];
    int fd;
    int addresses[OS_MAX_I2C_ADDRESSES];
    int nb_addresses;
} t_os_i2c_struct;

typedef struct
{
    struct bcm2835_peripheral device;
    t_os_i2c_register *map;
    t_os_i2c_clock clk_speed;
    t_uint32 sda_pin;
    t_os_gpio_func sda_func;
    t_uint32 scl_pin;
    t_os_gpio_func scl_func;
    int addresses[OS_MAX_I2C_ADDRESSES];
    int nb_addresses;
    OS_mutex_t mutex;
    base_bool is_init;
} t_os_i2c_struct_dev;

/* Local functions */

#endif /* OS_I2C_H_ */

