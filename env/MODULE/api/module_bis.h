#ifndef MODULE_BIS_H_
#define MODULE_BIS_H_

/* Includes globaux */
#include <string.h>

/* Includes locaux */
#include "base_typ.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************/
/*                           Defines                                 */
/*********************************************************************/

#define MOD_MAX_LENGTH_NAME     64

/*********************************************************************/
/*                           Typedef                                 */
/*********************************************************************/

/* Forward declaration pour t_module_class */
typedef struct s_mod_context t_mod_context;

/* Fonction de base sans argument et retournant un entier de statut */
typedef int     (*base_func)    (void);

/* Function used by the module when the context is required */
typedef int     (*mod_func)     (t_mod_context *p_context);

/* Fonction utilisant la structure d'attributs pour fonctionner */
typedef void*   (*entry_func)   (void *p_context);

struct s_mod_context
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
    mod_func        wait_and_loop;
    mod_func        stop_and_exit;

    /* Fonctions specifiques a chaque module */
    base_func       start_module;
    base_func       stop_module;
    base_func       init_after_wait;
    base_func       exec_loop;
};


/*********************************************************************/
/*                              API                                  */
/*********************************************************************/

void* MODULE_init       (void *p_context);
void* MODULE_exit       (void *p_context);

int MODULE_config       (t_mod_context *p_context,
                         const char name[MOD_MAX_LENGTH_NAME],
                         OS_mutex_t *p_mutex_main, OS_mutex_t *p_mutex_mod);

int MODULE_wait_and_loop(t_mod_context *p_context);
int MODULE_stop_and_exit(t_mod_context *p_context);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_BIS_H_ */

