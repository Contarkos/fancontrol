// Global includes
#include <fcntl.h>                //Needed for SPI port
#include <sys/ioctl.h>            //Needed for SPI port
#include <linux/spi/spidev.h>     //Needed for SPI port
#include <unistd.h>               //Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// Local includes
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_spi.h"

t_os_spi_struct spi_device_0 =
{
    .fd = 0,
    .mode = SPI_MODE_0, // CPOL = 0 et CPHA = 0
    .bits_per_word = 8, // 8 bits par mots
    .speed = 1000000,   // 1MHz
    .id = OS_SPI_DEVICE_0
};

t_os_spi_struct spi_device_1 =
{
    .fd = 0,
    .mode = SPI_MODE_0, // CPOL = 0 et CPHA = 0
    .bits_per_word = 8, // 8 bits par mots
    .speed = 1000000,   // 1MHz
    .id = OS_SPI_DEVICE_1
};

t_os_ret_okko is_init_spi = OS_RET_KO;

/***********************************/
/*          SPI OPEN PORT          */
/***********************************/
//spi_device    0=CS0, 1=CS1
int OS_spi_open_port (t_os_spi_device i_spi_id, unsigned char i_mode, unsigned char i_bits, unsigned int i_speed)
{
    int ret = 0;
    t_os_spi_struct *spi_device;

    switch (i_spi_id)
    {
        case OS_SPI_DEVICE_0:
            spi_device = &spi_device_0;
            spi_device->fd = open("/dev/spidev0.0", O_RDWR | O_NONBLOCK);
            break;
        case OS_SPI_DEVICE_1:
            spi_device = &spi_device_1;
            spi_device->fd = open("/dev/spidev0.1", O_RDWR | O_NONBLOCK);
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            ret = -1;
    }

    //----- SET SPI MODE -----
    //SPI_MODE_0 (0,0)     CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    //SPI_MODE_1 (0,1)     CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_2 (1,0)     CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_3 (1,1)     CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
    spi_device->mode = i_mode;

    //----- SET BITS PER WORD -----
    spi_device->bits_per_word = i_bits;

    //----- SET SPI BUS SPEED -----
    spi_device->speed = i_speed;        //1000000 = 1MHz (1uS per bit) 

    if (0 == ret)
    {
        LOG_INF1("OS : ouverture fichier SPI%d", spi_device->id);

        if (spi_device->fd < 0)
        {
            LOG_ERR("OS : Error - Could not open SPI device");
            return(1);
        }

        ret = ioctl(spi_device->fd, SPI_IOC_WR_MODE, &(spi_device->mode));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPIMode (WR)...ioctl fail");
            return(1);
        }

        ret = ioctl(spi_device->fd, SPI_IOC_RD_MODE, &(spi_device->mode));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPIMode (RD)...ioctl fail");
            return(1);
        }
        LOG_INF3("OS : spi mode = %d", spi_device->mode);

        ret = ioctl(spi_device->fd, SPI_IOC_WR_BITS_PER_WORD, &(spi_device->bits_per_word));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI bitsPerWord (WR)...ioctl fail, errno = %d", errno);
            return(1);
        }

        ret = ioctl(spi_device->fd, SPI_IOC_RD_BITS_PER_WORD, &(spi_device->bits_per_word));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI bitsPerWord(RD)...ioctl fail");
            return(1);
        }

#if 0
        int spi_lsb = 0;
        ret = ioctl(spi_device->fd, SPI_IOC_WR_LSB_FIRST, &spi_lsb);
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI LSB (WR)...ioctl fail errno = %d", errno);
            return(1);
        }

        ret = ioctl(spi_device->fd, SPI_IOC_RD_LSB_FIRST, &spi_lsb);
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI LSB (RD)...ioctl fail errno = %d", errno);
            return(1);
        }
#endif

        ret = ioctl(spi_device->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(spi_device->speed));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI speed (WR)...ioctl fail");
            return(1);
        }

        ret = ioctl(spi_device->fd, SPI_IOC_RD_MAX_SPEED_HZ, &(spi_device->speed));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI speed (RD)...ioctl fail");
            return(1);
        }
    }

    return(ret);
}

/************************************/
/*          SPI CLOSE PORT          */
/************************************/
int OS_spi_close_port (t_os_spi_device i_spi_id)
{
    int ret = 0;
    t_os_spi_struct *spi_device;

    switch (i_spi_id)
    {
        case OS_SPI_DEVICE_0:
            spi_device = &spi_device_0;
            break;
        case OS_SPI_DEVICE_1:
            spi_device = &spi_device_1;
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            ret = -1;
    }

    if (0 == ret)
    {
        ret = close(spi_device->fd);

        if(ret < 0)
        {
            LOG_ERR("OS : Could not close SPI device %d", spi_device->id);
        }
    }

    return(ret);
}

