#pragma once

// Includes globaux
#include <poll.h>

// Includes locaux
#include "com.h"

#define TEMP_MODULE_NAME        "TEMP"
#define TEMP_PIN_IN             (COM_ADC_PIN_RDY)       // /DRDY ADC
#define TEMP_PIN_OUT            (COM_ADC_PIN_ENB)                    // Controle de l'alim du thermistor
#define TEMP_DEFAULT_PREC       (1024)
#define TEMP_DEFAULT_CYCLE      (0.0F)
#define TEMP_TIMER_USEC         (500000)

#define TEMP_THERM_DEF_TEMP     (298.15F)               // Température ambiante
#define TEMP_THERM_COEFF        (3950)                  // Coefficient B de l'équation de Steinhart-Hart
#define TEMP_THERM_DEFAULT      (10000)                 // Resistance à 25°C de la résistance
#define TEMP_THERM_COMP         (10000)                 // Resistance de comparaison de tension

#define TEMP_POLL_TIMEOUT       (100)

typedef enum
{
    TEMP_FD_SOCKET = 0,
    TEMP_FD_NB = 1
} t_temp_fd_index;

class TEMP : public MODULE
{
    public:
        TEMP(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod);
        ~TEMP();

    private:
        struct pollfd p_fd[TEMP_FD_NB];
        int timer_fd = -1;
        int socket_fd = -1;
        int fan_fd = -1;
        float fan_temp;
        bool fan_temp_valid;
        float room_temp;
        bool room_temp_valid;

        /***********************************************/
        /*             Methodes virtuelles             */
        /***********************************************/
        int start_module(void);
        int stop_module(void);

        int init_after_wait(void);
        int exec_loop(void);

        /***********************************************/
        /*             Methodes spécifiques            */
        /***********************************************/
        static void temp_timer_handler(int i_timer_id, void * i_data);

        // Accesseurs
        float temp_getTemp(void)     { return fan_temp; }
        bool  temp_getValidity(void) { return fan_temp_valid; }

        // Gestion des données
        int temp_retrieve_data(void);
        int temp_send_data(void);
        int temp_treat_msg(t_com_msg i_msg);
};
