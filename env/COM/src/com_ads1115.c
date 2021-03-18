/* Global includes */

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"

#include "com_ads.h"

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/

/*********************************************************************/
/*                        Static variables                           */
/*********************************************************************/

static t_com_ads_config com_ads_config[OS_I2C_DEVICE_NB] =
{
    /* Device 0 */
    {
        .bits =
        {
            .os = COM_ADS_OS_NULL,
            .mux = COM_ADS_PAIR_AIN0_GND,
            .pga = COM_ADS_GAIN_4096,
            .mode = COM_ADS_MODE_CONTINUOUS,
            .rate = COM_ADS_RATE_128SPS,
            .comp_mode = COM_ADS_COMP_MODE_TRAD,
            .comp_pol = COM_ADS_COMP_POL_LOW,
            .comp_latch = COM_ADS_COMP_LATCH_OFF,
            .comp = COM_ADS_COMP_ASSERT_DISABLE,
        }
    },
    /* Device 1 */
    {
        .bits =
        {
            .os = COM_ADS_OS_NULL,
            .mux = COM_ADS_PAIR_AIN0_GND,
            .pga = COM_ADS_GAIN_4096,
            .mode = COM_ADS_MODE_CONTINUOUS,
            .rate = COM_ADS_RATE_128SPS,
            .comp_mode = COM_ADS_COMP_MODE_TRAD,
            .comp_pol = COM_ADS_COMP_POL_LOW,
            .comp_latch = COM_ADS_COMP_LATCH_OFF,
            .comp = COM_ADS_COMP_ASSERT_DISABLE,
        }
    },
};

/*********************************************************************/
/*                       Static declarations                         */
/*********************************************************************/

static int _com_ads_set_config(t_os_i2c_device i_dev);
static int _com_ads_print_config(t_os_i2c_device i_dev);
static t_int16 _com_ads_read_result(t_os_i2c_device i_dev);

/*********************************************************************/
/*                         API Functions                             */
/*********************************************************************/

int COM_ads_init(t_os_i2c_device i_device, t_os_i2c_clock i_speed)
{
    int ret = 0;

    if (0 == ret)
    {
        if (i_device >= OS_I2C_DEVICE_NB)
        {
            LOG_ERR("COM : wrong I2C device ID");
            ret = -1;
        }
    }

    if (0 == ret)
    {
        switch (i_speed)
        {
            case OS_I2C_CLOCK_100KHZ:
            case OS_I2C_CLOCK_400KHZ:
                break;
            default:
                LOG_ERR("COM : wrong value for I2C clock value");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = OS_i2c_init_device(i_device);

        if (0 != ret)
            LOG_ERR("COM : could not init I2C device %d", i_device);
    }

    if (0 == ret)
    {
        ret = OS_i2c_set_clock(i_device, i_speed);

        if (0 != ret)
            LOG_ERR("COM : could not configure clock for I2C device %d", i_device);
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_device);

        if (0 != ret)
            LOG_ERR("COM : could not configure the ADS properly");
        else
            LOG_INF1("COM : current config = %#04hx, default gain is %d", com_ads_config[i_device].setup, com_ads_config[i_device].bits.pga);
    }

    return ret;
}

