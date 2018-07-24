#pragma once

#include "module.h"

#define FAN_MODULE_NAME "FAN"
#define FAN_PIN_OUT     17
#define FAN_PIN_IN      4

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
