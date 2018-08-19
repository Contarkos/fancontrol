
// Global includes

// Local includes
#include "base.h"
#include "com.h"
#include "os.h"

#include "com_adc.h"

// Defines
#define COM_BYTE_SHIFT      8
#define COM_DATA_LENGTH     2
#define COM_SETUP_LENGTH    1
#define COM_CLOCK_LENGTH    1

// Variables globales
t_com_adc_setup com_device_0_setup =
{
    .clk_disable = COM_ADC_CLOCK_ON,
    .clk_div = COM_STATE_OFF,
    .clk_rate = COM_ADC_CLOCK_1MHZ,
    .clk_filter = COM_ADC_CLOCK_FILT_25_60,
    .pair = COM_ADC_PAIR_0,
    .mode = COM_ADC_MODE_NORMAL,
    .gain = COM_ADC_GAIN_1,
    .bipolarity = COM_STATE_OFF,
    .buffer_mode = COM_STATE_OFF,
    .filter_sync = COM_STATE_ON
};

t_com_adc_setup com_device_1_setup =
{
    .clk_disable = COM_ADC_CLOCK_ON,
    .clk_div = COM_STATE_OFF,
    .clk_rate = COM_ADC_CLOCK_1MHZ,
    .clk_filter = COM_ADC_CLOCK_FILT_25_60,
    .pair = COM_ADC_PAIR_0,
    .mode = COM_ADC_MODE_NORMAL,
    .gain = COM_ADC_GAIN_1,
    .bipolarity = COM_STATE_OFF,
    .buffer_mode = COM_STATE_OFF,
    .filter_sync = COM_STATE_ON
};

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

t_uint16 COM_adc_read_result(t_os_spi_device i_device, t_com_adc_pair i_pair)
{
    t_uint16 result = 0;
    t_uint8 data[COM_DATA_LENGTH + 1];
    int ret = 0;

    // Sauvegarde de la pair demandée
    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.pair = i_pair;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.pair = i_pair;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Mise en forme du buffer pour lire le registre de comparaison
    data[0] = (t_uint8)  (  COM_ADC_WRITE_MASK                          // Ecriture dans le registre de com
                          | (COM_ADC_REG_DATA << COM_ADC_REG_SHIFT)     // Selection du registre de données
                          | COM_ADC_RW_MASK                             // Lecture des données
                          | (i_pair << COM_ADC_CHAN_SHIFT) );           // Selection de la paire a ecouter

    // Vide pour ne pas bloquer la lecture
    data[1] = COM_ADC_NULL;
    data[2] = COM_ADC_NULL;

    // On va récupérer les données
    ret = OS_spi_write_read(i_device, data, COM_DATA_LENGTH + 1);

    if (ret < 0)
    {
        printf("[ER] COM : AD7705, pas de données disponibles...\n");
    }
    else
    {
        result = (t_uint16) ( (data[1] << COM_BYTE_SHIFT) | data[2]);
    }

    return result;
}

int COM_adc_set_filter_sync(t_os_spi_device i_device, t_com_state i_filter_sync)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.filter_sync = i_filter_sync;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.filter_sync = i_filter_sync;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);
    }

    return ret;
}

int COM_adc_set_buffer_mode(t_os_spi_device i_device, t_com_state i_buffer_mode)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.buffer_mode = i_buffer_mode;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.buffer_mode = i_buffer_mode;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);
    }

    return ret;
}

int COM_adc_set_bipolarity(t_os_spi_device i_device, t_com_state i_bipolarity)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.bipolarity = i_bipolarity;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.bipolarity = i_bipolarity;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);
    }

    return ret;
}

int COM_adc_set_gain(t_os_spi_device i_device, t_com_adc_gain i_gain)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.gain = i_gain;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.gain = i_gain;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);
    }

    return ret;
}

