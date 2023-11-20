#include "own_wifi.h"

static const char *TAG_WIFI = "wifi station";

static __uint8_t s_retry_num = 0;
static __uint8_t check_wifi_num = 0;


void selectWifi(){

    wifi_config_t wifi_config = {
        .sta = {
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    switch (check_wifi_num)
    {
    case 0:
        strcpy((char*)wifi_config.sta.ssid, HOUSE_ESP_WIFI_SSID);
        strcpy((char*)wifi_config.sta.password, HOUSE_ESP_WIFI_PASSWORD);
        break;
    case 1:
        strcpy((char*)wifi_config.sta.ssid, CEL_ESP_WIFI_SSID);
        strcpy((char*)wifi_config.sta.password, CEL_ESP_WIFI_PASSWORD);
        break;
    case 2:
        strcpy((char*)wifi_config.sta.ssid, HOME_ESP_WIFI_SSID);
        strcpy((char*)wifi_config.sta.password, HOME_ESP_WIFI_PASSWORD);
        break;
    default:
        break;
    }


    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start() ); 
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    EventGroupHandle_t * p_s_wifi_event_group = (EventGroupHandle_t*) arg;
    EventGroupHandle_t s_wifi_event_group = *(p_s_wifi_event_group);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
        } else if(check_wifi_num < 3){
            s_retry_num = 0;
            check_wifi_num++;
            esp_wifi_stop();        // Stops with this configuration
            selectWifi();           // Tries another one
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_WIFI,"connect to the AP fail");

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(EventGroupHandle_t s_wifi_event_group)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        &s_wifi_event_group,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        &s_wifi_event_group,
                                                        &instance_got_ip));

    // Config here, previously

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

    selectWifi();       // Try differents WiFis 
    
    // Start wifi here, previously

    ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_WIFI, "connected to ap");
    } else if (bits & WIFI_FAIL_BIT) {
        // Wifi Failed
        error_flags = 1;  // TODO do it properly
        ESP_LOGI(TAG_WIFI, "Failed to connect");
    } else {
        ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
    }
}

void wifi_conn(EventGroupHandle_t s_wifi_event_group)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG_WIFI, "ESP_WIFI_MODE_STA");
    wifi_init_sta(s_wifi_event_group);
}
