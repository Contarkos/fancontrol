#pragma once

// Includes globaux
#include <poll.h>

// Includes locaux
#include "com.h"
#include "temp.h"

#define FAN_MODULE_NAME         "FAN"
#define FAN_PIN_PWM             (18)    // Pin de sortie du signal PWM
#define FAN_PIN_IN              (23)    // Pin non utilisee car gestion interruption
#define FAN_PIN_OUT             (24)    // Activation/desactivation du relais de puissance
#define FAN_DEFAULT_PREC        (1024)
#define FAN_DEFAULT_CYCLE       (0.0F)
#define FAN_TIMER_USEC          (40000)
#define FAN_POLL_TIMEOUT        1000

#define FAN_PWM_FREQ            (25000)
#define FAN_PWM_ECART           (0.3F)
#define FAN_ECART_MAX_TEMP      (10.0F)

#define FAN_HITS_PER_CYCLE      2
#define FAN_SEC_TO_MSEC         1000000

typedef enum
{
    FAN_FD_SOCKET = 0,
    FAN_FD_IRQ = 1,
    FAN_FD_NB
} t_fan_fd_index;

class FAN : public MODULE
{
    public:
        FAN(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m_main, std::mutex *m_mod);
        FAN();
        ~FAN();

    private:
        struct pollfd p_fd[FAN_FD_NB];  // Structure pour polling
        int timer_fd;                   // Index du timer requested
        int timeout_fd;                 // File descriptor donné au timer pour envoyer les messages de timeout
        int socket_fd;                  // File descriptor pour recevoir les messages
        int irq_fd;                     // File descriptor pour recevoir les interruptions

        fan_e_mode current_mode;
        fan_e_power_mode current_power_mode;

        int consigne_speed;     // Consigne de vitesse
        int consigne_temp;      // Température consigne à atteindre
        int current_temp;       // Température de l'élément à refroidir
        int room_temp;          // Température de la pièce

        int current_speed;      // Vitesse du ventilateur

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
        // Methodes de constructeurs
        void fan_init_pollfd();

        void fan_setConsSpeed(int s) { this->consigne_speed = s; }
        int  fan_getConsSpeed(void)  { return consigne_speed;    }

        void fan_setCurSpeed(int s)  { this->current_speed = s; }
        int  fan_getCurSpeed(void)   { return current_speed;    }

        // Algorithme de décision pour le dutycycle
        static void fan_timer_handler_old(int i_timer_id, void * i_data);
        static void fan_timer_handler(int i_timer_id, void * i_data);
        int fan_compute_duty(void);

        // Recuperation des données
        int fan_treat_msg(int i_fd);
        int fan_update_mode(t_fan_mode *i_data);
        int fan_update_power(t_fan_power_mode *i_data);
        int fan_update_data(t_temp_data *i_data);

        // Gestion IT
        int fan_treat_irq(int i_fd);
};

