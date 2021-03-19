#ifndef COM_ADS_H_
#define COM_ADS_H_

/*****************************************************************************/
/*                                  Defines                                  */
/*****************************************************************************/

/* Address for ADS1115 when ADDR plugged to GND */
#define COM_ADS1115_ADDRESS             0x48

/* Registers addresses */
#define COM_ADS_CONVERSION_REGISTER     0x00
#define COM_ADS_CONFIG_REGISTER         0x01
#define COM_ADS_LO_THRESH_REGISTER      0x02
#define COM_ADS_HI_THRESH_REGISTER      0x03

#define COM_ADS_CONVERSION_SIZE         2U
#define COM_ADS_CONFIG_SIZE             2U
#define COM_ADS_LO_THRESH_SIZE          2U
#define COM_ADS_HI_THRESH_SIZE          2U

#define COM_ADS_POINTER_SIZE            1U

/* Masks and shift for Conversion Register */

/* Masks and shift for Config Register */
#define COM_ADS_CONFIG_COMP_MASK        (0x0003)
#define COM_ADS_CONFIG_COMP_SHIFT       0U
#define COM_ADS_CONFIG_COMP_LATCH_MASK  (0x0004)
#define COM_ADS_CONFIG_COMP_LATCH_SHIFT 2U
#define COM_ADS_CONFIG_COMP_POL_MASK    (0x0008)
#define COM_ADS_CONFIG_COMP_POL_SHIFT   3U
#define COM_ADS_CONFIG_COMP_MODE_MASK   (0x0010)
#define COM_ADS_CONFIG_COMP_MODE_SHIFT  4U
#define COM_ADS_CONFIG_DR_MASK          (0x00E0)
#define COM_ADS_CONFIG_DR_SHIFT         5U
#define COM_ADS_CONFIG_MODE_MASK        (0x0100)
#define COM_ADS_CONFIG_MODE_SHIFT       8U
#define COM_ADS_CONFIG_PGA_MASK         (0x0E00)
#define COM_ADS_CONFIG_PGA_SHIFT        9U
#define COM_ADS_CONFIG_MUX_MASK         (0x7000)
#define COM_ADS_CONFIG_MUX_SHIFT        12U
#define COM_ADS_CONFIG_OS_MASK          (0x8000)
#define COM_ADS_CONFIG_OS_SHIFT         15U

typedef union
{
    t_uint16 u16;
    t_int16 i16;
    struct {
        t_uint8 lsb;
        t_uint8 msb;
    } u8;
} t_com_ads_data;

typedef union
{
    t_com_ads_data setup;
    struct
    {
        t_uint16 comp_queue:2;
        t_uint16 comp_latch:1;
        t_uint16 comp_pol:1;
        t_uint16 comp_mode:1;
        t_uint16 rate:3;
        t_uint16 mode:1;
        t_uint16 pga:3;
        t_uint16 mux:3;
        t_uint16 os:1;
    } bits;
} t_com_ads_config;

typedef union
{
    t_com_ads_data raw;
    struct
    {
        t_uint16 value:15;
        t_uint16 mode:1;
    } bits;
} t_com_ads_threshold;

/* Masks and shift for Lo_Thresh and Hi_Thresh registers */

#endif /* COM_ADS_H_ */
