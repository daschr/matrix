#include "matrixstuff.h"
#include "matrixstuff_font.h"

#include <nvs_flash.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config.h"
#include <max7219.h>

#define SCROLL_DELAY 1000
#define CASCADE_SIZE 4

#define HOST HSPI_HOST

#define PIN_NUM_MOSI 19
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

#define CHECK(expr, msg) \
    while ((res = expr) != ESP_OK) { \
        printf(msg "\n", res); \
        vTaskDelay(250 / portTICK_RATE_MS); \
    }

static void matrix_show_buf(max7219_t *dev, uint8_t *buf, uint8_t *zb, int8_t first_pause) {
    for(int p=0; p<8; ++p) {
        for(int m=0; m<dev->cascade_size; ++m) {
            for(int i=0; i<8; ++i)
                zb[sizeof(int64_t)*m+i]=(*(buf+sizeof(int64_t)*m+i)>>p) + ((*(buf+sizeof(int64_t)*(m+1)+i) & ((1<<p)-1))<<(8-p));
        }

        for(int m=0; m<dev->cascade_size; ++m)
            max7219_draw_image_8x8(dev, m*8, (const void *) (zb+sizeof(int64_t)*m) );

        vTaskDelay((first_pause && p==0?400:50) / portTICK_PERIOD_MS);
    }
}

static void matrix_draw_text(max7219_t *dev, char *s) {
    uint8_t *buf=malloc(sizeof(int64_t)*(dev->cascade_size+1));
    uint8_t *zb=malloc(sizeof(int64_t)*dev->cascade_size);

    for(int p=0; s[p]!='\0'; ++p) {
        memset(buf, 0, sizeof(int64_t)*(dev->cascade_size+1));
        for(int i=0; s[p+i]!='\0' && i<dev->cascade_size+1; ++i) {
            if(s[p+i]>=' ' && s[p+i]<='~'+3)
                memcpy(buf+sizeof(int64_t)*i, &matrix_font[s[p+i]-' '], sizeof(int64_t));
            else
                memcpy(buf+sizeof(int64_t)*i, &matrix_font['?'-' '], sizeof(int64_t));
        }
        matrix_show_buf(dev, buf, zb, (p==0));
    }

    memset(buf, 0, sizeof(int64_t)*(dev->cascade_size+1));
    matrix_show_buf(dev, buf, zb, 0);

    free(buf);
    free(zb);
}

void matrix_task(void *pvParameter) {
    esp_err_t res;

    // Configure SPI bus
    spi_bus_config_t cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0
    };
    CHECK(spi_bus_initialize(HOST, &cfg, 1),
          "Could not initialize SPI bus: %d");

    // Configure device
    max7219_t dev = {
        .cascade_size = CASCADE_SIZE,
        .digits = 0,
        .mirrored = true
    };
    CHECK(max7219_init_desc(&dev, HOST, PIN_NUM_CS),
          "Could not initialize MAX7129 descriptor: %d");
    CHECK(max7219_init(&dev),
          "Could not initialize MAX7129: %d");

    nvs_handle_t nvs_handle;
    while(nvs_open("nvs", NVS_READONLY, &nvs_handle)!=ESP_OK) {
        printf("could not init nvs, sleeping...\n");
        vTaskDelay(3000/portTICK_PERIOD_MS);
    }

    char *buf=malloc(STND_BUFSIZE);
    size_t length;

    for(;;) {
        memset(buf, 0, STND_BUFSIZE);
        length=STND_BUFSIZE;
        nvs_get_str(nvs_handle, "text", buf, &length);

        matrix_draw_text(&dev, buf);

        vTaskDelay(SCROLL_DELAY / portTICK_PERIOD_MS);
    }
}
