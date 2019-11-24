
// Global includes

// Local includes
#include "base.h"
#include "integ_log.h"
#include "com.h"
#include "os.h"

#include "com_adc.h"

/*********************************************************************/
/*                            Defines                                */
/*********************************************************************/

// Utilisation de la self calibration plutot que zero calibration
#define COM_ADC_SELF_CAL

#define COM_BYTE_SHIFT        8
#define COM_DATA_LENGTH       2
#define COM_SETUP_LENGTH      1
#define COM_CLOCK_LENGTH      1

/*********************************************************************/
/*                       Variables globales                          */
/*********************************************************************/

t_com_adc_setup com_device_0_setup =
{
    .pin_rst = COM_ADC_PIN_RST0,
    .clk_disable = COM_ADC_CLOCK_ON,
    .clk_div = COM_STATE_OFF,
    .clk_rate = COM_ADC_CLOCK_2MHZ4,
    .clk_filter = COM_ADC_CLOCK_FILT_25_60,
    .pair = COM_ADC_PAIR_0,
    .mode = COM_ADC_MODE_NORMAL,
    .gain = COM_ADC_GAIN_1,
    .bipolarity = COM_STATE_OFF,
    .buffer_mode = COM_STATE_OFF,
    .filter_sync = COM_STATE_OFF
};

t_com_adc_setup com_device_1_setup =
{
    .pin_rst = COM_ADC_PIN_RST1,
    .clk_disable = COM_ADC_CLOCK_ON,
    .clk_div = COM_STATE_OFF,
    .clk_rate = COM_ADC_CLOCK_2MHZ4,
    .clk_filter = COM_ADC_CLOCK_FILT_25_60,
    .pair = COM_ADC_PAIR_0,
    .mode = COM_ADC_MODE_NORMAL,
    .gain = COM_ADC_GAIN_1,
    .bipolarity = COM_STATE_OFF,
    .buffer_mode = COM_STATE_OFF,
    .filter_sync = COM_STATE_OFF
};

t_com_adc_setup com_spi_device_array[OS_SPI_DEVICE_NB] =
{
    {
        .pin_rst = COM_ADC_PIN_RST0,
        .clk_disable = COM_ADC_CLOCK_ON,
        .clk_div = COM_STATE_OFF,
        .clk_rate = COM_ADC_CLOCK_2MHZ4,
        .clk_filter = COM_ADC_CLOCK_FILT_25_60,
        .pair = COM_ADC_PAIR_0,
        .mode = COM_ADC_MODE_NORMAL,
        .gain = COM_ADC_GAIN_1,
        .bipolarity = COM_STATE_OFF,
        .buffer_mode = COM_STATE_OFF,
        .filter_sync = COM_STATE_OFF
    },
    {
        .pin_rst = COM_ADC_PIN_RST1,
        .clk_disable = COM_ADC_CLOCK_ON,
        .clk_div = COM_STATE_OFF,
        .clk_rate = COM_ADC_CLOCK_2MHZ4,
        .clk_filter = COM_ADC_CLOCK_FILT_25_60,
        .pair = COM_ADC_PAIR_0,
        .mode = COM_ADC_MODE_NORMAL,
        .gain = COM_ADC_GAIN_1,
        .bipolarity = COM_STATE_OFF,
        .buffer_mode = COM_STATE_OFF,
        .filter_sync = COM_STATE_OFF
    }
};

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

