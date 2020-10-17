#include "esp_http_server.h"
#include "config.h"

httpd_handle_t start_webserver();
void stop_webserver(httpd_handle_t server);
