#ifndef MODULE_BIS_H_
#define MODULE_BIS_H_

/* Includes globaux */

/* Includes locaux */
#include "base_typ.h"
#include "os.h"

/*********************************************************************/
/*                           Defines                                 */
/*********************************************************************/

#define MOD_MAX_LENGTH_NAME     64

/*********************************************************************/
/*                           Typedef                                 */
/*********************************************************************/

/* Forward declaration pour t_module_class */
typedef struct s_module_class t_module_class;

/* Fonction de base sans argument et retournant un entier de statut */
typedef int     (*base_func)    (void);

/* Fonction utilisant la structure d'attributs pour fonctionner */
typedef void*   (*entry_func)   (t_module_class *p_class);

struct s_module_class
{
    /* Attributs simples pour la classe */
    OS_thread_t     thread;
    OS_mutex_t      *mutex_mod;
    OS_mutex_t      *mutex_main;

    char            name[MOD_MAX_LENGTH_NAME];

    t_uint32        is_running;
    t_uint32        is_init;

    /* Fonctions de demarrage et d'arret du module */
    entry_func      init_module;
    entry_func      exit_module;

    /* Fonctions generiques pour le module */
    base_func       wait_and_loop;
    base_func       stop_and_exit;

    /* Fonctions specifiques a chaque module */
    base_func       start_module;
    base_func       stop_module;
    base_func       init_after_wait;
    base_func       exec_loop;
};


/*********************************************************************/
/*                              API                                  */
/*********************************************************************/

int MODULE_wait_and_loop(void);
int MODULE_stop_and_exit(void);

#endif /* MODULE_BIS_H_ */