int COM_adc_init(t_os_spi_device i_device, t_com_adc_clock_rate i_rate)
{
    int ret = 0;
    t_uint32 pin_rst;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.clk_rate = i_rate;
            pin_rst = com_device_0_setup.pin_rst;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.clk_rate = i_rate;
            pin_rst = com_device_1_setup.pin_rst;
            break;
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        // Configuration du module SPI
        ret = OS_spi_open_port(i_device, SPI_MODE_3, COM_ADC_BITS_PER_WORD, COM_ADC_SPEED_4M9);

        if (ret < 0)
        {
            LOG_ERR("COM : erreur à l'ouverture du device SPI, ret = %d", ret);
        }
        else
        {
            // Reglage pin de reset en OUT
            ret += OS_set_gpio(pin_rst, OS_GPIO_FUNC_OUT);

            // Reset physique
            ret += COM_adc_reset_hard(i_device);

            if (ret < 0)
            {
               LOG_ERR("COM : erreur reset hard durant l'init de l'ADC, ret = %d", ret);
            }
            else
            {
               // Reset de l'ADC
               ret += COM_adc_reset_soft(i_device);

               // Configuration de la clock de l'ADC avec la vitesse demandée
               ret += COM_adc_set_clock_rate(i_device, i_rate);

               // #if 0 pour faire une selfcalibration, #if 1 pour faire une zero calibration
#ifdef COM_ADC_SELF_CAL
               // Setup et demarrage de la calibration
               ret += COM_adc_set_mode(i_device, COM_ADC_MODE_SELFCAL);
#else
               // Descente de la pin d'activation
               ret += OS_write_gpio(COM_ADC_PIN_ENB, 0);

               // Attente pour être certain que la pin est down
               OS_usleep(10);

               // Setup et demarrage de la calibration
               ret += COM_adc_set_mode(i_device, COM_ADC_MODE_ZEROCAL);
#endif
            } // Fin if pour reset hard
        } // Fin if ouverture module SPI
    } // Fin if selection module
    return ret;
}

// Reset physique via la pin RST du module
int COM_adc_reset_hard(t_os_spi_device i_device)
{
    int ret = 0;
    t_uint32 pin;

    // Verification du device a resetter
    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            pin = com_device_0_setup.pin_rst;
            break;
        case OS_SPI_DEVICE_1:
            pin = com_device_1_setup.pin_rst;
            break;
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device à reset inexistant, device = %d", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        LOG_INF1("COM : reset hard pour ADC");

        // Reset de la pin
        ret += OS_write_gpio(pin, 0);

        // Attente pour être sur que le device recoit la commande
        OS_usleep(10);

        // Remontee de la pin
        ret += OS_write_gpio(pin, 1);

        ret += COM_adc_read_setup(i_device, NULL);

        // On attend qu'un mot soit pret
        ret += com_adc_wait_ready(i_device);
    }

    return ret;
}

// Reset du module SPI, equivalent a un ON/OFF physique
int COM_adc_reset_soft(t_os_spi_device i_device)
{
    int ret = 0;
    t_uint8 a[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

    // Verification du device a resetter
    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
        case OS_SPI_DEVICE_1:
            break;
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device à reset inexistant, device = %d", i_device);
            ret = -1;
    }

    if (ret < 0)
    {
        // Ecriture HIGH sur 32 bits
        ret += OS_spi_write_read(i_device, a, 4);

        // On attend qu'un mot soit pret
        ret += com_adc_wait_ready(i_device);

        LOG_INF2("COM : reset = %x %x %x %x", a[0], a[1], a[2], a[3]);
    }

    return ret;
}

// Lecture du resultat de la conversion analogique->numerique
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
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    if (ret < 0)
    {
        // Ne rien faire
        ;
    }
    else
    {
        // Mise en forme du buffer pour lire le registre de comparaison
        data[0] = (t_uint8)  (
                (0 << COM_ADC_WRITE_SHIFT)                  // Ecriture dans le registre de com
                | (COM_ADC_REG_DATA << COM_ADC_REG_SHIFT)     // Selection du registre de données
                | (COM_ADC_RW_MASK)                           // Lecture des données
                | (i_pair << COM_ADC_CHAN_SHIFT)              // Selection de la paire a ecouter
                );

        // Vide pour ne pas bloquer la lecture
        data[1] = COM_ADC_NULL;
        data[2] = COM_ADC_NULL;
        LOG_INF3("COM : valeur requete = 0x%x", data[0]);

        // On attend qu'un mot soit pret
        ret = com_adc_wait_ready(i_device);

        if (ret < 0)
        {
            LOG_ERR("COM : AD7705, pas de données disponibles...");
        }
        else
        {
            // On va récupérer les données
            ret = OS_spi_write_read(i_device, data, COM_DATA_LENGTH + 1);
            LOG_INF3("COM : valeur result = 0x%x & 0x%x", data[1], data[2]);

            result = (t_uint16) ( (data[1] << COM_BYTE_SHIFT) | data[2]);
            LOG_INF3("COM : resultat ADC = %d", result);
        }
    }

    return result;
}

