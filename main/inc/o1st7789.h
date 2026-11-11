

#ifndef __O1ST7789_H__
#define __O1ST7789_H__


#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
// #include "lvgl.h"



void lcd_init(void);
void lcd_test(void);

void LCD_Fill(uint16_t color);
void LCD_ColorTest(void);

#endif
