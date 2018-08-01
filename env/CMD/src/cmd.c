// Module pour lire les commandes de l'utilisateur

// Global includes
#include <stdio.h>
#include <string.h>
#include <regex.h>

// Local includes
#include "main.h"
#include "cmd.h"
#include "cmd_parse.h"

// Variables globales

// Fonctions
int CMD_read(void)
{
   int ret = 0;
   char line[MAX_SIZE_CMD];

   while(main_is_running)
   {
      // Affichage du prompt
      printf("fan-app>");

      fgets(line, sizeof(line), stdin);

      cmd_parse_and_exec(line);
   }

   return ret;
}

int cmd_parse_and_exec(char line[MAX_SIZE_CMD])
{
   int ret = 0;

   printf("Commande recue : %s\n", line);

   if (0 == strncmp(line, "quit", 4))
   {
      main_is_running = 0;
   }

   return ret;
}

