# Smart Lock (ESP32-S3)

基于 ESP32-S3 的多模态智能门锁系统：支持密码、指纹、RFID 本地认证，具备震动检测、中文语音播报、蜂鸣器告警、舵机开锁，以及 WiFi + MQTT 远程开锁与状态上报。界面使用 ST7789 + LVGL。

## 功能

- 密码开锁（默认 `225566`）
- 指纹认证（FPM383C，UART）
- RFID 刷卡（MFRC522 / RC522，SPI）
- 震动防撬检测（SW-420）
- 中文语音（SYN6288，GB2312）
- 舵机开锁（SG90，LEDC PWM）、无源蜂鸣器提示
- WiFi + MQTT（巴法云）远程开锁与事件上报
- LVGL 九页面人机界面（菜单/密码/指纹/人脸/RFID/距离/开锁/提示/远程）

## 软件架构

```
键盘 / 指纹 / RFID / 云端
        │ 只发事件
        ▼
   事件队列 app_event
        ▼
   fsm_task（唯一改状态、唯一执行开锁）
        ▼
   UI(LVGL) / 舵机 / 蜂鸣器 / 语音 / 云上报
```

- 所有输入外设只发送事件，只有 `fsm_task` 处理，避免多任务并发竞争
- 开锁带 3 秒冷却，防重复触发
- 驱动层 / 逻辑层 / 界面层分层

## 引脚定义

| 模块 | 接口 | 引脚 |
| --- | --- | --- |
| ST7789 屏幕 | SPI2 | SCLK=12, MOSI=11, MISO=18, CS=13, DC=9, RST=10, BLK=14 |
| MFRC522 RFID | SPI2（共用） | CS=21, RST=17, MISO=18 |
| FPM383C 指纹 | UART1 57600 | TX=15, RX=16 |
| SYN6288 语音 | UART2 9600 | TX=6, RX=42 |
| PCF8574 键盘 | I2C | SDA=8, SCL=41 |
| SG90 舵机 | LEDC PWM 50Hz | GPIO7 |
| 无源蜂鸣器 | LEDC PWM | GPIO40 |
| SW-420 震动 | GPIO | GPIO39 |

详见 `main/inc/pins.h`。

## 目录结构

```
main/
  main.c             入口、初始化、任务启动
  app_fsm.c/.h       状态机
  app_event.c/.h     事件队列 + fsm_task
  ui.c/.h            页面
  lvgl_app.c/.h      LVGL 移植（esp_lvgl_port）
  o1st7789.c/.h      ST7789 驱动
  keypad.c/.h        PCF8574 键盘
  fpm383c.c/.h       FPM383C 协议驱动
  fp_app.c/.h        指纹任务
  rfid_app.c/.h      RC522 应用
  vibration.c/.h     震动检测
  syn6288.c/.h       语音
  tts_text.h         中文 GB2312 字模
  servo.c/.h         舵机
  buzzer.c/.h        蜂鸣器
  cloud.c/.h         云（WiFi + MQTT）
  cloud_config.example.h
  lock_action.c/.h   开锁流程
  pins.h             引脚定义
```

## 编译与烧录

需要 ESP-IDF v5.5（或兼容版本）。

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

组件依赖由 `main/idf_component.yml` 自动下载（LVGL、esp_lvgl_port、servo、rc522）。Flash 配置见 `sdkconfig.defaults`（16MB，自定义 4MB 应用分区）。

## 云端配置

1. 复制示例配置并填入自己的信息：
   ```bash
   cp main/inc/cloud_config.example.h main/inc/cloud_config.h
   ```
   填写 WiFi、巴法云私钥（`CLOUD_CLIENT_ID`）与主题（`CLOUD_TOPIC`）。
2. 在 `main/main.c` 中取消注释 `Cloud_Start();`。
3. 巴法云同一私钥同时只能有一个连接；设备与 PC 上位机不能共用同一 `client_id`。

## 按键映射

当前键盘接线只能识别左上 3×3 + 右上 1 个键，映射如下：

```
S1=1  S2=2  S3=3  S4=C(确认)
S5=D(删除)  S6=5  S7=6
S9=7  S10=8  S11=9
```

密码 `225566`：按 `S2 S2 S6 S6 S7 S7`，再按 `S4` 确认。

## 已知问题

- 屏幕背光正常但当前不显示内容（数据/控制线或排线接触问题）
- RC522 自检报 `FIFO length mismatch`（SPI MISO / 供电待排查）
- 指纹模块无心跳（接线 / 供电 / 波特率待排查）
- WiFi 启动时出现 brownout，云功能默认关闭；需稳定 5V/2A 电源
- 震动告警测试期间关闭，测完可在 `main.c` 恢复 `Vibration_Start()`
