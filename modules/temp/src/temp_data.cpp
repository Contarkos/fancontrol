// Includes globaux
#include <stdio.h>
#include <math.h>

// Includes locaux
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "com_msg.h"
#include "module.h"
#include "temp.h"
#include "temp_class.h"

void TEMP::temp_timer_handler(int i_timer_id, void * i_data)
{
    TEMP *p_this = reinterpret_cast<TEMP *> (i_data);

    if (p_this && (p_this->timer_fd == i_timer_id))
    {
        p_this->temp_retrieve_data();
    }
}

int TEMP::temp_retrieve_data(void)
{
    int ret = 0;
    t_uint16 d;
    float r = 1;

    // Activation de la pin connectée au thermistor
    ret = OS_write_gpio(TEMP_PIN_OUT, 1);

    if (ret < 0)
    {
        LOG_ERR("TEMP : erreur activation GPIO pour lecture temp, ret = %d", ret);
    }
    else
    {
        // Lecture de la donnée dans le AD7705
        d = COM_adc_read_result(OS_SPI_DEVICE_0, COM_ADC_PAIR_0);

        COM_adc_read_clock(OS_SPI_DEVICE_0, NULL);

        // Désactivation de la pin connectée au thermistor
        ret = OS_write_gpio(TEMP_PIN_OUT, 0);

        if ( (0 == d) || (d == COM_ADC_MAXVALUE) )
        {
            LOG_WNG("TEMP : donnée invalide pour la température, value = %d", d);
            ret = 1;

            // Validité fausse pour la température
            this->fan_temp_valid = false;
        }
        else
        {
            // Calcul de la résistance équivalente
            r = d * ( (float)TEMP_THERM_COMP / (float) (COM_ADC_MAXVALUE - d));

            // Calcul de la température (formule de Steinhart-Hart)
            this->fan_temp = TEMP_THERM_COEFF / ( (TEMP_THERM_COEFF/TEMP_THERM_DEF_TEMP) + (float) (log(r) - log(TEMP_THERM_COMP)) );
            //this->fan_temp = 1 / ( (1/TEMP_THERM_DEF_TEMP) + (float) (log( r / TEMP_THERM_COMP ) / TEMP_THERM_COEFF) );

            // Validité de la température
            this->fan_temp_valid = true;
        }

        // Envoi de la donnée à FAN
        ret = temp_send_data();
    }

    return ret;
}

int TEMP::temp_send_data(void)
{
    int ret = 0;
    t_temp_data d;

    if (fan_fd < 0)
    {
        LOG_ERR("TEMP : pas de socket valide pour envoyer les données");
        ret = -1;
    }
    else
    {
        // On remplit la structure
        d.fan_temp = this->fan_temp;
        d.fan_temp_valid = this->fan_temp_valid ? TEMP_VALIDITY_VALID : TEMP_VALIDITY_INVALID;
        d.room_temp = this->room_temp;
        d.room_temp_valid = this->room_temp_valid ? TEMP_VALIDITY_VALID : TEMP_VALIDITY_INVALID;

        // On envoie le tout
        LOG_INF1("TEMP : envoi des données");
        ret = COM_send_data(this->fan_fd, TEMP_DATA, &d, sizeof(d), 0);

        if (ret < 0)
        {
            LOG_ERR("TEMP : erreur lors de l'envoi des données, ret = %d", ret);
        }
    }

    return ret;
}
