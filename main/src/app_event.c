

#include "app_event.h"


#include "app_fsm.h"
#include "lock_action.h"
#include "esp_log.h"

#define TAG "EVT"
#define EVT_QUEUE_LEN 16//队列长度

static QueueHandle_t s_evt_queue;
//队列句柄

void AppEvent_Init(void)
{
    //创建队列
    // TODO: s_evt_queue = xQueueCreate(EVT_QUEUE_LEN, sizeof(app_event_t));


    s_evt_queue = xQueueCreate(EVT_QUEUE_LEN, sizeof(app_event_t));
    if (s_evt_queue == NULL) {
        ESP_LOGE(TAG, "队列创建失败");
        

    }


}


//发送事件
bool AppEvent_Send(const app_event_t *e)
{
    // TODO: 队列为空返回 false；xQueueSend(s_evt_queue, e, 0) == pdTRUE
    if (s_evt_queue == NULL) 
    return false;

    //事件塞进队列，等待时间0
    if (xQueueSend(s_evt_queue, e, 0) == pdTRUE) {
        return true;  // 成功
    }
    return false;     // 队列满

}


//
bool AppEvent_SendKey(char key)
{
  

    app_event_t e = {
        .type = EVT_KEY,
        .key = key
    };
    
    // 调用发送函数
    return AppEvent_Send(&e);
}

bool AppEvent_Post(app_event_type_t type)
{
    app_event_t e = { .type = type };
    return AppEvent_Send(&e);
}


static void fsm_task(void *arg)
{
    app_event_t e;
    TickType_t last_unlock = 0;
    while (1) {
        //等待事件，没有则睡眠
        if (xQueueReceive(s_evt_queue, &e, portMAX_DELAY) == pdTRUE) {
            {
               switch (e.type) {
                case EVT_KEY:
                    // ESP_LOGI(TAG, "收到按键: %c", e.key);
                    FSM_HandleKey(e.key); 
                    break;

                case EVT_FINGER_OK:
                case EVT_RFID_OK:
                case EVT_FACE_OK:
                case EVT_CLOUD_UNLOCK: {
                    TickType_t now = xTaskGetTickCount();
                    if ((now - last_unlock) < pdMS_TO_TICKS(3000)) {
                        break;   // 3 秒冷却，防重复开锁
                    }
                    last_unlock = now;

                    if (e.type == EVT_FINGER_OK) {
                        FSM_FingerResult(true);
                    } else if (e.type == EVT_RFID_OK) {
                        FSM_RfidResult(true);
                    } else if (e.type == EVT_FACE_OK) {
                        FSM_FaceResult(true);
                    } else {
                        Lock_Open();
                        FSM_Init();
                    }
                    break;
                }

                case EVT_FINGER_FAIL:
                    FSM_FingerResult(false);
                    break;

                case EVT_RFID_FAIL:
                    FSM_RfidResult(false);
                    break;

                case EVT_FACE_FAIL:
                    FSM_FaceResult(false);
                    break;

                default:
                    ESP_LOGW(TAG, "收到未知事件类型: %d", e.type);
                    break;
               }
            }
    }
}
}

void FSM_Task_Start(void)
{
    // xTaskCreatePinnedToCore(fsm_task, "fsm", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(fsm_task, "fsm", 4096, NULL, 5, NULL, 1);
}