// Configuration du regitre FSYNC du module ADC (1 = arret des fonctions de conversions)
int COM_adc_set_filter_sync(t_os_spi_device i_device, t_com_state i_filter_sync)
{
    int ret = 0;

    switch (i_filter_sync)
    {
        case COM_STATE_ON:
        case COM_STATE_OFF:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.filter_sync = i_filter_sync;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.filter_sync = i_filter_sync;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : erreur sur la valeur de synchro filtre, i_filter_sync = %d", i_filter_sync);
            ret = -2;
            break;
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

    switch (i_buffer_mode)
    {
        case COM_STATE_ON:
        case COM_STATE_OFF:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.buffer_mode = i_buffer_mode;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.buffer_mode = i_buffer_mode;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : erreur pour la valeur du buffer mode, i_buffer_mode = %d", i_buffer_mode);
            ret = -2;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);
    }

    return ret;
}

// Reglage de la bipolarite de l'ADC (1 = bipolaire, 0 = monopolaire)
int COM_adc_set_bipolarity(t_os_spi_device i_device, t_com_state i_bipolarity)
{
    int ret = 0;

    switch (i_bipolarity)
    {
       case COM_STATE_ON:
       case COM_STATE_OFF:
          switch (i_device)
          {
             case OS_SPI_DEVICE_0:
                com_device_0_setup.bipolarity = i_bipolarity;
                break;
             case OS_SPI_DEVICE_1:
                com_device_1_setup.bipolarity = i_bipolarity;
                break;
             case OS_SPI_DEVICE_NB:
             default:
                LOG_ERR("COM : device inexistant, device = %d", i_device);
                ret = -2;
                break;
          }
          break;
       default:
          LOG_ERR("COM : mauvaise valeur de bipolarité");
          ret = -1;
          break;
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

    switch (i_gain)
    {
        case COM_ADC_GAIN_1:
        case COM_ADC_GAIN_2:
        case COM_ADC_GAIN_4:
        case COM_ADC_GAIN_8:
        case COM_ADC_GAIN_16:
        case COM_ADC_GAIN_32:
        case COM_ADC_GAIN_64:
        case COM_ADC_GAIN_128:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.gain = i_gain;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.gain = i_gain;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : erreur de valeur pour le gain de l'ADC, i_gain = %d", i_gain);
            ret = -2;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);
    }

    return ret;
}

// Configuration et realisation de la calibration de l'ADC
int COM_adc_set_mode(t_os_spi_device i_device, t_com_adc_mode i_mode)
{
    int ret = 0;

    switch (i_mode)
    {
        case COM_ADC_MODE_NORMAL:
        case COM_ADC_MODE_SELFCAL:
        case COM_ADC_MODE_ZEROCAL:
        case COM_ADC_MODE_FULLCAL:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.mode = i_mode;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.mode = i_mode;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : erreur de valeur pour mode de calibration, i_mode = %d", i_mode);
            ret = -2;
            break;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_setup(i_device);

        // On attend que le device ait fini sa calibration
        ret = com_adc_wait_ready(i_device);
    }

    // Retour à la normal (pas de calibration à chaque setup)
    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            com_device_0_setup.mode = COM_ADC_MODE_NORMAL;
            break;
        case OS_SPI_DEVICE_1:
            com_device_1_setup.mode = COM_ADC_MODE_NORMAL;
            break;
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    return ret;
}

