/* Global includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module_bis.h"

#include "fan.h"
#include "fan_module.h"

/*********************************************************************/
/*                               Defines                             */
/*********************************************************************/

/*********************************************************************/
/*                          Global variables                         */
/*********************************************************************/
int fan_timer_id = -1;
int fan_irq_fd = -1;
int fan_sem_fd = -1;

struct pollfd fan_poll_fd[FAN_FD_NB] = 
{
    /* IRQ file descriptor */
    { .fd = -1, .events = POLLIN, },
    /* COM file descriptor */
    { .fd = -1, .events = POLLIN, }
};

/*********************************************************************/
/*                          Local variables                          */
/*********************************************************************/

static t_uint32 fan_msg_array[] =
{
    MAIN_START,
    MAIN_SHUTDOWN,
    FAN_MODE,
    FAN_POWER,
    FAN_TIMER,
    TEMP_DATA
};

static t_uint32 fan_msg_array_size = sizeof(fan_msg_array) / sizeof(fan_msg_array[0]);

/*********************************************************************/
/*                             Functions                             */
/*********************************************************************/

int fan_start_module (void)
{
    int ret = 0;

    LOG_INF1("FAN : Starting module class");

    /* Creating timer for the loop */
    if (0 == ret)
    {
        fan_timer_id = COM_create_timer_msg(FAN_TIMER_USEC, OS_TIMER_PERIODIC, FAN_TIMER);

        if (0 > fan_timer_id)
        {
            LOG_ERR("FAN : error while creating looping timer");
            ret = -1;
        }
    }

    /* Setting input diode to read speed */
    if (0 == ret)
    {
        ret = OS_set_gpio(FAN_PIN_IN, OS_GPIO_FUNC_IN);

        if (ret != 0)
            LOG_ERR("FAN : error setting GPIO input, ret = %d", ret);
    }

    /* Setting output diode to control fan power */
    if (0 == ret)
    {
        ret = OS_set_gpio(FAN_PIN_OUT, OS_GPIO_FUNC_OUT);

        if (ret != 0)
            LOG_ERR("FAN : error setting GPIO output, ret = %d", ret);
    }

    /* Setting output diode for PWM mode */
    if (0 == ret)
    {
        ret = OS_set_gpio(FAN_PIN_PWM, OS_GPIO_FUNC_ALT5);

        if (ret != 0)
            LOG_ERR("FAN : error setting GPIO PWM function, ret = %d", ret);
    }

    /* Configuration of the PWM Hardware */
    if (0 == ret)
    {
        ret = fan_set_pwm();

        if (ret)
            LOG_ERR("FAN : error while setting up PWM hardware, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Request interruption */
        fan_irq_fd = OS_irq_request(OS_IRQ_TIME_NAME, O_RDONLY);

        if (0 == fan_irq_fd)
        {
            LOG_ERR("FAN : error requesting interruption for reading fan speed");
            ret = -4;
        }
        else
        {
            LOG_INF3("FAN : request for IRQ OK, fd = %d", fan_irq_fd);
            fan_poll_fd[FAN_FD_IRQ].fd = fan_irq_fd;
        }
    }

    if (0 == ret)
    {
        /* Register the module */
        ret = COM_msg_register(COM_ID_FAN, &fan_sem_fd);

        if (0 != ret)
        {
            LOG_ERR("FAN : error while registering the module in COM");
            ret = -8;
        }
        else
        {
            fan_poll_fd[FAN_FD_COM].fd = fan_sem_fd;

            ret = COM_msg_subscribe_array(COM_ID_FAN, fan_msg_array, fan_msg_array_size);

            if (ret)
                LOG_ERR("FAN : error while subscribing to messages, ret = %d", ret);
        }
    }

    LOG_INF3("FAN : end of module init (ret = %d)", ret);
    return ret;
}

int fan_init_after_wait (void)
{
    int ret = 0;

    if (0 == ret)
    {
        /* Starting timer */
        ret = OS_start_timer(fan_timer_id);

        if (ret < 0)
            LOG_ERR("FAN : timer not started, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Enabling PWM... */
        ret = OS_pwn_enable(OS_STATE_ON);

        if (ret)
            LOG_ERR("FAN : error while configuring PWM, ret = %d", ret);
    }

    if (0 == ret)
    {
        /* Powering fan */
        ret = fan_set_power(FAN_POWER_MODE_ON);

        if (ret)
            LOG_ERR("FAN : error while activating fan, ret = %d", ret);
    }

    return ret;
}

int fan_exec_loop (void)
{
    int ret = 0, read_fd = 0, ii;

    if (0 == ret)
    {
        read_fd = poll(fan_poll_fd, FAN_FD_NB, FAN_POLL_TIMEOUT);

        if (read_fd <= 0)
        {
            /* Expired timeout */
            LOG_WNG("FAN : timeout poll expired");
            ret = 1;
        }
        else
        {
            for (ii = 0; ii < FAN_FD_NB; ii++)
            {
                if (POLLIN & fan_poll_fd[ii].revents)
                {
                    switch (ii)
                    {
                        case FAN_FD_IRQ:
                            ret = fan_treat_irq(fan_poll_fd[ii].fd);
                            break;
                        case FAN_FD_COM:
                            ret = fan_treat_com();
                            break;
                        case FAN_FD_NB:
                        default:
                            LOG_WNG("FAN : bad file descriptor, index = %d", ii);
                            ret = 2;
                            break;
                    }
                }
            }
        }
    }

    return ret;
}

int fan_stop_module (void)
{
    int ret = 0;

    /* Shutting down the fan */
    {
        ret = fan_set_power(FAN_POWER_MODE_OFF);

        if (ret)
            LOG_ERR("FAN : error while stopping fan, ret = %d", ret);
    }

    /* TODO : Stop PWM properly */
    {
        ret = OS_pwn_enable(OS_STATE_OFF);

        if (ret)
            LOG_ERR("FAN : error while stopping PWM, ret = %d", ret);
    }

    /* Stopping timer */
    {
        ret = OS_stop_timer(fan_timer_id);

        if (ret)
            LOG_ERR("FAN : error while stopping timer, ret = %d", ret);
    }

    /* Closing IRQ */
    {
        ret = OS_irq_close(fan_irq_fd);

        if (ret)
            LOG_ERR("FAN : error while closing IRQ, ret = %d", ret);
    }

    LOG_INF3("FAN : end of stop_module, ret = %d", ret);
    return ret;
}

