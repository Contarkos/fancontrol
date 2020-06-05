#ifndef TEMP_MODULE_H_
#define TEMP_MODULE_H_

/* Global includes */
#include <poll.h>

/* Local includes */

/*********************************************************************/
/*                           Defines                                 */
/*********************************************************************/

#define TEMP_MODULE_NAME        "TEMP"
#define TEMP_PIN_IN             (COM_ADC_PIN_RDY0)       /* /DRDY ADC */
#define TEMP_PIN_OUT            (COM_ADC_PIN_ENB0)       /* Thermistor power control */
#define TEMP_DEFAULT_PREC       (1024)
#define TEMP_DEFAULT_CYCLE      (0.0F)
#define TEMP_TIMER_USEC         (500000)

#define TEMP_THERM_K_TEMP       (273.15F)               /* Conversion K/°C */
#define TEMP_THERM_DEF_TEMP     (298.15F)               /* Ambient temperature */
#define TEMP_THERM_COEFF        (3950)                  /* B coefficient B for Steinhart-Hart equation */
#define TEMP_THERM_DEFAULT      (10000)                 /* Resistance at 25°C for the resistor */
#define TEMP_THERM_COMP         (10000)                 /* Comparison resistor */
#define TEMP_VREF_ADC           (1.225F)                /* Voltage of reference for ADC powered in 3.3V */
#define TEMP_VDD_ADC            (3.3F)                  /* Voltage of output GPIO */

#define TEMP_POLL_TIMEOUT       (100)

typedef enum
{
    TEMP_FD_IRQ = 0,
    TEMP_FD_COM = 1,
    TEMP_FD_NB
} t_temp_fd_index;

typedef struct
{
    t_float32 temp;
    t_uint32 temp_valid;
} t_temp_value;

/*********************************************************************/
/*                        Global variables                           */
/*********************************************************************/
extern t_mod_context temp_modules[NB_INSTANCES_TEMP];

extern int temp_timer_id;
extern int temp_irq_fd;
extern int temp_sem_fd;

extern struct pollfd temp_poll_fd[TEMP_FD_NB];

extern t_temp_value temp_fan;
extern t_temp_value temp_room;

extern t_uint32 temp_adc_gain;

/*********************************************************************/
/*                      Internal functions                           */
/*********************************************************************/

/* Functions for handling the module */
int temp_start_module       (void);
int temp_stop_module        (void);

int temp_init_after_wait    (void);
int temp_exec_loop          (void);

/* Data handling */
int temp_treat_com          (void);
int temp_retrieve_data      (void);
int temp_send_data          (void);

/* Handling IRQ */
int temp_treat_irq          (void);
int temp_tic                (unsigned long i_tic);

#endif /* TEMP_MODULE_H_ */
