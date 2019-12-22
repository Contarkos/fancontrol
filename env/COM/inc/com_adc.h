#pragma once

/*****************************************************************************/
/*              Definition des registres et shift pour ADC                   */
/*****************************************************************************/

/* Message nul dans le cas de la lecture */
#define COM_ADC_NULL            (0x00)

/* Registre COM */
#define COM_ADC_CHAN_SHIFT      0
#define COM_ADC_CHAN_MASK       ((t_uint8) 0x03)
#define COM_ADC_STDBY_SHIFT     2
#define COM_ADC_STDBY_MASK      ((t_uint8) 0x04)
#define COM_ADC_RW_SHIFT        3
#define COM_ADC_RW_MASK         ((t_uint8) 0x08)
#define COM_ADC_REG_SHIFT       4
#define COM_ADC_REG_MASK        ((t_uint8) 0x70)
#define COM_ADC_WRITE_SHIFT     7
#define COM_ADC_WRITE_MASK      ((t_uint8) 0x80)

/* Registre SETUP */
#define COM_ADC_SET_FILT_SHIFT  0
#define COM_ADC_SET_FILT_MASK   ((t_uint8) 0x01)
#define COM_ADC_SET_BUF_SHIFT   1
#define COM_ADC_SET_BUF_MASK    ((t_uint8) 0x02)
#define COM_ADC_SET_BIP_SHIFT   2
#define COM_ADC_SET_BIP_MASK    ((t_uint8) 0x04)
#define COM_ADC_SET_GAIN_SHIFT  3
#define COM_ADC_SET_GAIN_MASK   ((t_uint8) 0x38)
#define COM_ADC_SET_MODE_SHIFT  6
#define COM_ADC_SET_MODE_MASK   ((t_uint8) 0xC0)

/* Registre CLOCK */
#define COM_ADC_CLK_FILT_SHIFT  0
#define COM_ADC_CLK_FILT_MASK   ((t_uint8) 0x03)
#define COM_ADC_CLK_SET_SHIFT   2
#define COM_ADC_CLK_SET_MASK    ((t_uint8) 0x04)
#define COM_ADC_CLK_DIV_SHIFT   3
#define COM_ADC_CLK_DIV_MASK    ((t_uint8) 0x08)
#define COM_ADC_CLK_DIS_SHIFT   4
#define COM_ADC_CLK_DIS_MASK    ((t_uint8) 0x10)

/* Variables globales */
extern t_com_adc_setup com_spi_device_array[OS_SPI_DEVICE_NB];

/* Fonctions locales */
t_com_adc_setup* com_adc_get_setup(t_os_spi_device i_device);

int com_adc_config_setup(t_os_spi_device i_device);
int com_adc_config_clock(t_os_spi_device i_device);

int com_adc_wait_ready(t_os_spi_device i_device);
