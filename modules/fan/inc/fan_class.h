#pragma once

#include "module.h"

class FAN : public MODULE
{
    private:
        int speed;

    public:
        FAN(char mod_name[MAX_LENGTH_MOD_NAME], std::mutex *m);
        ~FAN();

        int start_module(void);
        int stop_module(void);

        void fan_setSpeed(int s);
        int fan_getSpeed(void);

        int exec_loop(void);
};
