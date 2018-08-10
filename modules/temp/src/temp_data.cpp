// Includes globaux
#include <stdio.h>
#include <math.h>

// Includes locaux
#include "base.h"
#include "os.h"
#include "com.h"
#include "module.h"
#include "temp_class.h"

void TEMP::temp_timer_handler(size_t i_timer_id, void * i_data)
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

    // Blablabla
    printf("[IS] TEMP : timer activé !\n");

    // Activation de la pin connectée au thermistor
    ret = OS_write_gpio(TEMP_PIN_OUT, 1);

    if (ret < 0)
    {
        printf("[ER] TEMP : erreur activation GPIO pour lecture temp, ret = %d\n", ret);
    }
    else
    {
        // Lecture de la donnée dans le AD7705
        d = COM_adc_read_result(OS_SPI_DEVICE_0, COM_ADC_PAIR_0);

        // Désactivation de la pin connectée au thermistor
        ret = OS_write_gpio(TEMP_PIN_OUT, 0);

        if ( (0 == d) || (d == COM_ADC_MAXVALUE) )
        {
            printf("[WG] TEMP : donnée invalide pour la température, value = %d\n", d);
            ret = 1;

            // Validité fausse pour la température
            this->hotspot_temp_valid = false;
        }
        else
        {
            // Calcul de la résistance équivalente
            r = d * ( (float)TEMP_THERM_COMP / (float) (COM_ADC_MAXVALUE - d));

            // Calcul de la température
            this->hotspot_temp = 1 / ( (1/TEMP_THERM_DEF_TEMP) + (float) (log( r / TEMP_THERM_COMP ) / TEMP_THERM_COEFF) );

            // Validité de la température
            this->hotspot_temp_valid = true;

            // Envoi de la donnée à FAN
            ret = temp_send_data();
        }
    }

    return ret;
}

int TEMP::temp_send_data(void)
{
    int ret = 0;

    return ret;
}
