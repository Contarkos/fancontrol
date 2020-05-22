#ifndef OS_SPI_H_
#define OS_SPI_H_

/* Defines */
#define OS_FILE_SPI0    "/dev/spidev0.0"
#define OS_FILE_SPI1    "/dev/spidev0.1"

/* Struct */
typedef struct
{
    char filename[OS_MAX_LENGTH_FILENAME];
    int fd;
    unsigned int speed;
    unsigned char mode;
    unsigned char bits_per_word;
    t_os_spi_device id;
} t_os_spi_struct;

/* Local functions */
t_os_spi_struct* os_spi_get_device(t_os_spi_device i_device);

#endif /* OS_SPI_H_ */

