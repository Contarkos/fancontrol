#ifndef MAIN_FACTORY_H_
#define MAIN_FACTORY_H_

/* Contient les prototypes pour les fonctions de démarrage et d'arret des modules */

/* Defines */

/* Variables globales */
extern mod_type t_start[];

/* Fonctions */
int main_start_factory(void);

int main_loop(void);

int main_stop_factory(void);

#endif /* MAIN_FACTORY_H_ */
