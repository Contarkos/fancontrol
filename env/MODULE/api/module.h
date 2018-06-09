#pragma once

#include <iostream>
#include <mutex>

// Macro

// Define variables
#define MAX_LENGTH_MOD_NAME     64

class MODULE {
    private:
        bool isRunning = false;
        std::mutex *m_init;

    protected:
        char name[MAX_LENGTH_MOD_NAME];

    public:
        MODULE(char *mod_name, std::mutex *m);
        virtual ~MODULE();

        // Fonction de démarrage spécifique au module
        virtual int start_module(void) = 0;
        virtual int stop_module(void) = 0;

        // Init utilisé par MAIN pour que chaque module attende son tour.
        int init_and_wait(void);
        int stop_and_exit(void);

        virtual int exec_loop(void) = 0;
};
