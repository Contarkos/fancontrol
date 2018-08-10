#pragma once

#define TEMP_MODULE_NAME        "TEMP"
#define TEMP_PIN_OUT            (27)
#define TEMP_PIN_IN             (25)
#define TEMP_DEFAULT_PREC       (1024)
#define TEMP_DEFAULT_CYCLE      (0.0F)
#define TEMP_TIMER_USEC         (500000)

#define TEMP_THERM_DEF_TEMP     (298.15F) // Température ambiante
#define TEMP_THERM_COEFF        (3950)    // Coefficient B de l'équation de Steinhart-Hart
#define TEMP_THERM_DEFAULT      (10000)   // Resistance à 25°C de la résistance
#define TEMP_THERM_COMP         (10000)   // Resistance de comparaison de tension


class TEMP : public MODULE
{
    public:
        TEMP(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m);
        ~TEMP();

    private:
        size_t timer_fd;
        size_t socket_fd;
        float hotspot_temp;
        bool hotspot_temp_valid;

        /***********************************************/
        /*             Methodes virtuelles             */
        /***********************************************/
        int start_module(void);
        int stop_module(void);

        int exec_loop(void);

        /***********************************************/
        /*             Methodes spécifiques            */
        /***********************************************/
        static void temp_timer_handler(size_t i_timer_id, void * i_data);

        // Accesseurs
        float temp_getTemp(void)     { return hotspot_temp; }
        bool  temp_getValidity(void) { return hotspot_temp_valid; }

        // Gestion des données
        int temp_retrieve_data(void);
        int temp_send_data(void);
};
