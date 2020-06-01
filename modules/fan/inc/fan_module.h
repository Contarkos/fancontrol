#ifndef FAN_MODULE_H_
#define FAN_MODULE_H_

/* Global includes */
#include <poll.h>

/* Local includes */
#include "temp.h"

/*********************************************************************/
/*                           Defines                                 */
/*********************************************************************/

#define FAN_MODULE_NAME         "FAN"
#define FAN_PIN_PWM             (OS_GPIO_PWM_0)    /* Pin de sortie du signal PWM */
#define FAN_PIN_IN              (OS_GPIO_BCM_23)   /* Pin non utilisee car gestion interruption */
#define FAN_PIN_OUT             (OS_GPIO_BCM_24)   /* Activation/desactivation du relais de puissance */
#define FAN_DEFAULT_PREC        (1024)
#define FAN_DEFAULT_CYCLE       (0.0F)
#define FAN_TIMER_USEC          (500000)
#define FAN_POLL_TIMEOUT        1000

#define FAN_PWM_FREQ            (25000)
#define FAN_PWM_ECART           (0.3F)
#define FAN_ECART_MAX_TEMP      (10.0F)

#define FAN_HITS_PER_CYCLE      2
#define FAN_SEC_TO_MSEC         1000000

#define FAN_DUTY_MAX            100.0F
#define FAN_DUTY_VERY_HIGH      100.0F
#define FAN_DUTY_HIGH           80.0F
#define FAN_DUTY_MEDIUM         60.0F
#define FAN_DUTY_LOW            40.0F
#define FAN_DUTY_MIN            0.0F

#define FAN_TEMP_MAX            50.0F
#define FAN_TEMP_VERY_HIGH      45.0F
#define FAN_TEMP_HIGH           40.0F
#define FAN_TEMP_MEDIUM         35.0F
#define FAN_TEMP_LOW            30.0F
#define FAN_TEMP_MIN            10.0F

#define FAN_TEMP_INVALID        (t_int32)0xFFFFFFFF

typedef enum
{
    FAN_FD_IRQ = 0,
    FAN_FD_COM = 1,
    FAN_FD_NB
} t_fan_fd_index;

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
extern t_mod_context fan_modules[NB_INSTANCES_FAN];

extern int fan_timer_id;
extern int fan_irq_fd;
extern int fan_sem_fd;

extern struct pollfd fan_poll_fd[FAN_FD_NB];

extern fan_e_mode       fan_current_mode;
extern fan_e_power_mode fan_current_power_mode;

extern t_uint32         fan_consigne_speed;        /* Speed command */
extern int              fan_consigne_temp;         /* Temperature to reach */
extern float            fan_current_temp;          /* Temperature of the cooled system */
extern float            fan_room_temp;             /* Temperature of the room */

extern t_uint32         fan_current_speed;         /* Fan speed */

/*********************************************************************/
/*                      Internal functions                           */
/*********************************************************************/

/* Functions for handling the module */
int fan_start_module    (void);
int fan_stop_module     (void);

int fan_init_after_wait (void);
int fan_exec_loop       (void);

/* Specific functions */
int fan_set_pwm         (void);
int fan_set_power       (fan_e_power_mode i_mode);
int fan_compute_duty    (void);

/* Handling messages */
int fan_treat_com       (void);
int fan_update_mode     (t_fan_mode *i_data);
int fan_update_power    (t_fan_power_mode *i_data);
int fan_update_data     (t_temp_data *i_data);

/* Handling IRQ */
int fan_treat_irq       (int i_fd);

#endif /* FAN_MODULE_H_ */