// Configuration de l'activation/desactivation de la clock de l'ADC
// WARNING : le ON = 0 et le OFF = 1
int COM_adc_enable_clock(t_os_spi_device i_device, t_com_adc_clock i_clock)
{
    int ret = 0;

    switch (i_clock)
    {
        case COM_ADC_CLOCK_ON:
        case COM_ADC_CLOCK_OFF:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.clk_disable = i_clock;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.clk_disable = i_clock;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : valeur erronee pour i_clock, i_clock = %d", i_clock);
            ret = -2;
            break;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_clock(i_device);
    }

    return ret;
}

// Configuration de la fréquence d'utilisation de l'ADC (au demarrage elle doit etre de 2MHz4)
int COM_adc_set_clock_rate(t_os_spi_device i_device, t_com_adc_clock_rate i_rate)
{
    int ret = 0;

    switch (i_rate)
    {
        case COM_ADC_CLOCK_1MHZ:
        case COM_ADC_CLOCK_2MHZ4:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.clk_rate = i_rate;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.clk_rate = i_rate;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : valeur erronee pour la frequence de fonctionnement de l'ADC, i_rate = %d", i_rate);
            ret = -2;
            break;
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

    switch (i_div)
    {
        case COM_STATE_ON:
        case COM_STATE_OFF:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.clk_div = i_div;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.clk_div = i_div;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : erreur sur la valeur du diviseur de fréquence, i_div = %d", i_div);
            ret = -2;
            break;
    }

    // Reconfig de la clock
    if (0 == ret)
    {
        com_adc_config_clock(i_device);
    }

    return ret;
}

int COM_adc_set_clock_filter(t_os_spi_device i_device, t_com_adc_clock_filt i_filter)
{
    int ret = 0;

    switch (i_filter)
    {
        case COM_ADC_CLOCK_FILT_20_50:
        case COM_ADC_CLOCK_FILT_25_60:
        case COM_ADC_CLOCK_FILT_100_250:
        case COM_ADC_CLOCK_FILT_200_500:
            switch (i_device)
            {
                case OS_SPI_DEVICE_0:
                    com_device_0_setup.clk_filter = i_filter;
                    break;
                case OS_SPI_DEVICE_1:
                    com_device_1_setup.clk_filter = i_filter;
                    break;
                case OS_SPI_DEVICE_NB:
                default:
                    LOG_ERR("COM : device inexistant, device = %d", i_device);
                    ret = -1;
                    break;
            }
            break;
        default:
            LOG_ERR("COM : erreur sur la valeur du filtre pour ADC, i_filter = %d", i_filter);
            ret = -2;
            break;
    }

    // Reconfig du setup
    if (0 == ret)
    {
        com_adc_config_clock(i_device);
    }

    return ret;
}

// Lecture du setup dans le registre de l'ADC
int COM_adc_read_setup(t_os_spi_device i_device, t_uint8 *o_setup)
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
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        // Mise en forme du buffer pour écrire dans le registre de configuration
        data[0] = (t_uint8)  (   (0 << COM_ADC_WRITE_SHIFT)                     // Ecriture dans le registre de com
                               | (COM_ADC_REG_SETUP << COM_ADC_REG_SHIFT)       // Selection du registre de données
                               | (1 << COM_ADC_RW_SHIFT)                        // Lecture des données
                               | (s->pair << COM_ADC_CHAN_SHIFT));              // On reste sur la dernière paire demandée

        // Lecture de la config courante
        data[1] = COM_ADC_NULL;
        LOG_INF3("COM : valeur de la cmd = 0x%x", data[0]);

        // On va écrire les données
        ret = OS_spi_write_read(i_device, data, COM_SETUP_LENGTH + 1);

        // TODO enregistrement des données
        LOG_INF3("COM : valeur du setup = 0x%x", data[1]);

        if (o_setup)
        {
            *o_setup = data[1];
        }
    }

    return ret;
}

