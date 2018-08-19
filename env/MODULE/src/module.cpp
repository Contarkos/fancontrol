/* Definition des fonctions basiques pour les MODULES */

// Include globaux
#include <string.h>
#include <mutex>

// Include locaux
#include "base.h"
#include "os.h"
#include "module.h"

MODULE::MODULE(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m)
{
    // Copie du nom
    strncpy(this->name, mod_name, MAX_LENGTH_MOD_NAME);
    this->m_init = m;

    // Init du pthread
    this->m_thread.loop = reinterpret_cast<loop_func> (&MODULE::init_module);
}

MODULE::~MODULE()
{
    ;
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
            printf("[ER] MODULE : erreur dans le wait_and_loop, ret = %d\n", ret);

            // Arret du module proprement
            p_module->stop_and_exit();
        }
    }
    else
    {
        printf("[ER] MODULE : pas de pointeur vers l'instance\n");
        ret = -1;
    }

    return (void *) NULL;
}

int MODULE::wait_and_loop(void)
{
    int ret = 0;

    // Init des variables
    if (0 == this->start_module())
    {
        // Demande de blocage du thread pour attendre que MAIN rende la main
        this->m_init->lock();

        // Initialisation d'objets spécifiques (ex : connexion aux sockets)
        ret = this->init_after_wait();

        // Démarrage de la boucle
        this->isRunning = true;

        while (isRunning)
        {
            if (0 > this->exec_loop())
            {
                printf("[ER] %s : Erreur de boucle. Arrêt en cours pour le thread\n", this->name);
                this->isRunning = false;
            }
        }

        // On débloque le mutex pour que MAIN puisse s'arrêter correctement
        this->m_init->unlock();
    }
    else
    {
        printf("[ER] MODULE : Erreur lors du démarrage du module %s\n", this->name);
        ret = -1;
    }

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
            printf("[ER] MODULE : erreur dans le stop_and_exit, ret = %d\n", ret);
        }

    }
    else
    {
        printf("[ER] MODULE : pas de pointeur vers l'instance\n");
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
    this->isRunning = i_isRunning;
}

bool MODULE::is_running(void)
{
    return this->isRunning;
}
