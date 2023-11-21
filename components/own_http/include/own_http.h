#pragma once

#include <stdio.h>
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <esp_log.h>
#include "math.h"
//~
#include "own_wifi.h"
#include "precIrr.h"
#include "secrets.h"

// Defines
    // AEMET
#define API_URL  "https://opendata.aemet.es/opendata/api/prediccion/especifica/municipio/diaria/41091"
//#define API_KEY  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"       // Inside secrets.h
    // TELEGRAM
//#define TELEGRAM_ID "XXX:XXXXXX"                                                                          // Inside secrets.h
//#define CHAT_ID_DANI "XXXXXXXX"                                                                           // Inside secrets.h
#define URL_TELEGRAM "https://api.telegram.org/bot"TELEGRAM_ID"/sendMessage?chat_id="CHAT_ID_DANI"&text="


// Global Variables
extern volatile __uint64_t nDays;
extern volatile __uint8_t hour;
extern volatile __uint8_t min;

// Declarations
    // own_http
esp_err_t _http_event_handle(esp_http_client_event_t *evt);
esp_err_t _get_http_handle(esp_http_client_event_t *evt);
void trigger_http_request(const char *url);
void get_http_url();
void http_init(EventGroupHandle_t wifi_event_group);

    // utils
float __getR0();
void calcET0(int max, int min);
int isDigit(char c);
void getStatus(char* data, char* dest);
void getMax(char* data, char* dest);
void getMin(char* data, char* dest);
void getTemp(char* data, char* max, char* min);
void getDate(char* data);
void getET0();
// ~
void trigger_Telegram_POST(float ETc);