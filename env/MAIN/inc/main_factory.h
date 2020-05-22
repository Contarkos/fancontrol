#ifndef MAIN_FACTORY_H_
#define MAIN_FACTORY_H_

/* Contient les prototypes pour les fonctions de démarrage et d'arret des modules */

/* Defines */
#define NB_MODULE 4

/* Variables globales */
extern mod_type t_start[NB_MODULE];

/* Fonctions */
int main_start_factory(void);
int main_init(void);

int main_loop(void);

int main_stop_factory(void);
int main_stop(void);

#endif /* MAIN_FACTORY_H_ */
