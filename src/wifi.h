#ifndef _inc_wifi
#define _inc_wifi


#include "esp_wifi.h"
void setup_wifi(void);
uint16_t scan_wifi(wifi_ap_record_t **ssids);
void reset_wifi(void);
#endif
