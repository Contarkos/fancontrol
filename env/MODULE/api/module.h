#pragma once

#include <iostream>
#include <mutex>

// Include locaux
#include "os.h"

// Macro

// Define variables
#define MAX_LENGTH_MOD_NAME     64

class MODULE {
    private:
        bool isRunning = false;
        std::mutex *m_init;

    protected:
        char name[MAX_LENGTH_MOD_NAME];
        OS_thread_t m_thread;

        void set_running(bool isRunning);

    public:
        MODULE(char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m);
        virtual ~MODULE();

        // Fonction de démarrage spécifique au module
        virtual int start_module(void) = 0;
        virtual int stop_module(void) = 0;

        // Init utilisé par MAIN pour que chaque module attende son tour.
        static void* init_module(void* p_this);

        int init_and_wait(void);
        int stop_and_exit(void);

        virtual int exec_loop(void) = 0;

        bool is_running(void);

        // Pour le thread
        OS_thread_t* MOD_getThread(void) { return &(this->m_thread); }
};
