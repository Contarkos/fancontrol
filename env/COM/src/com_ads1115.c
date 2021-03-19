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
            .comp_queue = COM_ADS_COMP_ASSERT_DISABLE,
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
            .comp_queue = COM_ADS_COMP_ASSERT_DISABLE,
        }
    },
};

static t_com_ads_threshold com_ads_high_thresh[OS_I2C_DEVICE_NB] =
{
    /* Device 0 */
    {
        .bits =
        {
            .value = COM_ADS_MAX_VALUE,
            .mode = COM_ADS_RDY_MODE_HIGH,
        }
    },
    /* Device 1 */
    {
        .bits =
        {
            .value = COM_ADS_MAX_VALUE,
            .mode = COM_ADS_RDY_MODE_HIGH,
        }
    }
};

static t_com_ads_threshold com_ads_low_thresh[OS_I2C_DEVICE_NB] =
{
    /* Device 0 */
    {
        .bits =
        {
            .value = COM_ADS_MIN_VALUE,
            .mode = COM_ADS_RDY_MODE_LOW,
        }
    },
    /* Device 1 */
    {
        .bits =
        {
            .value = COM_ADS_MIN_VALUE,
            .mode = COM_ADS_RDY_MODE_LOW,
        }
    }
};

/*********************************************************************/
/*                       Static declarations                         */
/*********************************************************************/

static int _com_ads_set_config(t_os_i2c_device i_dev);
static int _com_ads_print_config(t_os_i2c_device i_dev);

static t_int16 _com_ads_read_result(t_os_i2c_device i_dev);

static int _com_ads_set_high_threshold(t_os_i2c_device i_dev);
static int _com_ads_set_low_threshold(t_os_i2c_device i_dev);

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
            LOG_INF1("COM : current config = %#04hx, default gain is %d", com_ads_config[i_device].setup.u16, com_ads_config[i_device].bits.pga);
    }

    return ret;
}

int COM_ads_set_comp_queue(t_os_i2c_device i_dev, t_com_ads_comp_queue i_comp_queue)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_comp_queue)
        {
            case COM_ADS_COMP_ASSERT_ONE:
            case COM_ADS_COMP_ASSERT_TWO:
            case COM_ADS_COMP_ASSERT_THREE:
            case COM_ADS_COMP_ASSERT_DISABLE:
                /* Update the current setup */
                com_ads_config[i_dev].bits.comp_queue = i_comp_queue;
                break;
            default:
                LOG_ERR("COM : wrong comparator queue for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set comparator queue for ADS");
        else
            ret = _com_ads_print_config(i_dev);
    }

    return ret;
}

int COM_ads_set_comp_latch(t_os_i2c_device i_dev, t_com_ads_comp_latch i_comp_latch)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_comp_latch)
        {
            case COM_ADS_COMP_LATCH_OFF:
            case COM_ADS_COMP_LATCH_ON:
                /* Update the current setup */
                com_ads_config[i_dev].bits.comp_latch = i_comp_latch;
                break;
            default:
                LOG_ERR("COM : wrong comparator latch for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set comparator latch for ADS");
        else
            ret = _com_ads_print_config(i_dev);
    }

    return ret;
}

int COM_ads_set_comp_pol(t_os_i2c_device i_dev, t_com_ads_comp_pol i_comp_pol)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_comp_pol)
        {
            case COM_ADS_COMP_POL_LOW:
            case COM_ADS_COMP_POL_HIGH:
                /* Update the current setup */
                com_ads_config[i_dev].bits.comp_pol = i_comp_pol;
                break;
            default:
                LOG_ERR("COM : wrong comparator polarity for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set comparator polarity for ADS");
        else
            ret = _com_ads_print_config(i_dev);
    }

    return ret;
}

int COM_ads_set_comp_mode(t_os_i2c_device i_dev, t_com_ads_comp_mode i_comp_mode)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_comp_mode)
        {
            case COM_ADS_COMP_MODE_TRAD:
            case COM_ADS_COMP_MODE_WINDOW:
                /* Update the current setup */
                com_ads_config[i_dev].bits.comp_mode = i_comp_mode;
                break;
            default:
                LOG_ERR("COM : wrong comparator mode for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set comparator mode for ADS");
        else
            ret = _com_ads_print_config(i_dev);
    }

    return ret;
}

