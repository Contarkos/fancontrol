#pragma once

#include <stdio.h>

#include "base.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

// Defines
#define COM_UNIX_PATH_MAX       108
#define COM_ADC_MAXVALUE        (65535)     // Valeur max en sortie de l'ADC
#define COM_MAX_NB_MSG          256
#define COM_MAX_SIZE_DATA       1024        // Taille max d'un message à l'envoi des messages
#define COM_EXTERN_BACKLOG      128

#define COM_ADC_PIN_RDY         (25)
#define COM_ADC_PIN_ENB         (27)
#define COM_ADC_BITS_PER_WORD   8
#define COM_ADC_SPEED           (2457600U)

// Typedef
typedef enum
{
    COM_STATE_OFF = 0,
    COM_STATE_ON  = 1
} t_com_state;

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

typedef enum
{
    COM_ADC_MODE_NORMAL = 0,
    COM_ADC_MODE_SELFCAL = 1,
    COM_ADC_MODE_ZEROCAL = 2,
    COM_ADC_MODE_FULLCAL = 3
} t_com_adc_mode;

typedef enum
{
    COM_ADC_GAIN_1 = 0,
    COM_ADC_GAIN_2 = 1,
    COM_ADC_GAIN_4 = 2,
    COM_ADC_GAIN_8 = 3,
    COM_ADC_GAIN_16 = 4,
    COM_ADC_GAIN_32 = 5,
    COM_ADC_GAIN_64 = 6,
    COM_ADC_GAIN_128 = 7
} t_com_adc_gain;

typedef enum
{
    COM_ADC_CLOCK_ON = 0,
    COM_ADC_CLOCK_OFF = 1
} t_com_adc_clock;

typedef enum
{
    COM_ADC_CLOCK_1MHZ = 0,
    COM_ADC_CLOCK_2MHZ4 = 1
} t_com_adc_clock_rate;

typedef enum
{
    COM_ADC_CLOCK_FILT_20_50 = 0,
    COM_ADC_CLOCK_FILT_25_60 = 1,
    COM_ADC_CLOCK_FILT_100_250 = 2,
    COM_ADC_CLOCK_FILT_200_500 = 3
} t_com_adc_clock_filt;

typedef struct
{
    t_com_adc_clock clk_disable;
    t_com_state clk_div;
    t_com_adc_clock_rate clk_rate;
    t_com_adc_clock_filt clk_filter;
    t_com_adc_pair pair;
    t_com_adc_mode mode;
    t_com_adc_gain gain;
    t_com_state bipolarity;
    t_com_state buffer_mode;
    t_com_state filter_sync;
} t_com_adc_setup;

typedef struct
{
    t_uint32 addr;
    t_uint16 port;
} __attribute__((packed)) t_com_inet_data;

typedef struct
{
    t_uint32 id;
    char data[COM_MAX_SIZE_DATA];
} __attribute__((packed)) t_com_msg;

// Fonctions init et stop
int COM_init(void);
int COM_stop(void);

// Fonctions API
int COM_create_socket(int i_family, int i_type, int i_proto, char *i_data);
int COM_connect_socket(int i_family, int i_type, char * i_data, int *o_fd);
int COM_socket_listen(int i_fd, int i_backlog);
int COM_send_data(int i_fd, t_uint32 i_id, void * i_data, size_t i_size, int i_flags);
int COM_receive_data(int i_sock, t_com_msg *o_m, int *o_size);
int COM_close_socket(int i_fd);

int COM_register_socket(int i_fd, int *i_list, int i_size);

// Gestion de l'AD7705
int COM_adc_init(t_os_spi_device i_device, t_com_adc_clock_rate i_rate);
int COM_adc_reset(t_os_spi_device i_device);

t_uint16 COM_adc_read_result(t_os_spi_device i_device, t_com_adc_pair i_pair);

int COM_adc_read_setup(t_os_spi_device i_device, t_uint8 *o_setup);
int COM_adc_set_filter_sync(t_os_spi_device i_device, t_com_state i_filter_sync);
int COM_adc_set_buffer_mode(t_os_spi_device i_device, t_com_state i_buffer_mode);
int COM_adc_set_bipolarity(t_os_spi_device i_device, t_com_state i_bipolarity);
int COM_adc_set_gain(t_os_spi_device i_device, t_com_adc_gain i_gain);
int COM_adc_set_mode(t_os_spi_device i_device, t_com_adc_mode i_mode);

int COM_adc_read_clock(t_os_spi_device i_device, t_uint8 *o_clock);
int COM_adc_enable_clock(t_os_spi_device i_device, t_com_adc_clock i_clock);
int COM_adc_set_clock_rate(t_os_spi_device i_device, t_com_adc_clock_rate i_rate);
int COM_adc_set_clock_div(t_os_spi_device i_device, t_com_state i_div);
int COM_adc_set_clock_filter(t_os_spi_device i_device, t_com_adc_clock_filt i_filter);

#ifdef __cplusplus
}
#endif
