idf_component_register(SRCS "own_wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi nvs_flash main)
