#pragma once

#include <mutex>

// Macro

// Define variables
#define MAX_LENGTH_MOD_NAME     64

class MODULE {
    private:
        bool isRunning = false;
        bool is_init   = false;
        std::mutex *m_mod_init;
        std::mutex *m_main_init;

    protected:
        char name[MAX_LENGTH_MOD_NAME];
        OS_thread_t m_thread;

        MODULE(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod);
        void set_running(bool isRunning);

        // Fonction de démarrage spécifique au module
        virtual int start_module(void) = 0;
        virtual int stop_module(void) = 0;

        virtual int init_after_wait(void) = 0;
        virtual int exec_loop(void) = 0;

        // Démarrage et arret du module
        int wait_and_loop(void);
        int stop_and_exit(void);

    public:
        MODULE();
        virtual ~MODULE();

        // Init utilisé par MAIN pour que chaque module attende son tour.
        static void* init_module(void* p_this);
        static void* exit_module(void* p_this);

        // Config du module
        void mod_config(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod);

        bool is_running(void);

        // Pour le thread
        OS_thread_t* MOD_getThread(void) { return &(this->m_thread); }
};
