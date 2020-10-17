#include <stddef.h>
#include <esp_https_ota.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_http_server.h"
#include <math.h>
#include <nvs_flash.h>
#include "esp_wifi.h"
#include "wifi.h"

#include "config.h"
#include "helper.h"

#define BME_BUFSIZE 128
#define QUERY_MAXSIZE 64

extern char misc_cert_pem[];
extern char misc_html_index_html[];
extern char misc_html_redirect_html[];

esp_err_t ota_get(httpd_req_t *r) {
    char *payload=	"<html><body><form action=\"/ota\" method=\"post\">"
                    "OTA-URL: <input type=\"text\" name=\"url\"><br>"
                    "<input type=\"submit\" value=\"Submit\">"
                    "</form></body></html>";
    httpd_resp_send(r, payload, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t ota_post(httpd_req_t *r) {
    if(r->content_len > STND_BUFSIZE) {
        httpd_resp_set_status(r, "413");
        httpd_resp_sendstr(r, "too large :(\n");
        return ESP_FAIL;
    }

    size_t recv_size = r->content_len<STND_BUFSIZE? r->content_len : STND_BUFSIZE;

    char *content=malloc(recv_size+1);
    memset(content, 0, recv_size+1);

    esp_err_t ret = httpd_req_recv(r, content, recv_size);
    if(ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            httpd_resp_send_408(r);
        free(content);
        return ESP_FAIL;
    }

    char *raw_url=malloc(recv_size+1);
    memset(raw_url, 0, recv_size+1);
    if(httpd_query_key_value(content, "url", raw_url, recv_size) != ESP_OK) {
        printf("raw_url: %s\n", raw_url);
        httpd_resp_set_status(r, "400");
        httpd_resp_sendstr(r, "url missing :(\n");
        free(content);
        free(raw_url);
        return ESP_FAIL;
    }

    char *url=content;
    memset(url, 0, recv_size+1);
    if(!url_decode(raw_url, url, recv_size+1)) {
        httpd_resp_set_status(r, "413");
        httpd_resp_sendstr(r, "url too long\n");
        free(content);
        free(raw_url);
        return ESP_FAIL;
    }

    free(raw_url);

    printf("ota-url: %s\n",url);

    esp_http_client_config_t upgrade_config= {0};

    upgrade_config.url = url;
    upgrade_config.timeout_ms=10000;
    upgrade_config.cert_pem= misc_cert_pem;
    upgrade_config.skip_cert_common_name_check=1;


    printf("doing upgrade...\n");
    printf("free heap size: %u min free heap size: %u\n", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    ret = esp_https_ota(&upgrade_config);
    free(url);

    if (ret == ESP_OK) {
        printf("upgrade successfull!\n");
        httpd_resp_sendstr(r, "upgrade successfull!\n");
        esp_restart();
    } else {
        printf("failed to upgrade\n!");
        httpd_resp_set_status(r, "400");
        httpd_resp_sendstr(r, "Failed to upgrade :(\n");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t store_wifi_creds_handler(httpd_req_t *r) {
    if(r->content_len > STND_BUFSIZE) {
        httpd_resp_set_status(r, "413");
        httpd_resp_sendstr(r, "too large :(\n");
        return ESP_FAIL;
    }

    nvs_handle_t nvs_handle;
    if(nvs_open("nvs", NVS_READWRITE, &nvs_handle)!=ESP_OK) {
        httpd_resp_set_status(r, "408");
        httpd_resp_sendstr(r, "could not open nvs :(\n");
        return ESP_FAIL;
    }

    static const char *params[]= {"ssid", "pass"};
    uint8_t retc=ESP_OK;
    char *content=NULL, *raw_val=NULL, *decoded_val=NULL;
    size_t recv_size = r->content_len<STND_BUFSIZE? r->content_len : STND_BUFSIZE;

    content=malloc(recv_size+1);
    memset(content, 0, recv_size+1);

    esp_err_t ret = httpd_req_recv(r, content, recv_size);
    if(ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            httpd_resp_send_408(r);
        retc=ESP_FAIL;
        goto end;
    }

    raw_val=malloc(recv_size+1);
    decoded_val=malloc(recv_size+1);

    for(size_t i=0; i<sizeof(params)/sizeof(const char *); ++i) {
        memset(raw_val, 0, recv_size+1);
        memset(decoded_val, 0, recv_size+1);

        if(httpd_query_key_value(content, params[i], raw_val, recv_size) != ESP_OK) {
            httpd_resp_set_status(r, "400");
            httpd_resp_sendstr(r, "ssid or pass missing :(\n");
            retc=ESP_FAIL;
            goto end;
        }

        if(!url_decode(raw_val, decoded_val, recv_size+1)) {
            httpd_resp_set_status(r, "413");
            httpd_resp_sendstr(r, "value too long :(\n");
            retc=ESP_FAIL;
            goto end;
        }

        if(nvs_set_str(nvs_handle, params[i], decoded_val) != ESP_OK) {
            httpd_resp_set_status(r, "408");
            httpd_resp_sendstr(r, "could not store config param\n");
            goto end;
        }

        printf("setting [%s]=%s\n", params[i], decoded_val);
    }

    httpd_resp_sendstr(r, misc_html_redirect_html);

    nvs_commit(nvs_handle);
end:
    nvs_close(nvs_handle);

    free(content);
    free(raw_val);
    free(decoded_val);

    return retc;
}

#define FORMAT "{\"ssid\":\"%s\",\"rssi\":%d}%c"
esp_err_t get_wifi_ssids(httpd_req_t *r) {
    wifi_ap_record_t *ssids=NULL;
    int found_ssids=scan_wifi(&ssids);
    httpd_resp_set_type(r, "application/json");
    if(found_ssids==0) {
        httpd_resp_sendstr(r, "[]");
        free(ssids);
        return ESP_OK;
    }

    char *buf=malloc((strlen(FORMAT)+33+8)*found_ssids);
    char *sp=buf+1;
    memset(buf, 0, (strlen(FORMAT)+33+8)*found_ssids);
    buf[0]='[';

    for(int i=0; i<found_ssids; ++i)
        sp+=sprintf(sp, FORMAT, ssids[i].ssid, ssids[i].rssi, i==found_ssids-1?']':',');

    httpd_resp_sendstr(r, buf);
    free(buf);
    free(ssids);
    return ESP_OK;
}
#undef FORMAT

esp_err_t reset_wifi_conf(httpd_req_t *r) {
    reset_wifi();
    httpd_resp_sendstr(r, "success\n");
    return ESP_OK;
}

esp_err_t matrix_handler(httpd_req_t *r) {
    if(r->content_len > STND_BUFSIZE) {
        httpd_resp_set_status(r, "413");
        httpd_resp_sendstr(r, "too large :(\n");
        return ESP_FAIL;
    }

    nvs_handle_t nvs_handle;
    if(nvs_open("nvs", NVS_READWRITE, &nvs_handle)!=ESP_OK) {
        httpd_resp_set_status(r, "408");
        httpd_resp_sendstr(r, "could not open nvs :(\n");
        return ESP_FAIL;
    }

    uint8_t retc=ESP_OK;
    char *content=NULL, *raw_val=NULL, *decoded_val=NULL;
    size_t recv_size = r->content_len<STND_BUFSIZE? r->content_len : STND_BUFSIZE;

    content=malloc(recv_size+1);
    memset(content, 0, recv_size+1);

    esp_err_t ret = httpd_req_recv(r, content, recv_size);
    if(ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            httpd_resp_send_408(r);
        retc=ESP_FAIL;
        goto end;
    }

    printf("content: %s\n", content);
    raw_val=malloc(recv_size+1);
    decoded_val=malloc(recv_size+1);

    memset(raw_val, 0, recv_size+1);
    memset(decoded_val, 0, recv_size+1);

    if(httpd_query_key_value(content, "text", raw_val, recv_size) != ESP_OK) {
        httpd_resp_set_status(r, "400");
        httpd_resp_sendstr(r, "text param missing :(\n");
        retc=ESP_FAIL;
        goto end;
    }

    if(!url_decode(raw_val, decoded_val, recv_size+1)) {
        httpd_resp_set_status(r, "413");
        httpd_resp_sendstr(r, "value too long :(\n");
        retc=ESP_FAIL;
        goto end;
    }

	int i=0;
    for(char *c=decoded_val; *c!='\0'; ++i) {
        if(*c==0xc3 && ( *(c+1)==0xb6 || *(c+1)==0x96)) {
            decoded_val[i]='~'+1;
            c+=2;
        }
        else if(*c==0xc3 && (*(c+1)==0xbc || *(c+1)==0x9c)) {
            decoded_val[i]='~'+2;
            c+=2;
        }
        else if(*c==0xc3 && (*(c+1)==0xa4 || *(c+1)==0x84)) {
            decoded_val[i]='~'+3;
            c+=2;
        }
        else{
            decoded_val[i]=*c;
			++c;
		}
    }
	decoded_val[i]='\0';

    if(nvs_set_str(nvs_handle, "text", decoded_val) != ESP_OK) {
        httpd_resp_set_status(r, "408");
        httpd_resp_sendstr(r, "could not store text\n");
        goto end;
    }

    printf("setting [text]=%s\n", decoded_val);

    httpd_resp_sendstr(r, misc_html_redirect_html);

    nvs_commit(nvs_handle);
end:
    nvs_close(nvs_handle);

    free(content);
    free(raw_val);
    free(decoded_val);

    return retc;
}

esp_err_t get_restart(httpd_req_t *r) {
    esp_restart();
    return ESP_OK;
}

esp_err_t get_index(httpd_req_t *r) {
    httpd_resp_sendstr(r, misc_html_index_html);
    return ESP_OK;
}

httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_index,
    .user_ctx = NULL
};

httpd_uri_t reset_wifi_uri = {
    .uri = "/reset-wifi",
    .method = HTTP_GET,
    .handler = reset_wifi_conf,
    .user_ctx = NULL
};

httpd_uri_t matrix_uri = {
    .uri = "/matrix",
    .method = HTTP_POST,
    .handler = matrix_handler,
    .user_ctx = NULL
};

httpd_uri_t wifi_uri = {
    .uri = "/wifi",
    .method = HTTP_POST,
    .handler = store_wifi_creds_handler,
    .user_ctx = NULL
};

httpd_uri_t get_wifi_ssids_uri = {
    .uri = "/ssids",
    .method = HTTP_GET,
    .handler = get_wifi_ssids,
    .user_ctx = NULL
};

httpd_uri_t get_restart_uri = {
    .uri = "/restart",
    .method = HTTP_GET,
    .handler = get_restart,
    .user_ctx = NULL
};

httpd_uri_t ota_uri_get = {
    .uri = "/ota",
    .method = HTTP_GET,
    .handler = ota_get,
    .user_ctx = NULL
};

httpd_uri_t ota_uri_post = {
    .uri = "/ota",
    .method = HTTP_POST,
    .handler = ota_post,
    .user_ctx = NULL
};

httpd_handle_t start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        //set matrix text
        httpd_register_uri_handler(server, &matrix_uri);

        //index
        httpd_register_uri_handler(server, &index_uri);

        //allows storing other wifi creds
        httpd_register_uri_handler(server, &wifi_uri);

        //reset wifi creds
        httpd_register_uri_handler(server, &reset_wifi_uri);

        //list ssids and rssi
        httpd_register_uri_handler(server, &get_wifi_ssids_uri);

        //allows restart over http-get
        httpd_register_uri_handler(server, &get_restart_uri);

        //OTA
        httpd_register_uri_handler(server, &ota_uri_get);
        httpd_register_uri_handler(server, &ota_uri_post);
    } else {
        printf("could not start webserver :(\n");
    }
    return server;
}



/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server) {
    if (server)
        httpd_stop(server);
}
