/* Global includes */
#include <fcntl.h>                /*Needed for SPI port */
#include <sys/ioctl.h>            /*Needed for SPI port */
#include <linux/spi/spidev.h>     /*Needed for SPI port */
#include <unistd.h>               /*Needed for SPI port */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "os_spi.h"

t_os_spi_struct spi_devices_array[OS_SPI_DEVICE_NB] = 
{
    {
        .filename = OS_FILE_SPI0,
        .fd = 0,
        .mode = SPI_MODE_0, /* CPOL = 0 et CPHA = 0 */
        .bits_per_word = 8, /* 8 bits par mots */
        .speed = 1000000,   /* 1MHz */
        .id = OS_SPI_DEVICE_0
    },
    {
        .filename = OS_FILE_SPI1,
        .fd = 0,
        .mode = SPI_MODE_0, /* CPOL = 0 et CPHA = 0 */
        .bits_per_word = 8, /* 8 bits par mots */
        .speed = 1000000,   /* 1MHz */
        .id = OS_SPI_DEVICE_1
    },
};

t_os_ret_okko is_init_spi = OS_RET_KO;

/* Mutex pour les acces aux registres SPI */
OS_mutex_t os_spi_mutex = OS_INIT_MUTEX;

/***********************************/
/*          SPI OPEN PORT          */
/***********************************/
/*spi_device    0=CS0, 1=CS1 */
int OS_spi_open_port (t_os_spi_device i_spi_id, unsigned char i_mode, unsigned char i_bits, unsigned int i_speed)
{
    int ret = 0;
    t_os_spi_struct *spi_device;

    spi_device = os_spi_get_device(i_spi_id);

    if (NULL == spi_device)
    {
        LOG_ERR("OS : device SPI inexistant");
        ret = -1;
    }

    if (0 == ret)
    {
        spi_device->fd = open(spi_device->filename, O_RDWR | O_NONBLOCK);

        if (spi_device->fd < 0)
        {
            LOG_ERR("OS : bad file descriptor, fd = %d, err = %d", spi_device->fd, errno);
            ret = -2;
        }
    }

    if (0 == ret)
    {
        /*----- SET SPI MODE ----- */
        /*SPI_MODE_0 (0,0)     CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge */
        /*SPI_MODE_1 (0,1)     CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge */
        /*SPI_MODE_2 (1,0)     CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge */
        /*SPI_MODE_3 (1,1)     CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge */
        switch (i_mode)
        {
            case SPI_MODE_0:
            case SPI_MODE_1:
            case SPI_MODE_2:
            case SPI_MODE_3:
                spi_device->mode = i_mode;
                break;
            default:
                ret += -2;
                break;
        }

        /*----- SET BITS PER WORD ----- */
        switch (i_bits)
        {
            case OS_SPI_BITS_WORD_8:
            case OS_SPI_BITS_WORD_9:
                spi_device->bits_per_word = i_bits;
                break;
            default:
                ret += -4;
                break;
        }

        /*----- SET SPI BUS SPEED ----- */
        if (i_speed <= OS_MAX_SPI_SPEED)
            spi_device->speed = i_speed;
        else
            ret += -8;
    }

    if (0 == ret)
    {
        LOG_INF1("OS : ouverture fichier SPI%d", spi_device->id);

        if (spi_device->fd < 0)
        {
            LOG_ERR("OS : Error - Could not open SPI device");
            return 1;
        }

        ret = ioctl(spi_device->fd, SPI_IOC_WR_MODE, &(spi_device->mode));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPIMode (WR)...ioctl fail");
            return 1;
        }

        ret = ioctl(spi_device->fd, SPI_IOC_RD_MODE, &(spi_device->mode));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPIMode (RD)...ioctl fail");
            return 1;
        }

        ret = ioctl(spi_device->fd, SPI_IOC_WR_BITS_PER_WORD, &(spi_device->bits_per_word));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI bitsPerWord (WR)...ioctl fail, errno = %d", errno);
            return 1;
        }

        ret = ioctl(spi_device->fd, SPI_IOC_RD_BITS_PER_WORD, &(spi_device->bits_per_word));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI bitsPerWord(RD)...ioctl fail");
            return 1;
        }

        ret = ioctl(spi_device->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(spi_device->speed));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI speed (WR)...ioctl fail");
            return 1;
        }

        ret = ioctl(spi_device->fd, SPI_IOC_RD_MAX_SPEED_HZ, &(spi_device->speed));
        if(ret < 0)
        {
            LOG_ERR("OS : Could not set SPI speed (RD)...ioctl fail");
            return 1;
        }

        LOG_INF3("OS : SPI status for SPI%d : fd = %d, mode = %d, BPW = %d, speed = %d",
                 spi_device->id,
                 spi_device->fd,
                 spi_device->mode,
                 spi_device->bits_per_word,
                 spi_device->speed);
    }

    return ret;
}

