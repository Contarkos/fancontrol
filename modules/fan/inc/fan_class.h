#pragma once

#define FAN_MODULE_NAME         "FAN"
#define FAN_PIN_OUT             (18)
#define FAN_PIN_IN              (25)
#define FAN_DEFAULT_PREC        (1024)
#define FAN_DEFAULT_CYCLE       (0.0F)

#define FAN_PWM_FREQ            25000

class FAN : public MODULE
{
    private:
        int speed;

    public:
        FAN(const char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m);
        ~FAN();

        int start_module(void);
        int stop_module(void);

        void fan_setSpeed(int s);
        int fan_getSpeed(void) { return speed; }

        int exec_loop(void);
};

