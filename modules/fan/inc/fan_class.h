#pragma once

// Includes globaux
#include <poll.h>

// Includes locaux
#include "com.h"
#include "temp.h"

#define FAN_MODULE_NAME         "FAN"
#define FAN_PIN_PWM             (18)
#define FAN_PIN_IN              (25)
#define FAN_PIN_OUT             (23)
#define FAN_DEFAULT_PREC        (1024)
#define FAN_DEFAULT_CYCLE       (0.0F)
#define FAN_TIMER_USEC          (40000)
#define FAN_POLL_TIMEOUT        1000

#define FAN_PWM_FREQ            (25000)
#define FAN_PWM_ECART           (0.3F)
#define FAN_ECART_MAX_TEMP      (10.0F)

typedef enum
{
    FAN_FD_SOCKET = 0,
    FAN_FD_NB = 1
} t_fan_fd_index;

class FAN : public MODULE
{
    public:
        FAN(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod);
        ~FAN();

    private:
        struct pollfd p_fd[FAN_FD_NB];
        int timer_fd;
        int socket_fd;

        fan_e_mode current_mode;
        fan_e_power_mode current_power_mode;

        int consigne_speed;     // Consigne de vitesse
        int consigne_temp;      // Température consigne à atteindre
        int current_temp;       // Température de l'élément à refroidir
        int room_temp;          // Température de la pièce

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
        void fan_setSpeed(int s);
        int fan_getSpeed(void) { return consigne_speed; }

        // Algorithme de décision pour le dutycycle
        static void fan_timer_handler(int i_timer_id, void * i_data);
        int fan_compute_duty(void);

        // Recuperation des données
        int fan_treat_msg(t_com_msg i_msg, int i_size);
        int fan_update_mode(t_fan_mode *i_data);
        int fan_update_power(t_fan_power_mode *i_data);
        int fan_update_data(t_temp_data *i_data);
};

