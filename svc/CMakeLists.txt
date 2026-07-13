idf_component_register(SRCS "svc_wifi.c" "svc_http.c" "svc_audio.c" "svc_display.c" "svc_storage.c" "svc_indicator.c" "test_svc.c"
                    INCLUDE_DIRS "."
                    REQUIRES bsp os esp_wifi esp_http_client nvs_flash lvgl__lvgl esp_driver_i2s esp_driver_ledc)
