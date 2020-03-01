/* Global includes */
#include <fcntl.h>                /*Needed for SPI port */
#include <stdio.h>
#include <errno.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_i2c.h"

t_os_i2c_struct i2c_devices_array[OS_SPI_DEVICE_NB] =
{
    {
        .filename = OS_FILE_I2C0,
        .fd = -1,
        .addresses = { 0 },
        .nb_addresses = 0,
    },
    {
        .filename = OS_FILE_I2C1,
        .fd = -1,
        .addresses = { 0 },
        .nb_addresses = 0,
    }
};

int OS_i2c_open_device(t_os_i2c_device i_i2c_id, int i_address)
{
    int ret = 0;
    t_os_i2c_struct *device;

    device = os_i2c_get_device(i_i2c_id);

    if (NULL == device)
    {
        LOG_ERR("OS : wrong I2C device, id = %d", i_i2c_id);
        ret = -1;
    }

    /* Open the file */
    if (0 == ret)
    {
        /* Check if file is already initialised */
        if (device->fd < 0)
            device->fd = open(device->filename, O_RDWR);

        if (device->fd < 0)
        {
            LOG_ERR("OS : error while opening I2C file, errno = %d", errno);
            ret = -1;
        }
    }

    /* Add the address */
    if (0 == ret)
    {
        if (device->nb_addresses < OS_MAX_I2C_ADDRESSES)
        {
            device->addresses[device->nb_addresses] = i_address;
            device->nb_addresses++;
        }
        else
        {
            LOG_ERR("OS : number max of addresses reached for that device");
            ret = -1;
        }
    }

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

t_os_i2c_struct* os_i2c_get_device(t_os_i2c_device i_device)
{
    if (i_device < OS_I2C_DEVICE_NB)
        return &i2c_devices_array[i_device];
    else
        return NULL;
}
