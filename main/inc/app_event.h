

#ifndef __APP_EVENT_H__
#define __APP_EVENT_H__

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum{
    EVT_KEY,
    EVT_FINGER_OK,EVT_FINGER_FAIL,
    EVT_FACE_OK,EVT_FACE_FAIL,
    EVT_RFID_OK,EVT_RFID_FAIL,
    EVT_VIBRATION,
    EVT_CLOUD_UNLOCK,
}app_event_type_t;

typedef struct {
    app_event_type_t type;
    char key;
}app_event_t;




void AppEvent_Init(void);
bool AppEvent_Send(const app_event_t *e);   // 队列满返回 false
bool AppEvent_SendKey(char key);            // 发按键事件
bool AppEvent_Post(app_event_type_t type);  // 只带类型的事件

void FSM_Task_Start(void);                  // 创建 fsm_task








#endif


