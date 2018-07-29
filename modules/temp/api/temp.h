#pragma once

// Defines
#define NB_INSTANCES_TEMP 1

// API pour module TEMP
int TEMP_start(std::mutex *m);

int TEMP_stop(void);