int COM_ads_set_rate(t_os_i2c_device i_dev, t_com_ads_rate i_rate)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_rate)
        {
            case COM_ADS_RATE_8SPS:
            case COM_ADS_RATE_16SPS:
            case COM_ADS_RATE_32SPS:
            case COM_ADS_RATE_64SPS:
            case COM_ADS_RATE_128SPS:
            case COM_ADS_RATE_250SPS:
            case COM_ADS_RATE_475SPS:
            case COM_ADS_RATE_860SPS:
                /* Update the current setup */
                com_ads_config[i_dev].bits.rate = i_rate;
                break;
            default:
                LOG_ERR("COM : wrong rate for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set rate for ADS");
        else
            ret = _com_ads_print_config(i_dev);
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

int COM_ads_set_gain(t_os_i2c_device i_dev, t_com_ads_gain i_gain)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_gain)
        {
            case COM_ADS_GAIN_6144:
            case COM_ADS_GAIN_4096:
            case COM_ADS_GAIN_2048:
            case COM_ADS_GAIN_1024:
            case COM_ADS_GAIN_0512:
            case COM_ADS_GAIN_0256:
                /* Update the current setup */
                com_ads_config[i_dev].bits.pga = i_gain;
                break;
            default:
                LOG_ERR("COM : wrong gain for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_config(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set gain for ADS");
        else
            ret = _com_ads_print_config(i_dev);
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
                break;
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

int COM_ads_set_high_threshold(t_os_i2c_device i_dev, t_int16 i_value)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        /* We cannot accpet negative values or values lower than the low threshold */
        if ( (i_value < COM_ADS_MIN_VALUE) || (i_value < com_ads_low_thresh[i_dev].bits.value) )
        {
            LOG_ERR("COM : ADS high threshold : value is too low, command ignored");
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Update the value and sync it with the ADS */
        com_ads_high_thresh[i_dev].raw.i16 = i_value;
        ret = _com_ads_set_high_threshold(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set High threshold value");
    }

    return ret;
}

int COM_ads_set_low_threshold(t_os_i2c_device i_dev, t_int16 i_value)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        /* We cannot accept positive values or values higher than the high threshold */
        if ( (i_value >= COM_ADS_MIN_VALUE) || (i_value > com_ads_high_thresh[i_dev].bits.value) )
        {
            LOG_ERR("COM : ADS low threshold : value is too high, command ignored");
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* Update the value and sync it with the ADS */
        com_ads_low_thresh[i_dev].raw.i16 = i_value;
        ret = _com_ads_set_high_threshold(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set High threshold value");
    }

    return ret;
}

int COM_ads_config_ready(t_os_i2c_device i_dev, t_com_ads_rdy_mode i_rdy_mode)
{
    int ret = 0;

    if (i_dev >= OS_I2C_DEVICE_NB)
    {
        LOG_ERR("COM : wrong I2C device ID");
        ret = -1;
    }

    if (0 == ret)
    {
        switch (i_rdy_mode)
        {
            case COM_ADS_RDY_MODE_ALERT:
                com_ads_high_thresh[i_dev].bits.mode = COM_ADS_RDY_MODE_LOW;
                com_ads_low_thresh[i_dev].bits.mode = COM_ADS_RDY_MODE_HIGH;
                break;
            case COM_ADS_RDY_MODE_READY:
                com_ads_high_thresh[i_dev].bits.mode = COM_ADS_RDY_MODE_HIGH;
                com_ads_low_thresh[i_dev].bits.mode = COM_ADS_RDY_MODE_LOW;
                break;
            default:
                LOG_ERR("COM : wrong value for ready mode for ADS");
                ret = -1;
                break;
        }
    }

    if (0 == ret)
    {
        ret = _com_ads_set_high_threshold(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set High threshold value for ready mode");
    }

    if (0 == ret)
    {
        ret = _com_ads_set_low_threshold(i_dev);

        if (0 != ret)
            LOG_ERR("COM : could not set Low threshold value for ready mode");
    }

    return ret;
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
    data[1] = com_ads_config[i_dev].setup.u8.msb;
    data[2] = com_ads_config[i_dev].setup.u8.lsb;

    /* Send the command to the ADS */
    ret = OS_i2c_write_data(i_dev, COM_ADS1115_ADDRESS, data, size);

    return ret;
}

static int _com_ads_print_config(t_os_i2c_device i_dev)
{
    int ret = 0;
    t_com_ads_data data;
    t_uint8 wdata[COM_ADS_POINTER_SIZE];
    t_uint8 rdata[COM_ADS_CONFIG_SIZE] = { 0 };

    /* Point to conversion register */
    wdata[0] = COM_ADS_CONFIG_REGISTER;
    ret = OS_i2c_write_data(i_dev, COM_ADS1115_ADDRESS, wdata, COM_ADS_POINTER_SIZE);

    if (0 == ret)
        ret = OS_i2c_read_data(i_dev, COM_ADS1115_ADDRESS, rdata, COM_ADS_CONFIG_SIZE);

    if (0 == ret)
    {
        data.u8.msb = rdata[0];
        data.u8.lsb = rdata[1];
        LOG_INF1("COM : current config read on the ADS = %#04hx", data.u16);
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
    {
        t_com_ads_data data;

        data.u8.msb = rdata[0];
        data.u8.lsb = rdata[1];
        res = data.i16;
    }

    return res;
}

static int _com_ads_set_high_threshold(t_os_i2c_device i_dev)
{
    int ret = 0;

    const t_uint32 size = COM_ADS_POINTER_SIZE + COM_ADS_HI_THRESH_SIZE;
    t_uint8 data[COM_ADS_POINTER_SIZE + COM_ADS_HI_THRESH_SIZE] = { 0 };

    /* Fill the array */
    data[0] = COM_ADS_HI_THRESH_REGISTER;
    data[1] = com_ads_high_thresh[i_dev].raw.u8.msb;
    data[2] = com_ads_high_thresh[i_dev].raw.u8.lsb;

    /* Send the command to the ADS */
    ret = OS_i2c_write_data(i_dev, COM_ADS1115_ADDRESS, data, size);

    return ret;
}

static int _com_ads_set_low_threshold(t_os_i2c_device i_dev)
{
    int ret = 0;

    const t_uint32 size = COM_ADS_POINTER_SIZE + COM_ADS_LO_THRESH_SIZE;
    t_uint8 data[COM_ADS_POINTER_SIZE + COM_ADS_LO_THRESH_SIZE] = { 0 };

    /* Fill the array */
    data[0] = COM_ADS_LO_THRESH_REGISTER;
    data[1] = com_ads_low_thresh[i_dev].raw.u8.msb;
    data[2] = com_ads_low_thresh[i_dev].raw.u8.lsb;

    /* Send the command to the ADS */
    ret = OS_i2c_write_data(i_dev, COM_ADS1115_ADDRESS, data, size);

    return ret;
}
