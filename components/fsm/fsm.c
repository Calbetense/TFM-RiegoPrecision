#include "fsm.h"

void fsm_fire(gptimer_handle_t gptimer)
{
    static STATES state = CHECK;        // TODO: Start on INIT, but to debug this is better
    static STATES next_state;

    printf("STATE: %d\n", state);

    switch (state)
   {
    case INIT:          // Check the well init of every periferic        
        if(checkWifi() && checkSensors())
            next_state = CHECK;
        else
            next_state = ERROR;
        break;
    case CHECK:                         // Wait timer and check if moisture is ready to be irrigated
        startTimer(gptimer);   
        if(checkMoisture() > MOISTURE_TH)  // TODO     
            next_state = CHECK;
        else
            next_state = IRRIGATE;
        break;
    case IRRIGATE :
        getET0();
        /*
            TODO: 
            Perfeccionar cálculo de ET0 

            Capturar precipitacion y restar a ET0
        */
        irrig();
        next_state = CHECK;
        break;

    case ERROR :
        printf("ERROR!!");
        gpio_set_level(GPIO_LED, 1);
        vTaskDelay(20000 / portTICK_PERIOD_MS);         // TODO : Better try to fix it
        break;
    
    default:
        next_state = INIT;
    }

    state = next_state;     
}
