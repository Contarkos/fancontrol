#pragma once

// Defines
#define NB_INSTANCES_FAN 1

// API pour module FAN
int FAN_start(std::mutex *m);

int FAN_stop(void);