int COM_adc_set_mode(t_os_spi_device i_device, t_com_adc_mode i_mode)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.mode = i_mode;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.mode = i_mode;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);
    }

    return ret;
}

int COM_adc_enable_clock(t_os_spi_device i_device, t_com_adc_clock i_clock)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.clk_disable = i_clock;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.clk_disable = i_clock;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_clock(i_device);
    }

    return ret;
}

int COM_adc_set_clock_rate(t_os_spi_device i_device, t_com_adc_clock_rate i_rate)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.clk_rate = i_rate;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.clk_rate = i_rate;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_clock(i_device);
    }

    return ret;
}

int COM_adc_set_clock_div(t_os_spi_device i_device, t_com_state i_div)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.clk_div = i_div;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.clk_div = i_div;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_clock(i_device);
    }

    return ret;
}

int COM_adc_set_clock_filter(t_os_spi_device i_device, t_com_adc_clock_filt i_filter)
{
    int ret = 0;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.clk_filter = i_filter;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.clk_filter = i_filter;
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_clock(i_device);
    }

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

int com_adc_config_setup(t_os_spi_device i_device)
{
    int ret = 0;
    t_uint8 data[COM_SETUP_LENGTH + 1];
    t_com_adc_setup *s;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            s = &(com_device_0_setup);
            break;
        case OS_SPI_DEVICE_1:
            s = &(com_device_1_setup);
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        // Mise en forme du buffer pour écrire dans le registre de configuration
        data[0] = (t_uint8)  (   COM_ADC_WRITE_MASK                             // Ecriture dans le registre de com
                               | (COM_ADC_REG_SETUP << COM_ADC_REG_SHIFT)       // Selection du registre de données
                               | (0 << COM_ADC_RW_MASK)                         // Ecriture des données
                               | (s->pair << COM_ADC_CHAN_SHIFT));              // On reste sur la dernière paire demandée

        // Ecriture de la config courante
        data[1] = (t_uint8) (   (s->mode << COM_ADC_SET_MODE_SHIFT)
                              | (s->gain << COM_ADC_SET_GAIN_SHIFT)
                              | (s->bipolarity << COM_ADC_SET_BIP_SHIFT)
                              | (s->buffer_mode << COM_ADC_SET_BUF_SHIFT)
                              | (s->filter_sync << COM_ADC_SET_FILT_SHIFT)
                );

        // On va écrire les données
        ret = OS_spi_write_read(i_device, data, COM_SETUP_LENGTH + 1);
    }

    return ret;
}

int com_adc_config_clock(t_os_spi_device i_device)
{
    int ret = 0;
    t_uint8 data[COM_CLOCK_LENGTH + 1];
    t_com_adc_setup *s;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            s = &(com_device_0_setup);
            break;
        case OS_SPI_DEVICE_1:
            s = &(com_device_1_setup);
            break;
        default:
            printf("[ER] COM : device inexistant, device = %d\n", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        // Mise en forme du buffer pour écrire dans le registre de configuration
        data[0] = (t_uint8)  (   COM_ADC_WRITE_MASK                             // Ecriture dans le registre de com
                               | (COM_ADC_REG_CLOCK << COM_ADC_REG_SHIFT)       // Selection du registre de données
                               | (0 << COM_ADC_RW_MASK)                         // Ecriture des données
                               | (s->pair << COM_ADC_CHAN_SHIFT));              // On reste sur la dernière paire demandée

        // Ecriture de la config courante
        data[1] = (t_uint8) (   (s->clk_disable << COM_ADC_CLK_DIS_SHIFT)
                              | (s->clk_div << COM_ADC_CLK_DIV_SHIFT)
                              | (s->clk_rate << COM_ADC_CLK_SET_SHIFT)
                              | (s->clk_filter << COM_ADC_CLK_FILT_SHIFT)
                );

        // On va écrire les données
        ret = OS_spi_write_read(i_device, data, COM_CLOCK_LENGTH + 1);
    }

    return ret;
}