/*******************************************/
/*           SPI SET SPEED VALUE           */
/*******************************************/
int OS_spi_set_speed (t_os_spi_device i_spi_id, unsigned int i_speed)
{
    int ret = 0;
    t_os_spi_struct *spi_device;

    switch (i_spi_id)
    {
        case OS_SPI_DEVICE_0:
            spi_device = &spi_device_0;
            break;
        case OS_SPI_DEVICE_1:
            spi_device = &spi_device_1;
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            ret = -1;
    }

    if (0 == ret)
    {
        ret  = ioctl (spi_device->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(i_speed));
        ret += ioctl (spi_device->fd, SPI_IOC_RD_MAX_SPEED_HZ, &(i_speed));

        if (ret < 0)
        {
            LOG_ERR("[OS] Error while updating speed for SPI %d, speed unchanged = %d", spi_device->fd, spi_device->speed);
        }
        else
        {
            spi_device->speed = i_speed;
            LOG_INF3("[OS] Updated speed for SPI %d, speed = %d", spi_device->fd, spi_device->speed);
        }
    }

    return ret;
}


/*******************************************/
/*           SPI SET MODE VALUE            */
/*******************************************/
int OS_spi_set_mode(t_os_spi_device i_spi_id, t_os_spi_mode i_mode)
{
    int ret = 0;
    t_os_spi_struct *spi_device;

    switch (i_spi_id)
    {
        case OS_SPI_DEVICE_0:
            spi_device = &spi_device_0;
            break;
        case OS_SPI_DEVICE_1:
            spi_device = &spi_device_1;
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            ret = -1;
    }

    if (0 == ret)
    {
        switch (i_mode)
        {
            case OS_SPI_MODE_0:
            case OS_SPI_MODE_1:
            case OS_SPI_MODE_2:
            case OS_SPI_MODE_3:
                break;
            default:
                ret = -2;
                break;
        }

        if (ret < 0)
        {
            LOG_ERR("[OS] Wrong value for SPI mode : %d", i_mode);
        }
        else
        {
            ret  = ioctl (spi_device->fd, SPI_IOC_WR_MODE, &(i_mode));
            ret += ioctl (spi_device->fd, SPI_IOC_RD_MODE, &(i_mode));

            if (ret < 0)
            {
                LOG_ERR("[OS] Error while updating mode for SPI %d, mode unchanged = %d", spi_device->fd, spi_device->mode);
            }
            else
            {
                spi_device->mode = i_mode;
                LOG_INF3("[OS] Updated mode for SPI %d, mode = %d", spi_device->fd, spi_device->mode);
            }
        }
    }

    return ret;
}

/*******************************************/
/*          SPI WRITE & READ DATA          */
/*******************************************/
//data        Bytes to write.  Contents is overwritten with bytes read.
int OS_spi_write_read (t_os_spi_device i_spi_id, unsigned char *data, int length)
{
    struct spi_ioc_transfer spi[length];
    int ii = 0;
    int retVal = 0, ret = 0;
    t_os_spi_struct *spi_device = NULL;

    switch (i_spi_id)
    {
        case OS_SPI_DEVICE_0:
            spi_device = &spi_device_0;
            break;
        case OS_SPI_DEVICE_1:
            spi_device = &spi_device_1;
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            ret = -1;
    }

    if ( spi_device )
    {
        printf("[OS] Data TX : ");
        for (ii = 0; ii < length; ii++)
        {
            printf("[%d] = %x, ", ii, *(data + ii));
        }
        printf("\n");

        //one spi transfer for each byte
        for (ii = 0 ; ii < length ; ii++)
        {
            // init de la zone memoire
            memset(&spi[ii], 0, sizeof (spi[ii]));

            // Remplissage de la demande de transfert
            spi[ii].tx_buf        = (unsigned long)(data + ii); // transmit from "data"
            spi[ii].rx_buf        = (unsigned long)(data + ii); // receive into "data"
            spi[ii].len           = sizeof(*(data + ii));
            spi[ii].delay_usecs   = 0;
            spi[ii].speed_hz      = spi_device->speed;
            spi[ii].bits_per_word = spi_device->bits_per_word;
            spi[ii].cs_change     = 0;
        }

        retVal = ioctl(spi_device->fd, SPI_IOC_MESSAGE(length), spi);

        if(retVal < 0)
        {
            LOG_ERR("Error - Problem transmitting spi data..ioctl");
            ret = -2;
        }
        else if (length != retVal)
        {
            LOG_WNG("COM : warning nombre de messages envoyés incohérents, %d != %d", length, retVal);
            ret = -4;
        }

        printf("[OS] Data RX : ");
        for (ii = 0; ii < length; ii++)
        {
            printf("[%d] = %x, ", ii, *(data + ii));
        }
        printf("\n");
    }

    return ret;
}
