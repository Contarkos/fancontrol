#pragma once

// Definition des registres et shift pour ADC
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

#define COM_ADC_NULL            (0x00) // Pour pouvoir décaler les bits en cas de lecture