// Lecture de la configuration de la clock sur l'ADC
int COM_adc_read_clock(t_os_spi_device i_device, t_uint8 *o_clock)
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
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        // Mise en forme du buffer pour écrire dans le registre de configuration
        data[0] = (t_uint8)  (   (0 << COM_ADC_WRITE_SHIFT)                     // Ecriture dans le registre de com
                               | (COM_ADC_REG_CLOCK << COM_ADC_REG_SHIFT)       // Selection du registre de données
                               | (1 << COM_ADC_RW_SHIFT)                        // Lecture des données
                               | (s->pair << COM_ADC_CHAN_SHIFT));              // On reste sur la dernière paire demandée

        // Lecture de la config courante
        data[1] = COM_ADC_NULL;

        // On va écrire les données
        ret = OS_spi_write_read(i_device, data, COM_SETUP_LENGTH + 1);

        // TODO enregistrement des données
        LOG_INF3("COM : valeur de la clock = 0x%x", data[1]);

        if (o_clock)
        {
            *o_clock = data[1];
        }
    }

    return ret;
}

/*********************************************************************/
/*                       Fonctions locales                           */
/*********************************************************************/

// Ecriture de la confuguration en memoire dans le registre setup de l'ADC
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
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        // Mise en forme du buffer pour écrire dans le registre de configuration
        data[0] = (t_uint8)  (   (0 << COM_ADC_WRITE_SHIFT)                     // Ecriture dans le registre de com
                               | (COM_ADC_REG_SETUP << COM_ADC_REG_SHIFT)       // Selection du registre de données
                               | (0 << COM_ADC_RW_SHIFT)                        // Ecriture des données
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

// Ecriture de la configuration en memoire dans le registre clock de l'ADC
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
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    if (0 == ret)
    {
        // Mise en forme du buffer pour écrire dans le registre de configuration
        data[0] = (t_uint8)  (   (0 << COM_ADC_WRITE_SHIFT)                     // Ecriture dans le registre de com
                               | (COM_ADC_REG_CLOCK << COM_ADC_REG_SHIFT)       // Selection du registre de données
                               | (0 << COM_ADC_RW_SHIFT)                        // Ecriture des données
                               | (s->pair << COM_ADC_CHAN_SHIFT));              // On reste sur la dernière paire demandée

        // Ecriture de la config courante
        data[1] = (t_uint8) (   (s->clk_disable << COM_ADC_CLK_DIS_SHIFT)
                              | (s->clk_div     << COM_ADC_CLK_DIV_SHIFT)
                              | (s->clk_rate    << COM_ADC_CLK_SET_SHIFT)
                              | (s->clk_filter  << COM_ADC_CLK_FILT_SHIFT)
                );

        // On va écrire les données
        ret = OS_spi_write_read(i_device, data, COM_CLOCK_LENGTH + 1);
    }

    return ret;
}

// Attente que l'ADC soit pret
int com_adc_wait_ready(t_os_spi_device i_device)
{
    int ret = 0, cpt = COM_ADC_MAX_WAIT;
    t_uint32 p;

    switch (i_device)
    {
        case OS_SPI_DEVICE_0:
            p = COM_ADC_PIN_RDY0;
            break;
        case OS_SPI_DEVICE_1:
            p = COM_ADC_PIN_RDY1;
            break;
        case OS_SPI_DEVICE_NB:
        default:
            LOG_ERR("COM : device inexistant, device = %d", i_device);
            ret = -1;
    }

    if (ret < 0)
    {
        // Ne rien faire
        ;
    }
    else
    {
        // On attend qu'un mot soit pret
        while ( (COM_STATE_ON == OS_read_gpio(p)) && cpt )
        {
            // Decrement du compteur
            cpt--;

            // 10ms de pause
            LOG_INF3("TEMP : waiting for RDY pin to get low during reset, cpt = %d", cpt);
            OS_usleep(10000);
        }
    }

    return ret;
}
