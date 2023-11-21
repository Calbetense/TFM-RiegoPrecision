#include "own_http.h"

// Constants and global
static const char* TAG_HTTP = "HTTP";
static EventGroupHandle_t s_wifi_event_group;

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    char maxTemp[3];   // String of max temperature
    char minTemp[3];   // String of min temperature

    switch(evt->event_id) { 
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_REDIRECT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_HEADER");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if(strstr((char*)evt->data, "temperatura")){
                    if(url_flags == 0){     // Do only for the first temperature
                        url_flags = 1;
                        getTemp( strstr((char*)evt->data, "temperatura"), maxTemp, minTemp );       // Extract Max and min temperatures
                        //printf("2   -   Month calculated %i\n", month);     // TODO
                        calcET0(atoi(maxTemp), atoi(minTemp));                                      // Calculate ET0 en mm/dia
                        printf("Temperatures:\n   Max: %d\n   Min: %d\n", atoi(maxTemp), atoi(minTemp) );

                    }
                }
                    else{
                        //printf("Nothing to do here\n");                     // Save all the msg in one string
                    }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

/*
    Get the actual URL of forecast and trigger it
*/
esp_err_t _get_http_handle(esp_http_client_event_t *evt){
    char status[3];
    char* prev;
    char* url;

    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_REDIRECT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_HEADER");
            if(strstr((char*)evt->header_value, "GMT"))
                getDate((char*)evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                getStatus(strstr((char*)evt->data, "estado"), status);
                if(atoi(status) == 200) {
                    prev = strstr((char*)evt->data, "https");
                    url = strtok(prev, "\"");    // Get actual AEMET URL forecast 
                    trigger_http_request(url);   // Get Request to AEMET
                }else{
                    error_flags = 1; // TODO do it properly
                    printf("Error on getting the porper URL\n\n");
                }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_HTTP, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

// Configure the request to get the actual AEMET's URL
void get_http_url()
{
    esp_http_client_config_t config = {
        .url = API_URL,
        .event_handler = _get_http_handle,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);    // Get URL sender
    esp_http_client_set_header(client, "Accept", "text/json");          // ADD HEADERS 
    esp_http_client_set_header(client, "api_key", API_KEY);             // ADD HEADERS

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        printf("HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG_HTTP, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

// Configure the request to get the forecast
void trigger_http_request(const char *url)
{
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handle,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        printf("HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG_HTTP, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

// Send Irrigation Data (L) to Telegram BOT
void trigger_Telegram_POST(float ETc){
    char ETchar[25];
    char url[sizeof(URL_TELEGRAM)+ 50] = URL_TELEGRAM; // Telegram Bot's URL

    sprintf(ETchar, "%.2f", ETc); // Transform float to char*
    strcat(url, "Hoy%20es%20necesario%20Regar%20");     // Append Text of MSG
    strcat(url, ETchar);                        // Append Data 
    strcat(url, "L");     // Append Units
    
    esp_http_client_config_t config = {
        .url = url,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        printf("HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG_HTTP, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

// Initialization of HTTP requests
void http_init(EventGroupHandle_t wifi_event_group){
    s_wifi_event_group = wifi_event_group;
}
