#ifndef __CLOUD_H__
#define __CLOUD_H__

#include "esp_err.h"

// 后台任务里连 WiFi + 巴法云 MQTT（不阻塞开机）
void Cloud_Start(void);

// 上报一条消息（未连接会返回错误）
esp_err_t Cloud_Publish(const char *msg);

#endif
