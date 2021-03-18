/* Global includes */
#include <fcntl.h>                /*Needed for SPI port */
#include <stdio.h>
#include <errno.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_rpi.h"

#include "os_i2c.h"

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/

/* I2C mapping structure initialisation */
struct bcm2835_peripheral os_periph_i2c0 = {I2C0_BASE, 0, NULL, NULL};
struct bcm2835_peripheral os_periph_i2c1 = {I2C1_BASE, 0, NULL, NULL};

/* Environment variables initialisation */
t_os_ret_okko is_init_i2c = OS_RET_KO;

/* Mutex for I2C registers */
OS_mutex_t os_i2c0_mutex = OS_INIT_MUTEX;
OS_mutex_t os_i2c1_mutex = OS_INIT_MUTEX;

/*********************************************************************/
/*                        Static variables                           */
/*********************************************************************/

static t_os_i2c_struct i2c_devices_array[OS_SPI_DEVICE_NB] =
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

static t_os_i2c_struct_dev i2c_devices_direct[] =
{
    /* I2C0 device */
    {
        .device = {I2C0_BASE, 0, NULL, NULL},
        .map = NULL,
        .clk_speed = OS_I2C_CLOCK_100KHZ,
        .sda_pin = OS_GPIO_I2C0_SDA,
        .sda_func = OS_GPIO_FUNC_ALT0,
        .scl_pin = OS_GPIO_I2C0_SCL,
        .scl_func = OS_GPIO_FUNC_ALT0,
        .addresses = { 0 },
        .nb_addresses = 0,
        .mutex = OS_INIT_MUTEX,
    },
    /* I2C1 device */
    {
        .device = {I2C1_BASE, 0, NULL, NULL},
        .map = NULL,
        .clk_speed = OS_I2C_CLOCK_100KHZ,
        .sda_pin = OS_GPIO_I2C1_SDA,
        .sda_func = OS_GPIO_FUNC_ALT0,
        .scl_pin = OS_GPIO_I2C1_SCL,
        .scl_func = OS_GPIO_FUNC_ALT0,
        .addresses = { 0 },
        .nb_addresses = 0,
        .mutex = OS_INIT_MUTEX,
    }
};

static const t_uint32 i2c_devices_nb = sizeof(i2c_devices_direct) / sizeof(i2c_devices_direct[0]);

/*********************************************************************/
/*                       Static declarations                         */
/*********************************************************************/

static t_os_i2c_struct_dev* _os_i2c_get_dev(t_os_i2c_device i_device);
static int _os_i2c_wait_done(t_os_i2c_struct_dev *i_dev);

static inline void      _os_i2c_start_write (t_os_i2c_struct_dev *i_dev);
static inline void      _os_i2c_start_read  (t_os_i2c_struct_dev *i_dev);
static inline void      _os_i2c_clear_status(t_os_i2c_struct_dev *i_dev);
static inline void      _os_i2c_set_addr    (t_os_i2c_struct_dev *i_dev, t_uint32 i_address);
static inline void      _os_i2c_set_dlen    (t_os_i2c_struct_dev *i_dev, t_uint32 i_dlen);
static inline void      _os_i2c_write_fifo  (t_os_i2c_struct_dev *i_dev, t_uint8 i_data);
static inline t_uint8   _os_i2c_read_fifo   (t_os_i2c_struct_dev *i_dev);
static inline void      _os_i2c_set_cdiv    (t_os_i2c_struct_dev *i_dev, t_uint16 i_cdiv);

static void _os_print_registers(t_os_i2c_struct_dev *i_dev, const char *i_msg);

/*********************************************************************/
/*                         API Functions                             */
/*********************************************************************/

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

int OS_i2c_close_device(t_os_i2c_device i_i2c_id)
{
    int ret = 0;
    t_os_i2c_struct *device;

    device = os_i2c_get_device(i_i2c_id);

    if (NULL == device)
    {
        LOG_ERR("OS : wrong I2C device, id = %d", i_i2c_id);
        ret = -1;
    }

    /* Closing open file */
    if (0 == ret)
    {
        ret = close(device->fd);

        if (ret < 0)
            LOG_ERR("OS : error while closing I2C device, errno = %d", errno);
    }

    if (0 == ret)
    {
        int ii;

        for (ii = 0; ii < OS_MAX_I2C_ADDRESSES; ii++)
            device->addresses[ii] = 0;

        device->nb_addresses = 0;
    }

    return ret;
}

