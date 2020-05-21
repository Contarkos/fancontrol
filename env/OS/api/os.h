#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <linux/spi/spidev.h>     /* Needed for SPI port */

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************/
/*                             Typedef                               */
/*********************************************************************/

typedef void * (*loop_func) (void *);
typedef void   (*timer_func)(int i_timer_id, void * i_data);

typedef enum
{
    OS_RET_KO = 0,
    OS_RET_OK = 1
} t_os_ret_okko;

typedef struct
{
    /* Actual thread structures */
    pthread_t thread;
    /* Attributes for the thread */
    pthread_attr_t attr;
    /* Starting function for this thread */
    loop_func loop;
} OS_thread_t;

typedef struct
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;
    t_os_ret_okko is_init;
} OS_mutex_t;

typedef struct
{
    sem_t semaphore;
    t_uint32 init_value;
    t_os_ret_okko is_init;
} OS_semaphore_t;

typedef struct
{
    int fd;
    t_uint32 init_value;
    t_os_ret_okko is_init;
} OS_semfd_t;

typedef enum
{
    OS_STATE_OFF = 0,
    OS_STATE_ON = 1
} t_os_state;

/* Functions for the GPIO */
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

/* Typedef for the PWM function */
typedef enum
{
    OS_PWM_DEVICE_0 = 0,
    OS_PWM_DEVICE_1 = 1,
    OS_PWM_DEVICE_NB
} t_os_pwm_device;

typedef enum
{
    OS_PWM_MODE_PWMMODE = 0,
    OS_PWM_MODE_MSMODE = 1
} os_pwm_mode;

typedef enum
{
    OS_PWM_MASH_FILTER_0 = 0,
    OS_PWM_MASH_FILTER_1 = 1,
    OS_PWM_MASH_FILTER_2 = 2,
    OS_PWM_MASH_FILTER_3 = 3
} os_mash_mode;

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

/* Typedef for I2C */
typedef enum
{
    OS_I2C_DEVICE_0 = 0,
    OS_I2C_DEVICE_1 = 1,
    OS_I2C_DEVICE_NB
} t_os_i2c_device;

/* Typedef for SPI */
typedef enum
{
    OS_SPI_DEVICE_0 = 0,
    OS_SPI_DEVICE_1 = 1,
    OS_SPI_DEVICE_NB
} t_os_spi_device;

typedef enum
{
    OS_SPI_MODE_0 = SPI_MODE_0,
    OS_SPI_MODE_1 = SPI_MODE_1,
    OS_SPI_MODE_2 = SPI_MODE_2,
    OS_SPI_MODE_3 = SPI_MODE_3
} t_os_spi_mode;

typedef enum
{
    OS_SPI_BITS_WORD_8 = 8,
    OS_SPI_BITS_WORD_9 = 9
} t_os_spi_bpw;

typedef enum
{
    OS_TIMER_SINGLE_SHOT = 0,
    OS_TIMER_PERIODIC
} t_os_timer_type;


/*********************************************************************/
/*                            Defines                                */
/*********************************************************************/

/* Initialisation of a struct OS_mutex_t */
#define OS_INIT_MUTEX          { PTHREAD_MUTEX_INITIALIZER, {{0}}, OS_RET_OK }

#define OS_MIN_PERCENT_PWM      0.0F
#define OS_MAX_PERCENT_PWM      100.0F

#define OS_IRQ_TIME_NAME        "/dev/gpio_ui0"
#define OS_IRQ_ADC_NAME         "/dev/gpio_ui1"

#define OS_MAX_SPI_SPEED        25000000U

#define OS_MAX_LENGTH_LONG      10

#define OS_MAX_LENGTH_FILENAME  256

/* Define for the pins of the GPIOs on a Rpi */
#define OS_GPIO_BCM_0           0U
#define OS_GPIO_BCM_1           1U
#define OS_GPIO_BCM_2           2U
#define OS_GPIO_BCM_3           3U
#define OS_GPIO_BCM_4           4U
#define OS_GPIO_BCM_5           5U
#define OS_GPIO_BCM_6           6U
#define OS_GPIO_BCM_7           7U
#define OS_GPIO_BCM_8           8U
#define OS_GPIO_BCM_9           9U
#define OS_GPIO_BCM_10          10U
#define OS_GPIO_BCM_11          11U
#define OS_GPIO_BCM_12          12U
#define OS_GPIO_BCM_13          13U
#define OS_GPIO_BCM_14          14U
#define OS_GPIO_BCM_15          15U
#define OS_GPIO_BCM_16          16U
#define OS_GPIO_BCM_17          17U
#define OS_GPIO_BCM_18          18U
#define OS_GPIO_BCM_19          19U
#define OS_GPIO_BCM_20          20U
#define OS_GPIO_BCM_21          21U
#define OS_GPIO_BCM_22          22U
#define OS_GPIO_BCM_23          23U
#define OS_GPIO_BCM_24          24U
#define OS_GPIO_BCM_25          25U
#define OS_GPIO_BCM_26          26U
#define OS_GPIO_BCM_27          27U

