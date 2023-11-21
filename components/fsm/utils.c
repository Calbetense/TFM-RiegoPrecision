#include "fsm.h"

// Calculate Kc 
float calcKc(){
    // Now first value of days passed is manually
    // But it must ask to user wich is the state of the crop

    // This will be implemented to pepper (also ask user wich crop)

    // Plantación
    if(nDays < 20)
        return 0.6;      // Days 0 - 20

    // Floración
    else if(21 < nDays && nDays < 30)
        return 0.71;  // Days 21 - 30
    else if(31 < nDays && nDays < 40)
        return 0.82;  // Days 31 - 40
    else if(41 < nDays && nDays < 50)
        return 0.93;  // Days 41 - 50
    else if(51 < nDays && nDays < 60)
        return 0.104; // Days 51 - 60

    // Fructificación
    else if(71 < nDays && nDays < 80)
        return 0.115; // Days 61 - 80

    // Cosecha
    else if(81 < nDays && nDays < 90)
        return 0.102; // Days 81 - 90
    else if(91 < nDays && nDays < 100)        
        return 0.88;  // Days 91 - 100
    else if(101 < nDays && nDays < 110)
        return 0.75;  // Days 101 - 110
    else
        return 0.6;
}

// Check if Wifi is connected
__uint8_t  checkWifi(){
    printf("Checking Wifi\n"); 
    if(error_flags) return 0; 
    printf("Wifi OK\n");
    return 1;
}

// Check if Sensors are well connected
__uint8_t checkSensors(){
    printf("Checking Moisture Sensor\n"); 

    // Read Sensor
    float value = (float)adc1_get_raw(ADC1_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(100));  
    value = (float)esp_adc_cal_raw_to_voltage(value, &adc_cal); 
    if(value <= MOISTURE_FAIL_THRESHOLD){
        error_flags = 1;                              // TODO
        printf("\rMoisture sensor disconected\n\n");
        return 0; 
    } 

    printf("Sensors OK\n");
    return 1;
} 

// Wait to take the measure
void startTimer(gptimer_handle_t gptimer)   // TODO Now just waits, but it must be done with interruptions
{
    printf("Started Timer\n");
    // Fire TIM
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    // TODO: Sleep processor and wake up with timer interruption
    // Wait until read
    while(true){
        if(timer_flag == 1){
            printf("\nTimes up!");
            timer_flag = 0;
            ESP_ERROR_CHECK(gptimer_stop(gptimer));
            break;
        }
        vTaskDelay(1);      // Watchdog prevent
    }
}

// Take a moisture measure
__uint64_t checkMoisture()    // Do it in the interruption handle
{
    // Read Sensor
    __uint64_t value = 0;

    // Discards the firs value and make an average of three readings
    adc1_get_raw(ADC1_CHANNEL_0);
    for(__uint8_t i = 0; i < AVERAGE_NUMBER; i++){
        value += (int)adc1_get_raw(ADC1_CHANNEL_0);
        vTaskDelay(50 / portTICK_PERIOD_MS);         
    }
    value /= AVERAGE_NUMBER;

    vTaskDelay(pdMS_TO_TICKS(50));  
    value = (__uint64_t)esp_adc_cal_raw_to_voltage(value, &adc_cal); 
    printf("\n    VALUE SOIL MEASURE: %lld, at %dh,%dm\n", value, hour, min);                    // TODO
    return value;
}


// Irrigate an especific time
void irrig(){
    static __uint64_t prevDay = 999;

    float t = 0;        // Calculated time to irrigate

    float ETc = ET0 * calcKc(); // Calculate ETc in mm/d
 
    // ETc (mm) = Caudal 8mm/h * tiempo de riego (s) -> caudal = 8mm/h*1h/60s = 0.1333333 mm/s
    // ETc = 8/60 *t
    t = ETc*60.0/8.0/*FLOW_sec*/;

    printf("\rET0 = %fmm/d, ETc = %fmm/d, time to irrigate = %fs\n\n", ET0, ETc, t);

    printf("Fire! ... Or better, Irrigate!!\nOpenning Valv\n");         // TODO
    // Open Valv
    gpio_set_level(GPIO_VALV, 1);   
    if(nDays != prevDay){
        prevDay = nDays;
        // Send data to Telegram Bot
        trigger_Telegram_POST(ETc); 
    }    

    vTaskDelay(t*1000 / portTICK_PERIOD_MS);         
    // Close Valv
    printf("Closing Valv\n");                                           // TODO
    gpio_set_level(GPIO_VALV, 0);    
}

