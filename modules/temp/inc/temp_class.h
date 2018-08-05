#pragma once

#define TEMP_MODULE_NAME         "TEMP"
#define TEMP_PIN_OUT             (18)
#define TEMP_PIN_IN              (25)
#define TEMP_DEFAULT_PREC        (1024)
#define TEMP_DEFAULT_CYCLE       (0.0F)
#define TEMP_TIMER_USEC          (500000)


class TEMP : public MODULE
{
    private:
        size_t timer_fd;
        int temp;

    public:
        TEMP(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m);
        ~TEMP();

        int start_module(void);
        int stop_module(void);

        int fan_getTemp(void) { return temp; }

        int exec_loop(void);

        // Récupération des données
        static void temp_timer_handler(size_t i_timer_id, void * i_data);
        int temp_retrieve_data(void);
};
