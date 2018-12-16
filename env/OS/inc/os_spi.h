#pragma once

typedef struct
{
    int fd;
    unsigned int speed;
    unsigned char mode;
    unsigned char bits_per_word;
    t_os_spi_device id;
} t_os_spi_struct;
