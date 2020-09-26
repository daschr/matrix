#include "ledstuff.h"
#include <string.h>
#include <led_strip.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void cool_led_effects(void *stuff) {
    led_strip_t *led_strip=malloc(sizeof(led_strip_t));
    memset(led_strip, 0, sizeof(led_strip_t));

    led_strip->type = LED_STRIP_SK6812;
    led_strip->channel = RMT_CHANNEL_0;
#ifdef ENV_CUSTOM_LED_PIN
    led_strip->gpio = ENV_CUSTOM_LED_PIN;
#else
    led_strip->gpio = GPIO_NUM_18;
#endif
    led_strip->buf=NULL;
    led_strip->length = LED_STRIP_LENGTH;

    led_strip_install();

    if(led_strip_init(led_strip)!=ESP_OK) {
        free(led_strip);
        printf("could not init led :((\n");
        return;
    }


    rgb_t color;
	color.b=0;
	color.r=0;
	color.g=0;

	int s=1, res=0;
    for(;;) {
		res=(res+s)%101;
        if(res==100) s=-1;
		else if(res==0 && s==-1) s=1;

		//color.r=(0xff*res)/100;
        color.g=(0xff*(100-res))/100;


		printf("r: %d g: %d b: %d\n", color.r, color.g, color.b);
        if(led_strip_fill(led_strip, 0, led_strip->length, color)!=ESP_OK) {
            printf("error filling leds :(\n");
            return;
        }

        led_strip_flush(led_strip);

        vTaskDelay(SLEEP_TIME/portTICK_PERIOD_MS);
	}
    return;
}
