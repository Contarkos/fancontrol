/* Definition des fonctions basiques pour les MODULES */

// Include globaux
#include <string.h>

// Include locaux
#include "module.h"

MODULE::MODULE(char *mod_name, std::mutex *m)
{
    strncpy(this->name, mod_name, MAX_LENGTH_MOD_NAME);
    this->m_init = m;
}

MODULE::~MODULE()
{
    ;
}


int MODULE::init_and_wait(void)
{
    int ret = 0;

    // Init des variables
    if (0 != this->start_module())
    {
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
        printf("Erreur lors du démarrade du module %s\n", this->name);
        ret = -1;
    }

    return ret;
}

int MODULE::stop_and_exit(void)
{
    return 0;
}
