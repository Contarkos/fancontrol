#pragma once

/* IO Access */
struct bcm2835_peripheral
{
    __off_t addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

/* Structure mémoire pour GPIO */
extern struct bcm2835_peripheral os_periph_gpio;
/* Structure mémoire pour PWM */
extern struct bcm2835_peripheral os_periph_pwm;
/* Structure mémoire pour CLOCK */
extern struct bcm2835_peripheral os_periph_clock;
/* Structure mémoire pour SPI */
extern struct bcm2835_peripheral os_periph_spi;

/* Adresse des zones de la mémoire pour BCM2835. A verifier pour 2708 */
#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define PWM_BASE                (BCM2708_PERI_BASE + 0x20C000) /* PWM controller */
#define SPI_BASE                (BCM2708_PERI_BASE + 0x214000) /* SPI controller */
#define CLOCK_BASE              (BCM2708_PERI_BASE + 0x101000) /* CLOCK controller */

#define BLOCK_SIZE              (4*1024)

/* Macros d'acces aux registres des GPIO */
#define INP_GPIO(g)        do { *( os_periph_gpio.addr + ((g)/10) ) &= (unsigned int) ~( 7 << ( ((g)%10)*3 ) ); } while (0)
#define OUT_GPIO(g)        do {\
                                INP_GPIO(g);\
                                *( os_periph_gpio.addr + ((g)/10) ) |= (unsigned int)  ( 1 << ( ((g)%10)*3 ) );\
                           } while (0) /* Toujours utiliser INP avant OUT */

#define SET_GPIO_ALT(g,a)  do {\
                                 INP_GPIO(g);\
                                 *(os_periph_gpio.addr + ((g)/10) ) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3));\
                           } while (0)

/* Macros de base GPIO pour modifier les valeurs */
#define GPIO_SET_REGISTER        *( os_periph_gpio.addr + 7  )
#define GPIO_SET(g)              do { GPIO_SET_REGISTER = (unsigned int) 1 << (g) } while (0) /* sets   bits which are 1 ignores bits which are 0 */
#define GPIO_CLR_REGISTER        *( os_periph_gpio.addr + 10 )
#define GPIO_CLR(g)              do { GPIO_CLR_REGISTER = (unsigned int) 1 << (g) } while (0) /* clears bits which are 1 ignores bits which are 0 */
 
#define GPIO_READ_REGISTER       *( os_periph_gpio.addr + 13 )
#define GPIO_READ(g)             ( (volatile int) (GPIO_READ_REGISTER & (t_uint32) (1 << (g))) >> (g) )

#define GPIO_MAX_NB              27U

/* Macros de base PWM pour modifier les paramètres */
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

/* Macros pour les registres de CLOCK */
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
