#ifndef OS_RPI_H_
#define OS_RPI_H_

/* IO Access */
struct bcm2835_peripheral
{
    __off_t addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

/* Memory struct for GPIO */
extern struct bcm2835_peripheral os_periph_gpio;
/* Memory struct for PWM */
extern struct bcm2835_peripheral os_periph_pwm;
/* Memory struct for CLOCK */
extern struct bcm2835_peripheral os_periph_clock;
/* Memory struct for SPI */
extern struct bcm2835_peripheral os_periph_spi;

/* Memory regions for BCM2835 */
#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define PWM_BASE                (BCM2708_PERI_BASE + 0x20C000) /* PWM controller */
#define I2C0_BASE               (BCM2708_PERI_BASE + 0x205000) /* I2C0 controller */
#define I2C1_BASE               (BCM2708_PERI_BASE + 0x804000) /* I2C1 controller */
#define SPI0_BASE               (BCM2708_PERI_BASE + 0x204000) /* SPI0 controller */
#define CLOCK_BASE              (BCM2708_PERI_BASE + 0x101000) /* CLOCK controller */

#define BLOCK_SIZE              (4*1024)

/* In/Out/Alt control registers for GPIO */
#define INP_GPIO(g)        do { *( os_periph_gpio.addr + ((g)/10) ) &= (unsigned int) ~( 7 << ( ((g)%10)*3 ) ); } while (0)
#define OUT_GPIO(g)        do {\
                                INP_GPIO(g);\
                                *( os_periph_gpio.addr + ((g)/10) ) |= (unsigned int)  ( 1 << ( ((g)%10)*3 ) );\
                           } while (0) /* Toujours utiliser INP avant OUT */

#define SET_GPIO_ALT(g,a)  do {\
                                 INP_GPIO(g);\
                                 *(os_periph_gpio.addr + ((g)/10) ) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3));\
                           } while (0)

/* GPIO basic control macros */
#define GPIO_SET_REGISTER        *( os_periph_gpio.addr + 7  )
#define GPIO_SET(g)              do { GPIO_SET_REGISTER = (unsigned int) 1 << (g); } while (0) /* sets   bits which are 1 ignores bits which are 0 */
#define GPIO_CLR_REGISTER        *( os_periph_gpio.addr + 10 )
#define GPIO_CLR(g)              do { GPIO_CLR_REGISTER = (unsigned int) 1 << (g); } while (0) /* clears bits which are 1 ignores bits which are 0 */
 
#define GPIO_READ_REGISTER       *( os_periph_gpio.addr + 13 )
#define GPIO_READ(g)             ( (volatile int) (GPIO_READ_REGISTER & (t_uint32) (1 << (g))) >> (g) )

#define GPIO_MAX_NB              27U

/* SPI registers, masks and shifts */
typedef volatile struct
{
    t_uint32 cs;
    t_uint32 fifo;
    t_uint32 cdiv;
    t_uint32 dlen;
    t_uint32 loth;
    t_uint32 dc;
} t_os_spi_register;

#define SPI_CS_CHIPSELCT_MASK   ((t_uint32) 0x00000003)
#define SPI_CS_CHIPSELECT_SHIFT 0U
#define SPI_CS_CPHA_MASK        ((t_uint32) 0x00000004)
#define SPI_CS_CPHA_SHIFT       2U
#define SPI_CS_CPOL_MASK        ((t_uint32) 0x00000008)
#define SPI_CS_CPOL_SHIFT       3U
#define SPI_CS_CLEAR_MASK       ((t_uint32) 0x00000030)
#define SPI_CS_CLEAR_SHIFT      4U
#define SPI_CS_CSPOL_MASK       ((t_uint32) 0x00000040)
#define SPI_CS_CSPOL_SHIFT      6U
#define SPI_CS_TA_MASK          ((t_uint32) 0x00000080)
#define SPI_CS_TA_SHIFT         7U
#define SPI_CS_DMAEN_MASK       ((t_uint32) 0x00000100)
#define SPI_CS_DMAEN_SHIFT      8U
#define SPI_CS_INTD_MASK        ((t_uint32) 0x00000200)
#define SPI_CS_INTD_SHIFT       9U
#define SPI_CS_INTR_MASK        ((t_uint32) 0x00000400)
#define SPI_CS_INTR_SHIFT       10U
#define SPI_CS_ADCS_MASK        ((t_uint32) 0x00000800)
#define SPI_CS_ADCS_SHIFT       11U
#define SPI_CS_REN_MASK         ((t_uint32) 0x00001000)
#define SPI_CS_REN_SHIFT        12U
#define SPI_CS_LEN_MASK         ((t_uint32) 0x00002000)
#define SPI_CS_LEN_SHIFT        13U
#define SPI_CS_DONE_MASK        ((t_uint32) 0x00010000)
#define SPI_CS_DONE_SHIFT       16U
#define SPI_CS_RXD_MASK         ((t_uint32) 0x00020000)
#define SPI_CS_RXD_SHIFT        17U
#define SPI_CS_TXD_MASK         ((t_uint32) 0x00040000)
#define SPI_CS_TXD_SHIFT        18U
#define SPI_CS_RXR_MASK         ((t_uint32) 0x00080000)
#define SPI_CS_RXR_SHIFT        19U
#define SPI_CS_RXF_MASK         ((t_uint32) 0x00100000)
#define SPI_CS_RXF_SHIFT        20U
#define SPI_CS_CSPOL0_MASK      ((t_uint32) 0x00200000)
#define SPI_CS_CSPOL0_SHIFT     21U
#define SPI_CS_CSPOL1_MASK      ((t_uint32) 0x00400000)
#define SPI_CS_CSPOL1_SHIFT     22U
#define SPI_CS_CSPOL2_MASK      ((t_uint32) 0x00800000)
#define SPI_CS_CSPOL2_SHIFT     23U
#define SPI_CS_DMALEN_MASK      ((t_uint32) 0x01000000)
#define SPI_CS_DMALEN_SHIFT     24U
#define SPI_CS_LONG_MASK        ((t_uint32) 0x02000000)
#define SPI_CS_LONG_SHIFT       25U

