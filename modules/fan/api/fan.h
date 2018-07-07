#pragma once

// Global includes
#include <mutex>

// Local includes
#include "base_typ.h"

// Defines
#define NB_INSTANCES_FAN 1

// API pour module FAN
int FAN_start(std::mutex *m);

int FAN_stop(void);
