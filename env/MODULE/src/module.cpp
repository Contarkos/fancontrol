/* Definition des fonctions basiques pour les MODULES */

// Include globaux
#include <string.h>

// Include locaux
#include "module.h"

MODULE::MODULE(char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m)
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

void* MODULE::init_module(void* p_this)
{
    int ret = 0;

    // On caste l'argument si il existe
    if (p_this)
    {
        MODULE * p_module = reinterpret_cast<MODULE *> (p_this);

        ret = p_module->init_and_wait();

        if (ret != 0)
        {
            printf("MODULE : erreur dans l'init_and_wait\n");
        }
    }
    else
    {
        ret = -1;
    }

    return (void *) NULL;
}


int MODULE::init_and_wait(void)
{
    int ret = 0;

    // Init des variables
    if (0 == this->start_module())
    {
        // Demande de blocage du thread pour attendre que MAIN rende la main
        this->m_init->lock();

        this->isRunning = true;

        // Démarrage de la boucle
        while (isRunning)
        {
            if (0 != exec_loop())
            {
                printf("[%s] Erreur de boucle. Arrêt en cours pour le thread\n", this->name);
                this->isRunning = false;
            }
        }

        // On débloque le mutex pour que MAIN puisse s'arrêter correctement
        this->m_init->unlock();
    }
    else
    {
        printf("Erreur lors du démarrage du module %s\n", this->name);
        ret = -1;
    }

    return ret;
}

int MODULE::stop_and_exit(void)
{
    return 0;
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