int COM_ads_set_mode(t_os_i2c_device i_device, t_com_ads_mode i_mode)
{
    int ret = 0;

    if (i_device >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_mode)
        {
            case COM_ADS_MODE_CONTINUOUS:
            case COM_ADS_MODE_POWERDOWN:
                /* Update the current setup */
                com_ads_config[i_device].bits.mode = i_mode;
                break;
            default:
                LOG_ERR("COM : wrong mode for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_device);

        if (0 != ret)
            LOG_ERR("COM : could not set mode for ADS");
        else
            ret = _com_ads_print_config(i_device);
    }

    return ret;
}

int COM_ads_set_pair (t_os_i2c_device i_dev, t_com_ads_pair i_pair)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_pair)
        {
            case COM_ADS_PAIR_AIN0_AIN1:
            case COM_ADS_PAIR_AIN0_AIN3:
            case COM_ADS_PAIR_AIN1_AIN3:
            case COM_ADS_PAIR_AIN2_AIN3:
            case COM_ADS_PAIR_AIN0_GND:
            case COM_ADS_PAIR_AIN1_GND:
            case COM_ADS_PAIR_AIN2_GND:
            case COM_ADS_PAIR_AIN3_GND:
                /* Update the current setup and ask to sync */
                com_ads_config[i_dev].bits.mux = i_pair;
                break;
            default:
                LOG_ERR("COM : wrong value for ADS pair");
                ret = -1;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set pair for ADS");
    }

    if (0 == ret)
    {
        /* Wait a bit to be sure the next conversion is about the right */
        if (COM_ADS_MODE_CONTINUOUS == com_ads_config[i_dev].bits.mode)
            OS_usleep(100000);

        ret = _com_ads_print_config(i_dev);
    }

    return ret;
}

t_int16 COM_ads_read_result(t_os_i2c_device i_dev)
{
    t_int16 result = 0;
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
        result = _com_ads_read_result(i_dev);

    return result;
}

/*********************************************************************/
/*                          Local functions                          */
/*********************************************************************/

/* For internal use only, this function does not check its arguments */
static int _com_ads_set_config(t_os_i2c_device i_dev)
{
    int ret = 0;

    const t_uint32 size = COM_ADS_POINTER_SIZE + COM_ADS_CONFIG_SIZE;
    t_uint8 data[COM_ADS_POINTER_SIZE + COM_ADS_CONFIG_SIZE] = { 0 };

    /* Fill the array */
    data[0] = COM_ADS_CONFIG_REGISTER;
    data[1] = (t_uint8) ( (com_ads_config[i_dev].setup & 0xFF00) >> 8 );
    data[2] = (t_uint8) ( (com_ads_config[i_dev].setup & 0x00FF) >> 0 );

    /* Send the command to the ADS */
    ret = OS_i2c_write_data(i_dev, COM_ADS1115_ADDRESS, data, size);

    return ret;
}

static int _com_ads_print_config(t_os_i2c_device i_dev)
{
    int ret = 0;
    t_uint16 config = 0;
    t_uint8 wdata[COM_ADS_POINTER_SIZE];
    t_uint8 rdata[COM_ADS_CONFIG_SIZE] = { 0 };

    /* Point to conversion register */
    wdata[0] = COM_ADS_CONFIG_REGISTER;
    ret = OS_i2c_write_data(i_dev, COM_ADS1115_ADDRESS, wdata, COM_ADS_POINTER_SIZE);

    if (0 == ret)
        ret = OS_i2c_read_data(i_dev, COM_ADS1115_ADDRESS, rdata, COM_ADS_CONFIG_SIZE);

    if (0 == ret)
    {
        config = (t_uint16) ( (rdata[0] << 8) + rdata[1] );
        LOG_INF1("COM : current config read on the ADS = %#04hx", config);
    }

    return ret;
}

static t_int16 _com_ads_read_result(t_os_i2c_device i_dev)
{
    t_int16 res = 0;

    int ret = 0;
    t_uint8 wdata[COM_ADS_POINTER_SIZE];
    t_uint8 rdata[COM_ADS_CONVERSION_SIZE] = { 0 };

    /* Point to conversion register */
    wdata[0] = COM_ADS_CONVERSION_REGISTER;
    ret = OS_i2c_write_data(i_dev, COM_ADS1115_ADDRESS, wdata, COM_ADS_POINTER_SIZE);

    if (0 == ret)
        ret = OS_i2c_read_data(i_dev, COM_ADS1115_ADDRESS, rdata, COM_ADS_CONVERSION_SIZE);

    if (0 == ret)
        res = (t_int16) ( (rdata[0] << 8) + rdata[1] );

    return res;
}
