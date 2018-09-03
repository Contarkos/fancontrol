#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <regex.h>

// Variables globales
extern regex_t reg_quit;
extern regex_t reg_msg;

// Fonctions API
int CMD_init(void);
int CMD_stop(void);

int CMD_read(void);

#ifdef __cplusplus
}
#endif

