#pragma once

// Defines
#define CMD_MAX_SIZE        128
#define CMD_NB_FD           1
#define CMD_POLL_TIMEOUT    100

#define CMD_REGEX_QUIT      "^quit$"
#define CMD_REGEX_MSG       "^s [a-zA-Z_]+"

// Fonctions locales
int cmd_parse_and_exec(char line[CMD_MAX_SIZE]);
