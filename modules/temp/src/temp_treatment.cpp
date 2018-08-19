// Includes globaux
#include <stdio.h>
#include <unistd.h>

// Includes locaux
#include "base.h"
#include "os.h"
#include "com.h"
#include "com_msg.h"
#include "module.h"
#include "temp.h"
#include "temp_class.h"
#include "fan.h"

int TEMP::temp_treat_msg(t_com_msg i_msg)
{
    int ret = 0;

    switch (i_msg.id)
    {
        case MAIN_SHUTDOWN:
            ret = this->stop_and_exit();
            break;
        default:
            printf("[ER] FAN : mauvaise ID pour message, id = %d", i_msg.id);
            ret = 1;
    }

    return ret;
}