int OS_i2c_init_device (t_os_i2c_device i_i2c_id)
{
    int ret = 0;
    t_os_i2c_struct_dev *dev = NULL;

    if (OS_RET_KO == is_init_i2c)
    {
        LOG_ERR("OS : I2C device not ready");
        ret = -1;
    }

    if (0 == ret)
    {
        dev = _os_i2c_get_dev(i_i2c_id);

        if (NULL == dev)
            ret = -1;
    }

    if (0 == ret)
    {
        if (dev->is_init == BASE_FALSE)
        {
            ret = OS_set_gpio(dev->sda_pin, dev->sda_func);

            if (0 != ret)
                LOG_ERR("OS : could not set SDA pin correctly for I2C%d", i_i2c_id);
        }
    }

    if (0 == ret)
    {
        if (dev->is_init == BASE_FALSE)
        {
            ret = OS_set_gpio(dev->scl_pin, dev->scl_func);

            if (0 != ret)
                LOG_ERR("OS : could not set SCL pin correctly for I2C%d", i_i2c_id);
        }
    }

    if (0 == ret)
        dev->is_init = BASE_TRUE;

    return ret;
}

int OS_i2c_set_clock (t_os_i2c_device i_id, t_os_i2c_clock i_clock)
{
    int ret = 0;
    t_os_i2c_struct_dev *dev = NULL;
    t_uint16 cdiv;

    if (OS_RET_KO == is_init_i2c)
    {
        LOG_ERR("OS : I2C device not ready");
        ret = -1;
    }

    if (0 == ret)
    {
        dev = _os_i2c_get_dev(i_id);

        if (NULL == dev)
            ret = -1;
    }

    if (0 == ret)
    {
        switch (i_clock)
        {
            case OS_I2C_CLOCK_100KHZ:
                cdiv = OS_I2C_CDIV_100KHZ;
                break;
            case OS_I2C_CLOCK_400KHZ:
                cdiv = OS_I2C_CDIV_400KHZ;
                break;
            default:
                LOG_ERR("OS : wrong value for I2C clock");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        dev->clk_speed = i_clock;
        _os_i2c_set_cdiv(dev, cdiv);
    }

    return ret;
}

int OS_i2c_write_data (t_os_i2c_device i_id, t_uint32 i_address, t_uint8 *i_data, t_uint32 i_length)
{
    int ret = 0;
    t_uint32 ii = 0;
    t_os_i2c_struct_dev *dev = NULL;

    if (OS_RET_KO == is_init_i2c)
    {
        LOG_ERR("OS : I2C device not ready");
        ret = -1;
    }

    if (0 == ret)
    {
        dev = _os_i2c_get_dev(i_id);

        if (NULL == dev)
            ret = -1;
    }

    /* Length cannot be more than 16 bytes */

    /* Take mutex */
    if (0 == ret)
        ret = OS_mutex_lock(&dev->mutex);

    if (0 == ret)
    {
        /* Write address we want to send data to */
        _os_i2c_set_addr(dev, i_address);

        /* Write number of bytes we want to send */
        _os_i2c_set_dlen(dev, i_length);

        /* Fill the FIFO with the data */
        for (ii = 0; ii < i_length; ii++)
            _os_i2c_write_fifo(dev, i_data[ii]);

        /* Clear status registr */
        _os_i2c_clear_status(dev);

        /* Start transfer */
        _os_i2c_start_write(dev);

        /* Wait for transfer to be done */
        ret = _os_i2c_wait_done(dev);

        ret = OS_mutex_unlock(&dev->mutex);
    }

    return ret;
}

int OS_i2c_read_data (t_os_i2c_device i_id, t_uint32 i_address, t_uint8 *i_data, t_uint32 i_length)
{
    int ret = 0;
    t_uint32 ii = 0;
    t_os_i2c_struct_dev *dev = NULL;

    if (OS_RET_KO == is_init_i2c)
    {
        LOG_ERR("OS : I2C device not ready");
        ret = -1;
    }

    if (0 == ret)
    {
        dev = _os_i2c_get_dev(i_id);

        if (NULL == dev)
            ret = -1;
    }

    /* Length cannot be more than 16 bytes */

    /* Take mutex */
    if (0 == ret)
        ret = OS_mutex_lock(&dev->mutex);

    if (0 == ret)
    {
        /* Write address we want to send data to */
        _os_i2c_set_addr(dev, i_address);

        /* Write number of bytes we want to send */
        _os_i2c_set_dlen(dev, i_length);

        /* Clear status registr */
        _os_i2c_clear_status(dev);

        /* Start to read */
        _os_i2c_start_read(dev);

        /* Wait for transfer to be done */
        ret = _os_i2c_wait_done(dev);

        /* Read the data in the FIFO */
        for (ii = 0; ii < i_length; ii++)
            i_data[ii] = _os_i2c_read_fifo(dev);

        ret = OS_mutex_unlock(&dev->mutex);
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

static t_os_i2c_struct_dev* _os_i2c_get_dev(t_os_i2c_device i_device)
{
    if (i_device < i2c_devices_nb)
        return &i2c_devices_direct[i_device];
    else
        return NULL;
}

static inline void _os_i2c_start_write(t_os_i2c_struct_dev *i_dev)
{
    i_dev->map->ctrl = (I2C_CTL_ENABLE_MASK | I2C_CTL_START_MASK);
}

static inline void _os_i2c_start_read(t_os_i2c_struct_dev *i_dev)
{
    i_dev->map->ctrl = (I2C_CTL_ENABLE_MASK | I2C_CTL_START_MASK | I2C_CTL_CLR_FIFO_MASK | I2C_CTL_RDWR_MASK);
}

static inline void _os_i2c_clear_status(t_os_i2c_struct_dev *i_dev)
{
    i_dev->map->status = (I2C_STA_DONE_MASK | I2C_STA_ERR_MASK | I2C_STA_CLKT_MASK);
}

static inline void _os_i2c_set_addr(t_os_i2c_struct_dev *i_dev, t_uint32 i_address)
{
    i_dev->map->addr = (i_address & I2C_ADDR_VALUE_MASK);
}

static inline void _os_i2c_set_dlen(t_os_i2c_struct_dev *i_dev, t_uint32 i_dlen)
{
    i_dev->map->dlen = (i_dlen & I2C_DLEN_VALUE_MASK);
}

static inline void _os_i2c_write_fifo(t_os_i2c_struct_dev *i_dev, t_uint8 i_data)
{
    i_dev->map->fifo = (i_data & I2C_FIFO_VALUE_MASK);
}

static inline t_uint8 _os_i2c_read_fifo(t_os_i2c_struct_dev *i_dev)
{
    return (i_dev->map->fifo & I2C_FIFO_VALUE_MASK);
}

static inline void _os_i2c_set_cdiv (t_os_i2c_struct_dev *i_dev, t_uint16 i_cdiv)
{
    i_dev->map->cdiv = (i_cdiv & I2C_DIV_VALUE_MASK);
}

static int _os_i2c_wait_done(t_os_i2c_struct_dev *i_dev)
{
    int ret = 0;
    int timeout = 100; /* FIXME find a real timeout value */

    t_uint32 sta = i_dev->map->status;
    while ( ((sta & I2C_STA_DONE_MASK) == 0) && (timeout > 0))
    {
        OS_usleep(100);
        sta = i_dev->map->status;
        timeout--;
    }

    LOG_INF3("OS : I2C transfer done, cpt = %d, status = %#x", (100 - timeout), i_dev->map->status);
    return ret;
}

static void _os_print_registers(t_os_i2c_struct_dev *i_dev, const char *i_msg)
{
    t_uint32 conf[8] = { 0 };

    for (int ii = 0; ii < 8; ii++)
        conf[ii] = *(i_dev->device.addr + ii);

    LOG_ERR("OS : %s, REG = %#02x %#02x %#02x %#02x %#02x %#02x %#02x %#02x",
            i_msg, conf[0], conf[1], conf[2], conf[3], conf[4], conf[5], conf[6], conf[7]);
}

int os_init_i2c (void)
{
    int ret = 0;
    t_uint32 ii = 0;

    if (OS_RET_OK == is_init_i2c)
    {
        LOG_INF1("OS : I2C init already done");
        goto endofinit;
    }

    for (ii = 0; (ii < i2c_devices_nb) && (0 == ret); ii++)
    {
        /* Mapping of memory region */
        ret = os_map_peripheral(&i2c_devices_direct[ii].device);

        if (0 != ret)
            LOG_ERR("OS : Error during I2C%d init, code : %d", ii, ret);
        else
        {
            LOG_INF1("OS : Init I2C%d ok", ii);
            i2c_devices_direct[ii].map = (t_os_i2c_register *) i2c_devices_direct[ii].device.map;
        }
    }

    if (0 == ret)
       is_init_i2c = OS_RET_OK;

endofinit:
    return ret;
}

int os_stop_i2c(void)
{
    int ret = 0;
    t_uint32 ii = 0;
    t_os_i2c_struct_dev *dev = NULL;

    if (OS_RET_KO == is_init_i2c)
    {
        LOG_WNG("OS : I2C not initialized");
        ret = 1;
    }

    if (0 == ret)
    {
        for (ii = 0; ii < i2c_devices_nb; ii++)
        {
            dev = &i2c_devices_direct[ii];

            /* Unmap data */
            dev->map = NULL;
            dev->is_init = BASE_FALSE;

            /* Proper unmapping for I2C0 */
            os_unmap_peripheral(&dev->device);
        }

        /* Change global status to KO */
        is_init_i2c = OS_RET_KO;
    }

    return ret;
}

