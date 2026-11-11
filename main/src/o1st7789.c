

#include "pins.h"
#include "o1st7789.h"

static const char *TAG = "example";

#define LCD_HZ     (10 * 1000 * 1000)
#define LCD_CMD_BITS           8         // 命令和参数
#define LCD_PARAM_BITS         8

esp_lcd_panel_handle_t panel_handle = NULL;//句柄？
 esp_lcd_panel_io_handle_t io_handle = NULL;

// 整屏显示缓存：240 * 240 * 2 = 115200 字节
uint8_t LCD_Buf[LCD_H_RES * LCD_V_RES * 2];



void lcd_init(void)
{
    ESP_LOGI(TAG, "Turn off LCD backlight");

    // RC522 由 rfid_app.c 初始化，这里不再让它待机

    //配置引脚与写入配置
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,//输出模式
        .pin_bit_mask = 1ULL << LCD_BLK
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));


    //初始化SPI
    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_SCLK,
        .mosi_io_num = LCD_MOSI,
        .miso_io_num = LCD_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        // .max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t),
        .max_transfer_sz = 240 * 240 * sizeof(uint16_t)+8,//直接刷完
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));


    //安装啥？这里不理解
    ESP_LOGI(TAG, "Install panel IO");
   
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC,
        .cs_gpio_num = LCD_CS,
        .pclk_hz = LCD_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,//这里要改但是不知道搞啥
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };


    //安装st7789的
    ESP_LOGI(TAG, "Install ST7789 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));//呃就是猜
    //没看到句柄定义在哪里呀


    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true)); // 颜色反了就改 false
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));



    
}


// 全屏填充一个 RGB565 颜色
void LCD_Fill(uint16_t color)
{
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);

    for (int i = 0; i < LCD_H_RES * LCD_V_RES * 2; i += 2) {
        LCD_Buf[i] = hi;
        LCD_Buf[i + 1] = lo;
    }

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0,
                                              LCD_H_RES, LCD_V_RES, LCD_Buf));
}

// 红 -> 绿 -> 蓝 -> 白 -> 黑 循环测试
void LCD_ColorTest(void)
{
    LCD_Fill(0xF800); // 红
    vTaskDelay(pdMS_TO_TICKS(1000));
    LCD_Fill(0x07E0); // 绿
    vTaskDelay(pdMS_TO_TICKS(1000));
    LCD_Fill(0x001F); // 蓝
    vTaskDelay(pdMS_TO_TICKS(1000));
    LCD_Fill(0xFFFF); // 白
    vTaskDelay(pdMS_TO_TICKS(1000));
    LCD_Fill(0x0000); // 黑
    vTaskDelay(pdMS_TO_TICKS(1000));
}


void lcd_test(void)
{
    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(LCD_BLK, LCD_BK_ON_LEVEL);

    // 循环播放颜色，方便观察（测试用）
    while (1) {
        LCD_ColorTest();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}