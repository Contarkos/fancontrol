/* Basic function for MODULES */

/* Global includes */
#include <string.h>

/* Local includes */
#include "base.h"
#include "integ_log.h"
#include "os.h"
#include "module.h"

MODULE::MODULE(const char mod_name[MAX_LENGTH_MOD_NAME], OS_mutex_t *m_main, OS_mutex_t *m_mod) : MODULE()
{
    this->mod_config(mod_name, m_main, m_mod);
}

MODULE::MODULE()
{
    /* Pthread init */
    this->m_thread.loop = reinterpret_cast<loop_func> (&MODULE::init_module);
}

MODULE::~MODULE()
{
    ;
}

/* Module configuration */
void MODULE::mod_config(const char mod_name[MAX_LENGTH_MOD_NAME], OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    /* Copy the name of the module */
    strncpy(this->name, mod_name, BASE_MIN(MAX_LENGTH_MOD_NAME, strlen(this->name)));

    this->m_mod_init = m_mod;
    this->m_main_init = m_main;

    /* Lock of MAIN until the end of the init */
    if (this->m_main_init && this->m_mod_init)
    {
        OS_mutex_lock(this->m_main_init);
        is_init = true;
    }
}

/* Fonction principale du thread du module */
void* MODULE::init_module(void* p_this)
{
    int ret = 0;

    /* Casting the argument if is provided */
    if (p_this)
    {
        MODULE * p_module = reinterpret_cast<MODULE *> (p_this);

        /* Specific initialisation and main loop */
        ret = p_module->wait_and_loop();

        if (ret != 0)
        {
            LOG_ERR("MODULE : error in wait_and_loop, ret = %d", ret);

            /* Proper stop for the thread */
            p_module->stop_and_exit();
        }
    }
    else
    {
        LOG_ERR("MODULE : no pointer to the instance");
        ret = -1;
    }

    return (void *) NULL;
}

int MODULE::wait_and_loop(void)
{
    int ret = 0;

    /* Initialisation of the variables */
    ret = this->start_module();

    if (0 == ret)
    {
        /* Unlocking MAIN */
        OS_mutex_unlock(this->m_main_init);

        /* Locking thread waiting for MAIN to release the lock */
        OS_mutex_lock(this->m_mod_init);

        /* Initialisation of specific objects after INIT (ex : sockets' connection) */
        ret = this->init_after_wait();

        /* Starting loop */
        this->isRunning = true;

        while (isRunning)
        {
            if (0 > this->exec_loop())
            {
                LOG_ERR("%s : Loop error. Stopping thread", this->name);
                this->isRunning = false;
            }
        }
    }
    else
    {
        LOG_ERR("MODULE : Error while starting module %s, ret = %d", this->name, ret);
        ret = this->stop_and_exit();
    }

    /* Unlocking mutex for MAIN to stop correctly */
    OS_mutex_unlock(this->m_mod_init);

    return ret;
}

void* MODULE::exit_module(void* p_this)
{
    int ret = 0;

    /* Casting the argument if is provided */
    if (p_this)
    {
        MODULE * p_module = reinterpret_cast<MODULE *> (p_this);

        /* Stopping module */
        ret = p_module->stop_and_exit();

        if (ret != 0)
        {
            LOG_ERR("MODULE : error in stop_and_exit, ret = %d", ret);
        }

    }
    else
    {
        LOG_ERR("MODULE : no pointer to the instance");
        ret = -1;
    }

    return (void *) NULL;
}

int MODULE::stop_and_exit(void)
{
    int ret = 0;

    /* Specific stop for the module */
    ret = this->stop_module();

    /* Generic stop */
    this->set_running(false);

    return ret;
}

/* Accessors */
void MODULE::set_running(bool i_isRunning)
{
    if (is_init)
    {
        this->isRunning = i_isRunning;
    }
    else
    {
        this->isRunning = false;
    }
}

bool MODULE::is_running(void)
{
    return this->isRunning;
}
