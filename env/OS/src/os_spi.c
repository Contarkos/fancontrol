#include <fcntl.h>                //Needed for SPI port
#include <sys/ioctl.h>            //Needed for SPI port
#include <linux/spi/spidev.h>     //Needed for SPI port
#include <unistd.h>               //Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Local includes
#include "base.h"
#include "integ_log.h"
#include "os.h"

int spi_cs0_fd;                //file descriptor for the SPI device
int spi_cs1_fd;                //file descriptor for the SPI device
unsigned char spi_mode;
unsigned char spi_bitsPerWord;
unsigned int spi_speed;

t_os_ret_okko is_init_spi = OS_RET_KO;

/***********************************/
/*          SPI OPEN PORT          */
/***********************************/
//spi_device    0=CS0, 1=CS1
int OS_spi_open_port (t_os_spi_device spi_device)
{
    int status_value = 0;
    int *spi_cs_fd;

    //----- SET SPI MODE -----
    //SPI_MODE_0 (0,0)     CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    //SPI_MODE_1 (0,1)     CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_2 (1,0)     CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
    //SPI_MODE_3 (1,1)     CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
    spi_mode = SPI_MODE_0;

    //----- SET BITS PER WORD -----
    spi_bitsPerWord = 8;

    //----- SET SPI BUS SPEED -----
    spi_speed = 976000;        //1000000 = 1MHz (1uS per bit) 

    switch (spi_device)
    {
        case OS_SPI_DEVICE_0:
            spi_cs_fd = &spi_cs0_fd;
            *spi_cs_fd = open("/dev/spidev0.0", O_RDWR | O_NONBLOCK);
            break;
        case OS_SPI_DEVICE_1:
            spi_cs_fd = &spi_cs1_fd;
            *spi_cs_fd = open("/dev/spidev0.1", O_RDWR | O_NONBLOCK);
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            status_value = -1;
    }

    if (0 == status_value)
    {
        LOG_INF1("OS : ouverture fichier SPI%d", spi_device);

        if (*spi_cs_fd < 0)
        {
            LOG_ERR("OS : Error - Could not open SPI device");
            return(1);
        }

        status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
        if(status_value < 0)
        {
            LOG_ERR("OS : Could not set SPIMode (WR)...ioctl fail");
            return(1);
        }

        status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
        if(status_value < 0)
        {
            LOG_ERR("OS : Could not set SPIMode (RD)...ioctl fail");
            return(1);
        }

        status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
        if(status_value < 0)
        {
            LOG_ERR("OS : Could not set SPI bitsPerWord (WR)...ioctl fail");
            return(1);
        }

        status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
        if(status_value < 0)
        {
            LOG_ERR("OS : Could not set SPI bitsPerWord(RD)...ioctl fail");
            return(1);
        }

        status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
        if(status_value < 0)
        {
            LOG_ERR("OS : Could not set SPI speed (WR)...ioctl fail");
            return(1);
        }

        status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
        if(status_value < 0)
        {
            LOG_ERR("OS : Could not set SPI speed (RD)...ioctl fail");
            return(1);
        }
    }

    return(status_value);
}

/************************************/
/*          SPI CLOSE PORT          */
/************************************/
int OS_spi_close_port (t_os_spi_device spi_device)
{
    int status_value = 0;
    int *spi_cs_fd;

    switch (spi_device)
    {
        case OS_SPI_DEVICE_0:
            spi_cs_fd = &spi_cs0_fd;
            break;
        case OS_SPI_DEVICE_1:
            spi_cs_fd = &spi_cs1_fd;
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            status_value = -1;
    }

    if (0 == status_value)
    {
        status_value = close(*spi_cs_fd);

        if(status_value < 0)
        {
            LOG_ERR("OS : Could not close SPI device");
        }
    }

    return(status_value);
}

/*******************************************/
/*          SPI WRITE & READ DATA          */
/*******************************************/
//data        Bytes to write.  Contents is overwritten with bytes read.
int OS_spi_write_read (t_os_spi_device spi_device, unsigned char *data, int length)
{
    struct spi_ioc_transfer spi[length];
    int i = 0;
    int retVal = 0;
    int *spi_cs_fd;

    switch (spi_device)
    {
        case OS_SPI_DEVICE_0:
            spi_cs_fd = &spi_cs0_fd;
            break;
        case OS_SPI_DEVICE_1:
            spi_cs_fd = &spi_cs1_fd;
            break;
        default:
            LOG_ERR("OS : device SPI inexistant");
            retVal = -1;
    }

    if ( (retVal < 0) )
    {
        LOG_ERR("OS : module SPI non chargé");
        retVal = -2;
    }
    else
    {
        //one spi transfer for each byte
        for (i = 0 ; i < length ; i++)
        {
            memset(&spi[i], 0, sizeof (spi[i]));
            spi[i].tx_buf        = (unsigned long)(data + i); // transmit from "data"
            spi[i].rx_buf        = (unsigned long)(data + i); // receive into "data"
            spi[i].len           = sizeof(*(data + i));
            spi[i].delay_usecs   = 0;
            spi[i].speed_hz      = spi_speed;
            spi[i].bits_per_word = spi_bitsPerWord;
            spi[i].cs_change = 0;
        }

        retVal = ioctl(*spi_cs_fd, SPI_IOC_MESSAGE(length), &spi);

        if(retVal < 0)
        {
            LOG_ERR("Error - Problem transmitting spi data..ioctl");
            retVal = -4;
        }
    }

    return retVal;
}