#define SPI_CDIV_MASK           ((t_uint32) 0x0000FFFF)
#define SPI_CDIV_SHIFT          0U

#define SPI_LTOH_MASK           ((t_uint32) 0x00000007)
#define SPI_LTOH_SHIFT          0U

#define SPI_DC_TDREQ_MASK       ((t_uint32) 0x000000FF)
#define SPI_DC_TDREQ_SHIFT      0U
#define SPI_DC_TPANIC_MASK      ((t_uint32) 0x0000FF00)
#define SPI_DC_TPANIC_SHIFT     8U
#define SPI_DC_RDREQ_MASK       ((t_uint32) 0x00FF0000)
#define SPI_DC_RDREQ_SHIFT      16U
#define SPI_DC_RPANIC_MASK      ((t_uint32) 0xFF000000)
#define SPI_DC_RPANIC_SHIFT     24U

/* I2C registers, masks and shifts */
typedef volatile struct
{
    t_uint32 ctrl;
    t_uint32 status;
    t_uint32 dlen;
    t_uint32 addr;
    t_uint32 fifo;
    t_uint32 cdiv;
    t_uint32 delay;
    t_uint32 clkt;
} t_os_i2c_register;

#define I2C_CTL_RDWR_MASK        ((t_uint32) 0x00000001)
#define I2C_CTL_RDWR_SHIFT       0U
#define I2C_CTL_CLR_FIFO_MASK    ((t_uint32) 0x00000030)
#define I2C_CTL_CLR_FIFO_SHIFT   4U
#define I2C_CTL_START_MASK       ((t_uint32) 0x00000080)
#define I2C_CTL_START_SHIFT      7U
#define I2C_CTL_INTD_MASK        ((t_uint32) 0x00000100)
#define I2C_CTL_INTD_SHIFT       8U
#define I2C_CTL_INTT_MASK        ((t_uint32) 0x00000200)
#define I2C_CTL_INTT_SHIFT       9U
#define I2C_CTL_INTR_MASK        ((t_uint32) 0x00000400)
#define I2C_CTL_INTR_SHIFT       10U
#define I2C_CTL_ENABLE_MASK      ((t_uint32) 0x00008000)
#define I2C_CTL_ENABLE_SHIFT     15U

#define I2C_STA_TA_MASK          ((t_uint32) 0x00000001)
#define I2C_STA_TA_SHIFT         0U
#define I2C_STA_DONE_MASK        ((t_uint32) 0x00000002)
#define I2C_STA_DONE_SHIFT       1U
#define I2C_STA_TXW_MASK         ((t_uint32) 0x00000004)
#define I2C_STA_TXW_SHIFT        2U
#define I2C_STA_RXR_MASK         ((t_uint32) 0x00000008)
#define I2C_STA_RXR_SHIFT        3U
#define I2C_STA_TXD_MASK         ((t_uint32) 0x00000010)
#define I2C_STA_TXD_SHIFT        4U
#define I2C_STA_RXD_MASK         ((t_uint32) 0x00000020)
#define I2C_STA_RXD_SHIFT        5U
#define I2C_STA_TXE_MASK         ((t_uint32) 0x00000040)
#define I2C_STA_TXE_SHIFT        6U
#define I2C_STA_RXF_MASK         ((t_uint32) 0x00000080)
#define I2C_STA_RXF_SHIFT        7U
#define I2C_STA_ERR_MASK         ((t_uint32) 0x00000100)
#define I2C_STA_ERR_SHIFT        8U
#define I2C_STA_CLKT_MASK        ((t_uint32) 0x00000200)
#define I2C_STA_CLKT_SHIFT       9U

