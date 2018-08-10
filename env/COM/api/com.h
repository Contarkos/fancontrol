#pragma once

#include <stdio.h>

#include "base.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

// Defines
#define COM_UNIX_PATH_MAX   108
#define COM_ADC_MAXVALUE    (65535)   // Valeur max en sortie de l'ADC

// Typedef
typedef struct
{
    t_uint32 addr;
    t_uint16 port;
} t_com_inet_data;

typedef enum
{
    COM_ADC_REG_COM = 0,
    COM_ADC_REG_SETUP = 1,
    COM_ADC_REG_CLOCK = 2,
    COM_ADC_REG_DATA = 3,
    COM_ADC_REG_TEST = 4,
    COM_ADC_REG_NOOP = 5,
    COM_ADC_REG_OFFSET = 6,
    COM_ADC_REG_GAIN = 7,
} t_com_adc_register;

typedef enum
{
    COM_ADC_PAIR_0 = 0, // AIN1(+) / AIN1(-)
    COM_ADC_PAIR_1 = 1, // AIN2(+) / AIN2(-)
    COM_ADC_PAIR_2 = 2, // AIN1(-) / AIN1(-)
    COM_ADC_PAIR_3 = 3, // AIN1(-) / AIN2(-)
} t_com_adc_pair;

// Fonctions API
int COM_create_socket(int i_family, int i_type, int i_proto, char *i_data);

// Gestion de l'AD7705
t_uint16 COM_adc_read_result(t_os_spi_device i_device, t_com_adc_pair i_pair);
int COM_adc_set_gain(t_os_spi_device i_device, t_uint8 i_gain);

#ifdef __cplusplus
}
#endif
