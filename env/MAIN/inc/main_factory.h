#pragma once
// Contient les prototypes pour les fonctions de démarrage et d'arret des modules

// Includes locaux
#include "base_typ.h"
#include "module.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "os.h"

#ifdef __cplusplus
}
#endif

// Defines
#define NB_MODULE 1

// Variables globales
extern mod_type t_start[NB_MODULE];

// Fonctions
int main_start_factory(void);

int main_stop_factory(void);