#define I2C_DLEN_VALUE_MASK      ((t_uint32) 0x0000FFFF)
#define I2C_DLEN_VALUE_SHIFT     0U

#define I2C_ADDR_VALUE_MASK      ((t_uint32) 0x0000007F)
#define I2C_ADDR_VALUE_SHIFT     0U

#define I2C_FIFO_VALUE_MASK      ((t_uint32) 0x000000FF)
#define I2C_FIFO_VALUE_SHIFT     0U

#define I2C_DIV_VALUE_MASK       ((t_uint32) 0x0000FFFF)
#define I2C_DIV_VALUE_SHIFT      0U

#define I2C_DEL_RISING_MASK      ((t_uint32) 0x0000FFFF)
#define I2C_DEL_RISING_SHIFT     0U
#define I2C_DEL_FALLING_MASK     ((t_uint32) 0xFFFF0000)
#define I2C_DEL_FALLING_SHIFT    16U

#define I2C_CLKT_TOUT_MASK       ((t_uint32) 0x0000FFFF)
#define I2C_CLKT_TOUT_SHIFT      0U

/* PWM registers, masks and shifts */
#define PWM_CTL_OFFSET           0U
#define PWM_CTL_REGISTER         *( os_periph_pwm.addr + PWM_CTL_OFFSET )
#define PWM_STA_OFFSET           1U
#define PWM_STA_REGISTER         *( os_periph_pwm.addr + PWM_STA_OFFSET )
#define PWM_RNG1_OFFSET          4U
#define PWM_RNG1_REGISTER        *( os_periph_pwm.addr + PWM_RNG1_OFFSET )
#define PWM_DAT1_OFFSET          5U
#define PWM_DAT1_REGISTER        *( os_periph_pwm.addr + PWM_DAT1_OFFSET )

#define PWM_CTL_PWEN1_MASK       ((t_uint32) 0x00000001)
#define PWM_CTL_PWEN1_SHIFT      0U
#define PWM_CTL_MODE1_MASK       ((t_uint32) 0x00000002)
#define PWM_CTL_MODE1_SHIFT      1U
#define PWM_CTL_RPTL1_MASK       ((t_uint32) 0x00000004)
#define PWM_CTL_RPTL1_SHIFT      2U
#define PWM_CTL_SBIT1_MASK       ((t_uint32) 0x00000008)
#define PWM_CTL_SBIT1_SHIFT      3U
#define PWM_CTL_POLA1_MASK       ((t_uint32) 0x00000010)
#define PWM_CTL_POLA1_SHIFT      4U
#define PWM_CTL_USEF1_MASK       ((t_uint32) 0x00000020)
#define PWM_CTL_USEF1_SHIFT      5U
#define PWM_CTL_CLRF1_MASK       ((t_uint32) 0x00000040)
#define PWM_CTL_CLRF1_SHIFT      6U
#define PWM_CTL_MSEN1_MASK       ((t_uint32) 0x00000080)
#define PWM_CTL_MSEN1_SHIFT      7U
#define PWM_CTL_PWEN2_MASK       ((t_uint32) 0x00000100)
#define PWM_CTL_PWEN2_SHIFT      8U
#define PWM_CTL_MODE2_MASK       ((t_uint32) 0x00000200)
#define PWM_CTL_MODE2_SHIFT      9U
#define PWM_CTL_RPTL2_MASK       ((t_uint32) 0x00000400)
#define PWM_CTL_RPTL2_SHIFT      10U
#define PWM_CTL_SBIT2_MASK       ((t_uint32) 0x00000800)
#define PWM_CTL_SBIT2_SHIFT      11U
#define PWM_CTL_POLA2_MASK       ((t_uint32) 0x00001000)
#define PWM_CTL_POLA2_SHIFT      12U
#define PWM_CTL_USEF2_MASK       ((t_uint32) 0x00002000)
#define PWM_CTL_USEF2_SHIFT      13U
#define PWM_CTL_MSEN2_MASK       ((t_uint32) 0x00008000)
#define PWM_CTL_MSEN2_SHIFT      15U