/* Define for specific pins */
#define OS_GPIO_PWM_0           OS_GPIO_BCM_18
#define OS_GPIO_SPI0_MISO       OS_GPIO_BCM_9
#define OS_GPIO_SPI0_MOSI       OS_GPIO_BCM_10
#define OS_GPIO_SPI0_SCLK       OS_GPIO_BCM_11
#define OS_GPIO_SPI0_CE0        OS_GPIO_BCM_8
#define OS_GPIO_SPI0_CE1        OS_GPIO_BCM_7

/*********************************************************************/
/*                         Functions API                             */
/*********************************************************************/

/* Init for the available modules */
int OS_init (void);
int OS_stop (void);

/* Time and timer handling */
int OS_create_timer (t_uint32 i_usec, timer_func i_handler, t_os_timer_type i_type, void * i_data);
int OS_create_timer_msg(t_uint32 i_usec, timer_func i_handler, t_os_timer_type i_type, t_uint32 i_id_msg);
int OS_start_timer (int i_timer_id);
int OS_stop_timer (int i_timer_id);

void OS_usleep (int i_usec);

int OS_gettime(t_int64 *o_time_sec, t_int64 *o_time_ns);

/* Threads handling */
int OS_create_thread (OS_thread_t * o_p_thread,
                      void * args);

int OS_joint_thread (OS_thread_t * i_p_thread, void **retval);
int OS_detach_thread (OS_thread_t * i_p_thread);

int OS_mutex_init(OS_mutex_t *i_mutex);
int OS_mutex_destroy(OS_mutex_t *i_mutex);
int OS_mutex_lock(OS_mutex_t *i_mutex);
int OS_mutex_unlock(OS_mutex_t *i_mutex);

int OS_signal_send(OS_thread_t *i_p_thread, int i_signal);

/* Semaphores handling */
int OS_sem_init         (OS_semaphore_t *i_sem, t_uint32 i_value);
int OS_sem_destroy      (OS_semaphore_t *i_sem);
int OS_sem_post         (OS_semaphore_t *i_sem);
int OS_sem_wait         (OS_semaphore_t *i_sem);
int OS_sem_trywait      (OS_semaphore_t *i_sem);

int OS_semfd_init       (OS_semfd_t *i_sem, t_uint32 i_value);
int OS_semfd_destroy    (OS_semfd_t *i_sem);
int OS_semfd_post       (OS_semfd_t *i_sem);
int OS_semfd_wait       (OS_semfd_t *i_sem);
int OS_semfd_trywait    (OS_semfd_t *i_sem);
int OS_semfd_timedwait  (OS_semfd_t *i_sem, int i_timeout);

/* About GPIO */
int OS_set_gpio (t_uint32 i_pin, t_os_gpio_func i_inout);
int OS_write_gpio (t_uint32 i_pin, t_uint32 bool_active);
int OS_read_gpio (t_uint32 i_pin);

/* About SPI */
int OS_spi_open_port (t_os_spi_device i_spi_id, unsigned char i_mode, unsigned char i_bits, unsigned int i_speed);
int OS_spi_close_port (t_os_spi_device i_spi_id);
int OS_spi_write_read (t_os_spi_device i_spi_id, unsigned char *data, int length);

int OS_spi_set_speed (t_os_spi_device i_spi_id, unsigned int i_speed);
int OS_spi_set_mode (t_os_spi_device i_spi_id, t_os_spi_mode i_mode);
int OS_spi_set_bits_per_word (t_os_spi_device i_spi_id, t_os_spi_bpw i_bpw);

/* About I2C */
int OS_i2c_open_device(t_os_i2c_device i_i2c_id, int i_address);
int OS_i2c_close_device(t_os_i2c_device i_i2c_id);

/* About IRQ */
int OS_irq_request (const char *i_irq, int i_flags);
int OS_irq_close (int i_fd);

/* About PWM */
int OS_pwn_enable (t_os_state i_enable);
int OS_pwm_set_clock_source (t_os_clock_source i_source);
int OS_pwm_set_frequency (t_uint32 i_freq);
int OS_pwm_set_dutycycle (float i_duty);
int OS_pwm_set_precision (t_uint32 i_prec);
int OS_pwm_set_mode (os_pwm_mode i_mode);
int OS_pwm_set_mash(os_mash_mode i_filter);

/* About CLOCK */
int OS_clock_set_source (t_os_clock_source i_source);
int OS_clock_set_freq (t_uint32 i_freq);
int OS_clock_set_mash(os_mash_mode i_filter);

#ifdef __cplusplus
}
#endif

