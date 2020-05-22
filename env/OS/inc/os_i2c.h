#ifndef OS_I2C_H_
#define OS_I2C_H_

/* Defines */
#define OS_FILE_I2C0            "/dev/i2c-0"
#define OS_FILE_I2C1            "/dev/i2c-1"

#define OS_MAX_I2C_ADDRESSES    128

/* Struct */
typedef struct
{
    char filename[OS_MAX_LENGTH_FILENAME];
    int fd;
    int addresses[OS_MAX_I2C_ADDRESSES];
    int nb_addresses;
} t_os_i2c_struct;

/* Local functions */
t_os_i2c_struct* os_i2c_get_device(t_os_i2c_device i_device);

#endif /* OS_I2C_H_ */