#define PWM_STA_FULL_MASK        ((t_uint32) 0x00000001)
#define PWM_STA_FULL_SHIFT       0U
#define PWM_STA_EMPT_MASK        ((t_uint32) 0x00000002)
#define PWM_STA_EMPT_SHIFT       1U
#define PWM_STA_WERR_MASK        ((t_uint32) 0x00000004)
#define PWM_STA_WERR_SHIFT       2U
#define PWM_STA_RERR_MASK        ((t_uint32) 0x00000008)
#define PWM_STA_RERR_SHIFT       3U
#define PWM_STA_GAPO1_MASK       ((t_uint32) 0x00000010)
#define PWM_STA_GAPO1_SHIFT      4U
#define PWM_STA_GAPO2_MASK       ((t_uint32) 0x00000020)
#define PWM_STA_GAPO2_SHIFT      5U
#define PWM_STA_GAPO3_MASK       ((t_uint32) 0x00000040)
#define PWM_STA_GAPO3_SHIFT      6U
#define PWM_STA_GAPO4_MASK       ((t_uint32) 0x00000080)
#define PWM_STA_GAPO4_SHIFT      7U
#define PWM_STA_BERR_MASK        ((t_uint32) 0x00000100)
#define PWM_STA_BERR_SHIFT       8U
#define PWM_STA_STAO1_MASK       ((t_uint32) 0x00000200)
#define PWM_STA_STAO1_SHIFT      9U
#define PWM_STA_STAO2_MASK       ((t_uint32) 0x00000400)
#define PWM_STA_STAO2_SHIFT      10U
#define PWM_STA_STAO3_MASK       ((t_uint32) 0x00000800)
#define PWM_STA_STAO3_SHIFT      11U
#define PWM_STA_STAO4_MASK       ((t_uint32) 0x00001000)
#define PWM_STA_STAO4_SHIFT      12U

/* Macros for CLOCK registers */
#define CLOCK_GP0_CTL_OFFSET    28
#define CLOCK_GP0_CTL_REGISTER  *( os_periph_clock.addr + CLOCK_GP0_CTL_OFFSET )
#define CLOCK_GP0_DIV_OFFSET    29
#define CLOCK_GP0_DIV_REGISTER  *( os_periph_clock.addr + CLOCK_GP0_DIV_OFFSET )
#define CLOCK_PWM_CTL_OFFSET    40
#define CLOCK_PWM_CTL_REGISTER  *( os_periph_clock.addr + CLOCK_PWM_CTL_OFFSET )
#define CLOCK_PWM_DIV_OFFSET    41
#define CLOCK_PWM_DIV_REGISTER  *( os_periph_clock.addr + CLOCK_PWM_DIV_OFFSET )

#define CLOCK_SRC_MASK          ((t_uint32) 0x0000000F)
#define CLOCK_SRC_SHIFT         0U
#define CLOCK_ENAB_MASK         ((t_uint32) 0x00000010)
#define CLOCK_ENAB_SHIFT        4U
#define CLOCK_BUSY_MASK         ((v_uint32) 0x00000080)
#define CLOCK_BUSY_SHIFT        5U
#define CLOCK_MASH_MASK         ((t_uint32) 0x00000600)
#define CLOCK_MASH_SHIFT        9U

#define CLOCK_DIVF_MASK         ((t_uint32) 0x00000FFF)
#define CLOCK_DIVF_SHIFT        0U
#define CLOCK_DIVI_MASK         ((t_uint32) 0x00FFF000)
#define CLOCK_DIVI_SHIFT        12U

#define CLOCK_PASSWD_MASK       ((t_uint32) 0x5a000000)
#define CLOCK_PASSWD_SHIFT      24U

#define CLOCK_MAX_DIVISOR       0xFFF
#define CLOCK_MAX_FREQ          19200000U

/* Variables d'initialisation */
extern t_os_ret_okko is_init_gpio;
extern t_os_ret_okko is_init_pwm;
extern t_os_ret_okko is_init_clock;
extern t_os_ret_okko is_init_spi;

/* Variables globales */
extern t_os_clock_source os_clock_source;
extern t_uint32 os_clock_max_freq[];

/* Init des GPIO */
int os_init_gpio(void);
int os_stop_gpio(void);

/* Init and stop for I2C */
int os_init_i2c(void);
int os_stop_i2c(void);

/* Init du PWM */
int os_init_pwm(void);
int os_stop_pwm(void);
int os_enable_pwm(t_os_state i_enable);

/* Init de la CLOCK */
int os_init_clock(void);
int os_stop_clock(void);

/* Init des timers */
int os_init_timer(void);
int os_end_timer(void);

/* Mapping des zones mémoires */
int os_map_peripheral(struct bcm2835_peripheral *p);
void os_unmap_peripheral(struct bcm2835_peripheral *p);

#endif /* OS_RPI_H_ */

