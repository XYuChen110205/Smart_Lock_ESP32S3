#include "rfid_app.h"
#include "pins.h"
#include "app_fsm.h"
#include "app_event.h"

#include "rc522.h"
#include "driver/rc522_spi.h"
#include "rc522_picc.h"
#include "esp_log.h"

#define TAG "RFID"
//和LCD复用MISO、MOSI、SCK引脚，NSS用自己的



//回调函数，当卡片状态改变触发
static void on_picc_state_changed(void *arg, esp_event_base_t base,
                                  int32_t event_id, void *data)
{
    (void)arg;
    (void)base;
    (void)event_id;

    rc522_picc_state_changed_event_t *event = data;
    rc522_picc_t *picc = event->picc;

    if (picc->state == RC522_PICC_STATE_ACTIVE) {
        char uid[32];
        if (rc522_picc_uid_to_str(&picc->uid, uid, sizeof(uid)) == ESP_OK) {
            ESP_LOGI(TAG, "card UID=%s", uid);
        }
        AppEvent_Post(EVT_RFID_OK);
    }
}

void Rfid_Start(void)
{
    static rc522_driver_handle_t s_driver;
    static rc522_handle_t s_scanner;

    // 复用屏幕已经初始化好的 SPI2：bus_config 传 NULL
    rc522_spi_config_t driver_cfg = {
        .host_id = SPI2_HOST,
        .bus_config = NULL,
        .dev_config = {
            .spics_io_num = RFID_CS,
        },
        .dma_chan = SPI_DMA_CH_AUTO,
        .rst_io_num = RFID_RST,
    };

    rc522_spi_create(&driver_cfg, &s_driver);
    rc522_driver_install(s_driver);

    rc522_config_t scanner_cfg = {
        .driver = s_driver,
    };
    rc522_create(&scanner_cfg, &s_scanner);
    rc522_register_events(s_scanner, RC522_EVENT_PICC_STATE_CHANGED,
                          on_picc_state_changed, NULL);
    rc522_start(s_scanner);

    ESP_LOGI(TAG, "started on SPI2 (CS=GPIO%d RST=GPIO%d)", RFID_CS, RFID_RST);
}
