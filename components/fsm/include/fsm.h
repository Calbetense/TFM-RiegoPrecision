#pragma once

// Includes
#include <stdio.h>
//~
#include "precIrr.h"
//~
#include "driver/gptimer.h"



// Defines
#define MOISTURE_TH             1700
#define MOISTURE_FAIL_THRESHOLD 142

#define AVERAGE_NUMBER  3

// structus
typedef enum{
    INIT=0,
    CHECK,
    IRRIGATE,
    ERROR
} STATES;


// Declarations
    // fsm
void fsm_fire(gptimer_handle_t gptimer);

    // utils
__uint8_t  checkWifi();    
__uint8_t checkSensors();

void startTimer(gptimer_handle_t gptimer);   
__uint64_t checkMoisture();    // Do it in the interruption handle

void irrig();

gptimer_handle_t confTim();