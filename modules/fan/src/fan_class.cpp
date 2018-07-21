// Includes globaux
#include <stdio.h>
#include <unistd.h>

// Includes locaux
#include "os.h"
#include "module.h"
#include "fan_class.h"

/* Définition des constructeurs */
FAN::FAN(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m) : MODULE(mod_name, m)
{
    ;
}

FAN::~FAN()
{
    ;
}

int FAN::start_module()
{
    int ret = 0;

    printf("Démarrage de la classe du module\n");

    // Set de la diode en sortie
    OS_set_gpio(FAN_PIN_OUT, OS_GPIO_FUNC_OUT);

    return ret;
}

int FAN::stop_module()
{
    int ret = 0;

    // Arret de la boucle
    this->set_running(false);

    return ret;
}

int FAN::exec_loop()
{
    static int n = 0;
    static int is_onoff = 0;
    const int max = 100000;

    // Allumage de la diode
//    printf("FAN : écriture dans la pin %d = %d, result = %d\n", FAN_PIN_OUT, is_onoff, OS_write_gpio(FAN_PIN_OUT, is_onoff) );

    // Sleep pour ne pas aller trop vite
    usleep(20);
    is_onoff = 1 - is_onoff;

    // Condition de sortie
    if (n > max)
    {
        printf("FAN : fin du module\n");
        this->set_running(false);
    }
    else
    {
        n++;
    }

    return 0;
}
