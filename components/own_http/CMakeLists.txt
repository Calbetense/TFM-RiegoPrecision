idf_component_register(SRCS "own_http.c" "utils.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client own_wifi main)
