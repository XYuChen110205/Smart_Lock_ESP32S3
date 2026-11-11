idf_component_register(SRCS "main.c" "src/o1st7789.c" "src/lvgl_app.c" "src/ui.c" "src/app_fsm.c" "src/app_event.c" "src/mock_input.c" "src/lock_action.c" "src/servo.c" "src/buzzer.c" "src/keypad.c" "src/fpm383c.c" "src/fp_app.c" "src/vibration.c" "src/syn6288.c" "src/rfid_app.c" "src/cloud.c"
                    INCLUDE_DIRS "." "inc"
                    REQUIRES esp_lcd esp_driver_spi esp_driver_gpio esp_timer esp_driver_i2c espressif__esp_lvgl_port espressif__servo esp_driver_uart abobija__rc522 esp_wifi esp_netif mqtt nvs_flash esp_event )
