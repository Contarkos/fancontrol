#pragma once
// Contient les prototypes pour les fonctions de démarrage et d'arret des modules

// Defines
#define NB_MODULE 2

// Variables globales
extern mod_type t_start[NB_MODULE];

// Fonctions
int main_start_factory(void);
int main_init(void);

int main_stop_factory(void);
int main_stop();
