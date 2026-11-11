

#include "keypad.h"

#include "pins.h"              // KEY_I2C_SDA / KEY_I2C_SCL
#include "app_fsm.h"           // FSM_HandleKey
#include "app_event.h"         // 事件队列
#include "syn6288.h"           // 语音播报按键
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>

/*
采用PCF8574扩展接键盘
I2C转8位的GPIO芯片
扫描通过：
每一把一行拉低，其他保持高
读8个脚，看哪一行被拉低就能知道哪一个按键被按下
连续几次读到同一个按键才算稳定（属于消抖操作）


*/

#define TAG "KEYPAD"

static i2c_master_bus_handle_t s_bus = NULL;//句柄，啥句柄啊
static i2c_master_dev_handle_t s_pcf = NULL;

//4x4按键映射
static const char s_keymap[16] =
{
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};


// 创建I2C主机总线
static esp_err_t keyboard_i2c_init(void)
{
   
    i2c_master_bus_config_t bus_config = {
        .i2c_port = -1,
        .sda_io_num = KEY_I2C_SDA,
        .scl_io_num = KEY_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    return i2c_new_master_bus(&bus_config, &s_bus);


}


// 2) 找 PCF8574 地址（0x20~0x27 / 0x38~0x3F）
static int keypad_find(void)
{
    for (uint8_t a = 0x20; a <= 0x27; a++) {
        if (i2c_master_probe(s_bus, a, 50) == ESP_OK) {
            ESP_LOGI(TAG, "found PCF8574 at 0x%02X", a);
            return a;
        }
    }
    for (uint8_t a = 0x38; a <= 0x3F; a++) {
        if (i2c_master_probe(s_bus, a, 50) == ESP_OK) {
            ESP_LOGI(TAG, "found PCF8574A at 0x%02X", a);
            return a;
        }
    }
    return -1;
}


//添加该地址为设备
static esp_err_t keypad_add(uint8_t addr)
{
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 100000,
    };
    return i2c_master_bus_add_device(s_bus, &cfg, &s_pcf);
}

// 4) 扫描一次：返回 0~15，无键 -1，I2C 错 -2
static int keypad_scan(int *drv, int *rdp)
{
    uint8_t rd;

    // 依次把 P0~P7 拉低，读其余 7 根，找到被按键连通的另一根
    for (int d = 0; d < 8; d++) {
        uint8_t out = (uint8_t)(0xFF & ~(1u << d));
        if (i2c_master_transmit_receive(s_pcf, &out, 1, &rd, 1, 50) != ESP_OK) {
            return -2;
        }
        for (int i = 0; i < 8; i++) {
            if (i == d) {
                continue;
            }
            if (!(rd & (1u << i))) {
                *drv = d;
                *rdp = i;
                return d * 8 + i;
            }
        }
    }
    return -1;
}

// 5) 任务：初始化 + 循环扫描 + 消抖 + 丢给 FSM
//是循环扫描而不是退出
// 把实际连通的“两根线号”翻译成字符（只映射能用的 10 个键）
static char pair_to_char(int a, int b)
{
    int lo = a < b ? a : b;
    int hi = a < b ? b : a;

    if (lo == 1 && hi == 5) return '1';  // S1
    if (lo == 2 && hi == 5) return '2';  // S2
    if (lo == 3 && hi == 5) return '3';  // S3
    if (lo == 4 && hi == 5) return 'C';  // S4 确认
    if (lo == 1 && hi == 6) return 'D';  // S5 删除
    if (lo == 2 && hi == 6) return '5';  // S6
    if (lo == 3 && hi == 6) return '6';  // S7
    if (lo == 1 && hi == 7) return '7';  // S9
    if (lo == 2 && hi == 7) return '8';  // S10
    if (lo == 3 && hi == 7) return '9';  // S11
    return 0;
}

static void task_keypad(void *arg)
{
    (void)arg;
    (void)s_keymap;   // 校准阶段暂时不用

    // 1) 初始化 I2C，失败就重试
    while (keyboard_i2c_init() != ESP_OK) {
        ESP_LOGW(TAG, "I2C bus init failed, retry...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // 2) 找 PCF8574 并添加，找不到就重试（不要退出）
    while (s_pcf == NULL) {
        int addr = keypad_find();
        if (addr < 0) {
            ESP_LOGW(TAG, "PCF8574 not found, retry...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        if (keypad_add((uint8_t)addr) != ESP_OK) {
            ESP_LOGW(TAG, "add device failed, retry...");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    ESP_LOGI(TAG, "keypad ready");

    // 3) 循环扫描 + 消抖
    int last = -1;
    int candidate = -1;
    int stable = 0;

    while (1) {
        int row = 0, col = 0;
        int key = keypad_scan(&row, &col);

        if (key == -2) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        if (key == candidate) {
            if (stable < 3) {
                stable++;
            }
        } else {
            candidate = key;
            stable = 1;
        }

        if (candidate >= 0 && stable >= 3 && candidate != last) {
            char ch = pair_to_char(row, col);
            ESP_LOGI(TAG, "key pair P%d-P%d -> %c", row, col, ch ? ch : '?');
            if (ch) {
                char say[2] = { ch, '\0' };
                Syn6288_PlayText(say);
                AppEvent_SendKey(ch);
            }
            last = candidate;
        }
        if (candidate < 0) {
            last = -1;
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

//任务创建，优先级4算是高中低
void Keypad_Start(void)
{
    xTaskCreate(task_keypad, "keypad", 4096, NULL, 4, NULL);
}