/************************************/
/*          SPI CLOSE PORT          */
/************************************/
int OS_spi_close_port (t_os_spi_device i_spi_id)
{
    int ret = 0;
    t_os_spi_struct *spi_device;

    spi_device = os_spi_get_device(i_spi_id);

    if (NULL == spi_device)
    {
        LOG_ERR("OS : device SPI inexistant");
        ret = -1;
    }

    if (0 == ret)
    {
        ret = close(spi_device->fd);

        if(ret < 0)
        {
            LOG_ERR("OS : Could not close SPI device %d with fd = %d, err = %d", spi_device->id, spi_device->fd, errno);
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

    spi_device = os_spi_get_device(i_spi_id);

    if (NULL == spi_device)
    {
        LOG_ERR("OS : device SPI inexistant");
        ret = -1;
    }

    if (0 == ret)
    {
        if (i_speed > OS_MAX_SPI_SPEED)
        {
            LOG_ERR("OS : wrong value for SPI speed, i_speed = %d", i_speed);
            ret = -2;
        }
        else
        {
            ret = OS_mutex_lock(&os_spi_mutex);

            if (ret < 0)
            {
                LOG_ERR("OS : error while locking mutex for SPI set SPEED, ret = %d", ret);
            }
            else
            {
                ret  = ioctl (spi_device->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(i_speed));
                ret += ioctl (spi_device->fd, SPI_IOC_RD_MAX_SPEED_HZ, &(i_speed));

                OS_mutex_unlock(&os_spi_mutex);

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

    spi_device = os_spi_get_device(i_spi_id);

    if (NULL == spi_device)
    {
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
            LOG_ERR("OS : Wrong value for SPI mode : %d", i_mode);
        }
        else
        {
            ret = OS_mutex_lock(&os_spi_mutex);

            if (ret < 0)
            {
                LOG_ERR("OS : error while locking mutex for SPI set MODE, ret = %d", ret);
            }
            else
            {
                ret  = ioctl (spi_device->fd, SPI_IOC_WR_MODE, &(i_mode));
                ret += ioctl (spi_device->fd, SPI_IOC_RD_MODE, &(i_mode));

                OS_mutex_unlock(&os_spi_mutex);

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
    }

    return ret;
}

/*******************************************/
/*          SPI WRITE & READ DATA          */
/*******************************************/
/*io_data        Bytes to write.  Contents is overwritten with bytes read. */
int OS_spi_write_read (t_os_spi_device i_spi_id, unsigned char *io_data, int i_length)
{
    struct spi_ioc_transfer spi[i_length];
    int ii = 0;
    int retVal = 0, ret = 0;
    t_os_spi_struct *spi_device = NULL;

    spi_device = os_spi_get_device(i_spi_id);

    if (NULL == spi_device)
    {
        LOG_ERR("OS : device SPI inexistant");
        ret = -1;
    }

    if (0 == ret)
    {
#if defined(INTEGRATION_LOG_LEVEL) && (INTEGRATION_LOG_LEVEL >= 6)
        printf("[OS] Data TX : ");
        for (ii = 0; ii < i_length; ii++)
        {
            printf("[%d] = 0x%02x, ", ii, *(io_data + ii));
        }
        printf("\n");
#endif

        /*one spi transfer for each byte */
        for (ii = 0 ; ii < i_length ; ii++)
        {
            /* init de la zone memoire */
            memset(&spi[ii], 0, sizeof (spi[ii]));

            /* Remplissage de la demande de transfert */
            spi[ii].tx_buf        = (unsigned long)(io_data + ii); /* transmit from "io_data" */
            spi[ii].rx_buf        = (unsigned long)(io_data + ii); /* receive into "io_data" */
            spi[ii].len           = sizeof(*(io_data + ii));
            spi[ii].delay_usecs   = 0;
            spi[ii].speed_hz      = spi_device->speed;
            spi[ii].bits_per_word = spi_device->bits_per_word;
            spi[ii].cs_change     = 0;
        }

        retVal = OS_mutex_lock(&os_spi_mutex);

        if (retVal < 0)
        {
           LOG_ERR("OS : error while locking mutex for SPI R/W, ret = %d", retVal);
        }
        else
        {
            retVal = ioctl(spi_device->fd, SPI_IOC_MESSAGE(i_length), spi);

            OS_mutex_unlock(&os_spi_mutex);

            if(retVal < 0)
            {
                LOG_ERR("Error - Problem transmitting spi data..ioctl");
                ret = -2;
            }
            else if (i_length != retVal)
            {
                LOG_WNG("COM : warning nombre de messages envoyés incohérents, %d != %d", i_length, retVal);
                ret = -4;
            }

#if defined(INTEGRATION_LOG_LEVEL) && (INTEGRATION_LOG_LEVEL >= 6)
            printf("[OS] Data RX : ");
            for (ii = 0; ii < i_length; ii++)
            {
                printf("[%d] = 0x%02x, ", ii, *(io_data + ii));
            }
            printf("\n");
#endif
        }
    }

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

t_os_spi_struct* os_spi_get_device(t_os_spi_device i_device)
{
    if (i_device < OS_SPI_DEVICE_NB)
        return &spi_devices_array[i_device];
    else
        return NULL;
}
