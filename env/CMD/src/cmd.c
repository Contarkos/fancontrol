// Module pour lire les commandes de l'utilisateur

// Global includes
#include <stdio.h>
#include <string.h>

// Local includes
#include "cmd.h"
#include "cmd_parse.h"

// Variables globales
int isRunning = 1;

int CMD_read(void)
{
   int ret = 0;
   char line[MAX_SIZE_CMD];

   while(isRunning)
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
      isRunning = 0;
   }

   return ret;
}
