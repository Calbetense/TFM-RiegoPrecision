#include "precIrr.h"

// Global
volatile __uint8_t url_flags    = 0;    // TODO do it properly
volatile __uint8_t error_flags  = 0;    // TODO do it properly
volatile __uint8_t timer_flag = 0;      // ...


volatile float ET0;
esp_adc_cal_characteristics_t adc_cal;

// Fire routine to getting ET0
void getET0()
{
    get_http_url(); // First request to start rutine to get ET0
}

// GPIO
void confGPIO()
{
    gpio_config_t gpio_conf = {
        //disable interrupts
        .intr_type = GPIO_INTR_DISABLE,
        //set output mode
        .mode = GPIO_MODE_OUTPUT,
        //bit mask of the pins you want to set
        .pin_bit_mask = ( (1ULL << GPIO_VALV) | (1ULL << GPIO_LED) ),
        //disable pull up and pull down
        .pull_down_en = 0,
        .pull_up_en   = 0
    };
    gpio_config(&gpio_conf);
    gpio_set_level(GPIO_LED, 0);    // Start with no errors
    gpio_set_level(GPIO_VALV, 0);   // Start with closed valv
}

// ADC
void confADC()
{
    adc1_config_width(ADC_WIDTH_BIT_10);                        
    
    //MOISTURE init (ADC1, Channel 0) -> Written VN in the develop board
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);   // SOIL
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_10, 1100, &adc_cal);    
}


//~

// Timer Callback  
static bool alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    timer_flag = 1;
    return pdTRUE;
}

// Timer Setup
gptimer_handle_t confTim(){
    // TIMER
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 3000, // 3KHz
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // ALARM
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0, // counter will reload with 0 on alarm event
        .alarm_count = 3*1000/*1s*/ * 60 /*1min*/, // period = 1min
        .flags.auto_reload_on_alarm = true, // enable auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // Callback config
    gptimer_event_callbacks_t cbs = {
        .on_alarm = alarm_cb, // register user callback
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // Enable timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    return gptimer;
}



// Management of the FSM
static void fsm(void *pvParameters){

    // Timer configuration
    gptimer_handle_t gptimer = confTim();   // TODO: Do it in main function

    while(1){
        fsm_fire(gptimer);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
}

// FIRE!
void app_main(void)
{
     /* FreeRTOS event group to signal when we are connected*/
    static EventGroupHandle_t s_wifi_event_group;

    //printf("Started Program\n");

    //GPIO configuration
    confGPIO();

    // ADC configuration
    confADC();
    
    wifi_conn(s_wifi_event_group);
    http_init(s_wifi_event_group);

    xTaskCreate(&fsm, "fsm management", 4086, NULL, 5, NULL);    // Main thread used to manage the fsm

}

