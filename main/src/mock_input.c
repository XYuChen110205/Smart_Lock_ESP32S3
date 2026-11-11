

#include "mock_input.h"
#include "app_fsm.h"
#include "app_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>


static void mock_task(void *arg)
{
    char line[32];
    while (1) {
        if (fgets(line, sizeof(line), stdin)) {
            for (int i = 0; line[i]; i++) {
                char c = line[i];
                if (c >= 'a' && c <= 'z') c -= 32;
                if ((c >= 'A' && c <= 'D') || (c >= '0' && c <= '9')) {
                    AppEvent_SendKey(c);
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void MockInput_Start(void)
{
    xTaskCreate(mock_task, "mock_in", 4096, NULL, 3, NULL);
}
