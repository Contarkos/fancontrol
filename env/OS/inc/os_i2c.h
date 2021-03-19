#ifndef OS_I2C_H_
#define OS_I2C_H_

#include "os_rpi.h"

/* Defines */
#define OS_I2C_DEFAULT_CLOCK    100000U /* Default is 100kHz on RPi */

#define OS_I2C_CDIV_100KHZ      1500U   /* Divider value to get a 100kHz clock */
#define OS_I2C_CDIV_400KHZ      375U    /* Divider value to get a 400kHz clock */

#define OS_I2C_MAX_LENGTH       16U     /* Max length of the FIFO for the data */

/* Struct */
typedef struct
{
    struct bcm2835_peripheral device;
    t_os_i2c_register *map;
    t_os_i2c_clock clk_speed;
    t_uint32 sda_pin;
    t_os_gpio_func sda_func;
    t_uint32 scl_pin;
    t_os_gpio_func scl_func;
    OS_mutex_t mutex;
    base_bool is_init;
} t_os_i2c_struct_dev;

/* Local functions */

#endif /* OS_I2C_H_ */

