#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "config.h"


static void setup_ap(void) {
    esp_netif_t *iface=esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = STA_WIFI_SSID,
            .ssid_len = strlen(STA_WIFI_SSID),
            .channel = STA_WIFI_CHANNEL,
            .password = STA_WIFI_PASS,
            .max_connection = STA_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(STA_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_netif_set_hostname(iface, CONF_HOSTNAME);
}

static void setup_sta(char *ssid, char *pass) {
    printf("setting up sta [%s]=%s\n", ssid, pass);
    esp_netif_t *iface=esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();

    static wifi_sta_config_t wifi_config;
    wifi_config.pmf_cfg.capable=true;
    wifi_config.pmf_cfg.required=false;
    strcpy((char *) wifi_config.ssid, ssid);
    strcpy((char *) wifi_config.password, pass);

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, (wifi_config_t *) &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_netif_set_hostname(iface, CONF_HOSTNAME);
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void setup_wifi(void) {
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    nvs_handle_t nvs_handle;
    if(nvs_open("nvs", NVS_READONLY, &nvs_handle)!=ESP_OK) {
        printf("could not open nvs, setting up ap...\n");
        setup_ap();
        return;
    }

    char ssid[32], pass[64];
    size_t length_ssid=31, length_pass=63;
    memset(ssid, 0, 32*sizeof(char));
    memset(pass, 0, 64*sizeof(char));

    if(	nvs_get_str(nvs_handle, "ssid", ssid, &length_ssid)!=ESP_OK ||
            nvs_get_str(nvs_handle, "pass", pass, &length_pass)!=ESP_OK) {
        printf("could not find ssid or pass, setting up ap...\n");
        setup_ap();
        return;
    }

    setup_sta(ssid, pass);
}

void reset_wifi(void){
	nvs_handle_t nvs_handle;
	if(nvs_open("nvs", NVS_READWRITE, &nvs_handle)!=ESP_OK){
		printf("could not open nvs");
		return;
	}

	nvs_erase_key(nvs_handle, "ssid");
	nvs_erase_key(nvs_handle, "pass");
}

uint16_t scan_wifi(wifi_ap_record_t **ssids){
	wifi_scan_config_t scan_config;
	wifi_scan_time_t scan_time={ .active=(wifi_active_scan_time_t) {.min=0, .max=150}, .passive=1500};
	memset(&scan_config, 0, sizeof(wifi_scan_config_t));
	
	scan_config.scan_type=WIFI_SCAN_TYPE_ACTIVE;
	scan_config.scan_time=scan_time;

	ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

	uint16_t num_found;
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&num_found));
	*ssids=malloc(sizeof(wifi_ap_record_t)*num_found);

	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num_found, *ssids));
	
	return num_found;
}
