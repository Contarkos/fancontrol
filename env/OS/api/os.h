#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*loop_func)(void *);

typedef struct
{
    pthread_t thread;
    pthread_attr_t attr;
    loop_func loop;
} OS_thread_t;

typedef enum
{
    OS_RET_KO = 0,
    OS_RET_OK = 1
} os_ret_okko;

// Fonction pour les GPIO
typedef enum e_os_gpio_func
{
    OS_GPIO_FUNC_ALT0 = 0,
    OS_GPIO_FUNC_ALT1 = 1,
    OS_GPIO_FUNC_ALT2 = 2,
    OS_GPIO_FUNC_ALT3 = 3,
    OS_GPIO_FUNC_ALT4 = 4,
    OS_GPIO_FUNC_ALT5 = 5,
    OS_GPIO_FUNC_IN = 6,
    OS_GPIO_FUNC_OUT = 7
} t_os_gpio_func;

typedef enum
{
    OS_GPIO_LOW = 0,
    OS_GPIO_HIGH = 1
} t_os_gpio_state;

// Typedef pour les PWM
typedef enum
{
    OS_PWM_MODE_PWMMODE = 0,
    OS_PWM_MODE_MSMODE = 1
} os_pwm_mode;

typedef enum
{
    OS_CLOCK_SRC_GND = 0,
    OS_CLOCK_SRC_OSC = 1,
    OS_CLOCK_SRC_TST1 = 2,
    OS_CLOCK_SRC_TST2 = 3,
    OS_CLOCK_SRC_PLLA = 4,
    OS_CLOCK_SRC_PLLC = 5,
    OS_CLOCK_SRC_PLLD = 6,
    OS_CLOCK_SRC_HDMI = 7
} t_os_clock_source;

/*********************************************************************/
/*                         Fonctions API                             */
/*********************************************************************/

// Init des modules disponibles
int OS_init(void);
int OS_stop(void);

// Gestion du temps
int OS_create_timer(void);
int OS_start_timer(int i_timer_id);

void OS_usleep(int i_usec);

// Gestion des threads
int OS_create_thread(OS_thread_t * p_o_thread,
                     void * args);

int OS_joint_thread(OS_thread_t * p_i_thread, void **retval);
int OS_detach_thread(OS_thread_t * p_i_thread);

// Pour GPIO
int OS_set_gpio(t_uint32 i_pin, t_os_gpio_func i_inout);
int OS_write_gpio(t_uint32 i_pin, t_uint32 bool_active);

// Pour SPI
int OS_spiOpenPort (int spi_device);
int OS_spiClosePort (int spi_device);
int OS_spiWriteAndRead (int spi_device, unsigned char *data, int length);

// Pour PWM
int OS_pwn_enable(os_ret_okko i_enable);
int OS_pwm_set_frequency(t_uint32 i_freq);
int OS_pwm_set_dutycycle(float i_duty);
int OS_pwm_set_precision(t_uint32 i_prec);
int OS_pwm_set_mode(os_pwm_mode i_mode);

// Pour CLOCK
int OS_clock_set_source(t_os_clock_source i_source);
int OS_clock_set_freq(t_uint32 i_freq);

#ifdef __cplusplus
}
#endif

