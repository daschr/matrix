#include "matrixstuff.h"
#include "matrixstuff_font.h"

#include <string.h>
#include <led_strip.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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


void matrix_draw_text(max7219_t *dev, char *s) {
    for(int i=0; s[i]!='\0'; ++i) {
        max7219_clear(dev);
        for(int t=0; t<dev->cascade_size && s[i+t]!='\0'; ++t)
            max7219_draw_image_8x8(dev, t*8, (const void *) &matrix_font[s[i+t]-' ']);
        vTaskDelay(750 / portTICK_PERIOD_MS);
    }
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

    for(;;) {
        matrix_draw_text(&dev, "Hallo Papa, das ist ein Test.");

        vTaskDelay(SCROLL_DELAY / portTICK_PERIOD_MS);
    }
    /*	int ch=' ';
        while (1)
        {
            printf("---------- draw\n");

            max7219_draw_image_8x8(&dev, 0, (const void *) &matrix_font[ch-' ']);
            vTaskDelay(SCROLL_DELAY / portTICK_PERIOD_MS);

    		if(++ch >'z')
    				ch=' ';
        }
    	*/
}
