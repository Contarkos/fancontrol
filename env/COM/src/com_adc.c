
// Global includes

// Local includes
#include "base.h"
#include "com.h"
#include "os.h"

#include "com_adc.h"

// Defines
#define COM_BYTE_SHIFT      8
#define COM_DATA_LENGTH     2

t_uint16 COM_adc_read_result(t_os_spi_device i_device, t_com_adc_pair i_pair)
{
    t_uint16 result = 0;
    t_uint8 data[COM_DATA_LENGTH + 1];
    int ret = 0;

    // Mise en forme du buffer pour lire le registre de comparaison
    data[0] = (t_uint8)  (  COM_ADC_WRITE_MASK                          // Ecriture dans le registre de com
                          | (COM_ADC_REG_DATA << COM_ADC_REG_SHIFT)     // Selection du registre de données
                          | COM_ADC_RW_MASK                             // Lecture des données
                          | (i_pair << COM_ADC_CHAN_SHIFT) );             // Selection de la paire a ecouter

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


int COM_adc_set_gain(t_os_spi_device i_device, t_uint8 i_gain)
{
    int ret = 0;

    return ret;
}

