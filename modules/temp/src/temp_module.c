/* Global includes */
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "com.h"
#include "module_bis.h"
#include "fan.h"

#include "temp.h"
#include "temp_module.h"

/*********************************************************************/
/*                               Defines                             */
/*********************************************************************/

/*********************************************************************/
/*                          Global variables                         */
/*********************************************************************/
int temp_timer_id = -1;
int temp_irq_fd = -1;
int temp_sem_fd = -1;

struct pollfd temp_poll_fd[TEMP_FD_NB] =
{
    /* IRQ file descriptor */
    { .fd = -1, .events = POLLIN, },
    /* COM file descriptor */
    { .fd = -1, .events = POLLIN, }
};

/*********************************************************************/
/*                          Local variables                          */
/*********************************************************************/

static t_uint32 temp_msg_array[] =
{
    MAIN_START,
    MAIN_SHUTDOWN,
    TEMP_TIMER,
    TEMP_TIC
};

static t_uint32 temp_msg_array_size = sizeof(temp_msg_array) / sizeof(temp_msg_array[0]);

/*********************************************************************/
/*                             Functions                             */
/*********************************************************************/

int temp_start_module()
{
    int ret = 0;

    LOG_INF1("TEMP : Starting module");

    /* Creating timer for the loop */
    if (0 == ret)
    {
        temp_timer_id = COM_create_timer_msg(TEMP_TIMER_USEC, OS_TIMER_PERIODIC, TEMP_TIMER);

        if (0 > temp_timer_id)
        {
            LOG_ERR("TEMP : error creating loop timer");
            ret = -1;
        }
    }

    if (0 == ret)
    {
        /* GPIO configuration */
        ret += OS_set_gpio(TEMP_PIN_OUT, OS_GPIO_FUNC_OUT);

        /* ADC initialisation */
        /* TODO : init at startup based on required device */
        ret += COM_adc_init(OS_SPI_DEVICE_0, COM_ADC_CLOCK_2MHZ4);

        /* ADC setup check */
        ret += COM_adc_read_setup(OS_SPI_DEVICE_0, NULL);
        ret += COM_adc_read_clock(OS_SPI_DEVICE_0, NULL);

        if ( ret < 0 )
            LOG_ERR("TEMP : no SPI device available");
    }

    if (0 == ret)
    {
        /* Interruption request */
        temp_irq_fd = OS_irq_request(OS_IRQ_ADC_NAME, O_RDONLY);

        if (0 == temp_irq_fd)
        {
            LOG_ERR("TEMP : error requesting interruption for SPI synchro");
            ret = -4;
        }
        else
        {
            LOG_INF3("TEMP : request for IRQ OK, fd = %d", temp_irq_fd);
            temp_poll_fd[TEMP_FD_IRQ].fd = temp_irq_fd;
        }
    }

    if (0 == ret)
    {
        /* Init ADS1115 on the I2C1 device @100kHz */
        ret = COM_ads_init(TEMP_I2C_MODULE, TEMP_I2C_CLOCK);

        if (0 != ret)
            LOG_ERR("TEMP: could not init I2C device, ret = %d", ret);
    }

    /* init of the module's queue */
    if (0 == ret)
    {
        ret = COM_msg_register(COM_ID_TEMP, &temp_sem_fd);

        if (0 != ret)
        {
            LOG_ERR("TEMP : error while registering the module in COM");
            ret = -8;
        }
        else
        {
            temp_poll_fd[TEMP_FD_COM].fd = temp_sem_fd;

            ret = COM_msg_subscribe_array(COM_ID_TEMP, temp_msg_array, temp_msg_array_size);
        }
    }

    LOG_INF3("TEMP : end of module init (ret = %d)", ret);
    return ret;
}

int temp_init_after_wait(void)
{
    int ret = 0;

    /* TODO : add the startup of the ADC */

    if (0 == ret)
    {
        /* Starting timer */
        ret = OS_start_timer(temp_timer_id);

        if (ret < 0)
            LOG_ERR("TEMP : error starting timer, ret = %d", ret);
    }

    return ret;
}

int temp_exec_loop()
{
    int ret = 0;
    int read_fd = 0, ii;

    /* Read on file descriptor */
    read_fd = poll(temp_poll_fd, TEMP_FD_NB, TEMP_POLL_TIMEOUT);

    /* Timeout test */
    if (read_fd <= 0)
    {
        /* Expired timeout */
        LOG_WNG("TEMP : timeout poll expired");
        ret = 1;
    }
    else
    {
        for (ii = 0; ii < TEMP_FD_NB; ii++)
        {
            if (POLLIN & temp_poll_fd[ii].revents)
            {
                switch (ii)
                {
                    case TEMP_FD_IRQ:
                        ret = temp_treat_irq();
                        break;
                    case TEMP_FD_COM:
                        ret = temp_treat_com();
                        break;
                    case TEMP_FD_NB:
                    default:
                        LOG_WNG("TEMP : error fd value, ii = %d", ii);
                        break;
                }
            }
        }
    }

    return ret;
}

int temp_stop_module()
{
    int ret = 0;

    /* Stopping timer */
    {
        ret = OS_stop_timer(temp_timer_id);

        if (ret)
            LOG_ERR("TEMP : error while stopping timer, ret = %d", ret);
    }

    /* Closing SPI device */
    {
        ret = OS_spi_close_port(OS_SPI_DEVICE_0);

        if (ret)
            LOG_ERR("TEMP : error while closing SPI device, ret = %d", ret);
    }

    /* Closing IRQ */
    {
        ret = OS_irq_close(temp_irq_fd);

        if (ret)
            LOG_ERR("TEMP : error while closing IRQ, ret = %d", ret);
    }

    LOG_INF3("TEMP : end of stop_module, ret = %d", ret);
    return ret;
}

