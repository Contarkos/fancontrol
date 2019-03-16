/* Definition des fonctions basiques pour les MODULES */

// Include globaux
#include <string.h>
#include <mutex>

// Include locaux
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
    // Init du pthread
    this->m_thread.loop = reinterpret_cast<loop_func> (&MODULE::init_module);
}

MODULE::~MODULE()
{
    ;
}

// Configuration du module
void MODULE::mod_config(const char mod_name[MAX_LENGTH_MOD_NAME], OS_mutex_t *m_main, OS_mutex_t *m_mod)
{
    // Copie du nom
    strncpy(this->name, mod_name, MAX_LENGTH_MOD_NAME);

    this->m_mod_init = m_mod;
    this->m_main_init = m_main;

    // Lock de MAIN jusqu'à la fin de l'init
    if (this->m_main_init && this->m_mod_init)
    {
        OS_lock_mutex(this->m_main_init);
        is_init = true;
    }
}

// Fonction principale du thread du module
void* MODULE::init_module(void* p_this)
{
    int ret = 0;

    // On caste l'argument si il existe
    if (p_this)
    {
        MODULE * p_module = reinterpret_cast<MODULE *> (p_this);

        // Lancement des démarrages spécifiques et de la boucle
        ret = p_module->wait_and_loop();

        if (ret != 0)
        {
            LOG_ERR("MODULE : erreur dans le wait_and_loop, ret = %d", ret);

            // Arret du module proprement
            p_module->stop_and_exit();
        }
    }
    else
    {
        LOG_ERR("MODULE : pas de pointeur vers l'instance");
        ret = -1;
    }

    return (void *) NULL;
}

int MODULE::wait_and_loop(void)
{
    int ret = 0;

    // Init des variables
    ret = this->start_module();

    if (0 == ret)
    {
        // Déblocage du MAIN
        OS_unlock_mutex(this->m_main_init);

        // Demande de blocage du thread pour attendre que MAIN rende la main
        OS_lock_mutex(this->m_mod_init);

        // Initialisation d'objets spécifiques (ex : connexion aux sockets)
        ret = this->init_after_wait();

        // Démarrage de la boucle
        this->isRunning = true;

        while (isRunning)
        {
            if (0 > this->exec_loop())
            {
                LOG_ERR("%s : Erreur de boucle. Arrêt en cours pour le thread", this->name);
                this->isRunning = false;
            }
        }
    }
    else
    {
        LOG_ERR("MODULE : Erreur lors du démarrage du module %s, ret = %d", this->name, ret);
        ret = this->stop_and_exit();
    }

    // On débloque le mutex pour que MAIN puisse s'arrêter correctement
    OS_unlock_mutex(this->m_mod_init);

    return ret;
}

void* MODULE::exit_module(void* p_this)
{
    int ret = 0;

    // On caste l'argument si il existe
    if (p_this)
    {
        MODULE * p_module = reinterpret_cast<MODULE *> (p_this);

        // Arret du module
        ret = p_module->stop_and_exit();

        if (ret != 0)
        {
            LOG_ERR("MODULE : erreur dans le stop_and_exit, ret = %d", ret);
        }

    }
    else
    {
        LOG_ERR("MODULE : pas de pointeur vers l'instance");
        ret = -1;
    }

    return (void *) NULL;
}

int MODULE::stop_and_exit(void)
{
    int ret = 0;

    // Arret specifique du module
    ret = this->stop_module();

    // Arret générique
    this->set_running(false);

    return ret;
}

// Accesseurs
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
