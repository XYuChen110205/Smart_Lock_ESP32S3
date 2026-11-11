#ifndef __CLOUD_CONFIG_H__
#define __CLOUD_CONFIG_H__

// 复制本文件为 cloud_config.h 并填入你自己的配置（cloud_config.h 已被 .gitignore 忽略）

#define CLOUD_WIFI_SSID  "YOUR_WIFI_SSID"
#define CLOUD_WIFI_PASS  "YOUR_WIFI_PASSWORD"

#define CLOUD_MQTT_URI   "mqtt://bemfa.com:9501"
#define CLOUD_CLIENT_ID  "YOUR_BEMFA_PRIVATE_KEY"   // 巴法云私钥
#define CLOUD_TOPIC      "YOUR_TOPIC"               // 巴法云主题

#define CLOUD_MAX_RETRY  3

#endif
