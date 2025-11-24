#include<stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "wifi_sta.h"

static const char *TAG = "ESP32C5";

static const uint64_t connection_timout_ms = 10000;
// static const uint32_t sleep_duration_ms = 1000;

void app_main() {
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS flash: %s", esp_err_to_name(ret));
        abort();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "Device started");

    // Initialize WiFi in station mode
    EventGroupHandle_t wifi_event_group = xEventGroupCreate();
    EventBits_t network_event_bits;

    ret = esp_netif_init();
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to initialize esp netif: %s", esp_err_to_name(ret));
        abort();
    }

    ret = esp_event_loop_create_default();
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to create default event loop: %s", esp_err_to_name(ret));
        abort();
    }

    ret = wifi_sta_init(wifi_event_group);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Failed to initialize WiFi STA: %s", esp_err_to_name(ret));
        abort();
    }

    network_event_bits = xEventGroupWaitBits(wifi_event_group,
                                                WIFI_STA_CONNECTED_BIT,
                                                pdFALSE,
                                                pdTRUE,
                                                pdMS_TO_TICKS(connection_timout_ms));
    if((network_event_bits & WIFI_STA_CONNECTED_BIT)){
        ESP_LOGI(TAG, "WiFi connected successfully");
    } else {
        ESP_LOGE(TAG, "WiFi connection timed out");
        abort();
    }

    network_event_bits = xEventGroupWaitBits(wifi_event_group,
                                                WIFI_STA_IPV4_OBTAINED_BIT,
                                                pdFALSE,
                                                pdTRUE,
                                                pdMS_TO_TICKS(connection_timout_ms));
    if((network_event_bits & WIFI_STA_IPV4_OBTAINED_BIT)){
        ESP_LOGI(TAG, "IPv4 address obtained successfully");
    } else if(network_event_bits & WIFI_STA_IPV6_OBTAINED_BIT){
        ESP_LOGI(TAG, "IPv6 address obtained successfully");
    } else {
        ESP_LOGE(TAG, "IP address obtain timed out");
    }

    while (1) {
        
        printf("Hello, World!\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}