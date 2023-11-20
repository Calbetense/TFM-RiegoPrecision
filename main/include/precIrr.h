#pragma once

// Includes
#include <stdio.h>
#include "own_wifi.h"
#include "fsm.h"
#include "own_http.h"
//~
#include "driver/gpio.h"
#include <driver/adc.h>         /*ADC*/
#include <esp_adc_cal.h>
#include "driver/gptimer.h"


// Global variables
extern volatile __uint8_t url_flags;    // TODO do it properly
extern volatile __uint8_t error_flags;  // TODO do it properly
extern volatile __uint8_t timer_flag;      // ...

extern volatile float ET0;
extern esp_adc_cal_characteristics_t adc_cal;

// MACROS
#define GPIO_VALV           GPIO_NUM_4
#define GPIO_LED            GPIO_NUM_2

#define FLOW        8.0       // mm/h
#define FLOW_sec    FLOW/60.0 // mm/s

// Declarations

// Fire routine to getting ET0
void getET0();