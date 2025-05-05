#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define POT ADC1_CHANNEL_0 //gpio36

static const char* TAG = "EMISOR";

uint8_t receptor_mac[] = {0x24, 0x6F, 0x28, 0x00, 0x00, 0x00};  // cambia la MAC

void app_main() {
    // init del NVS y Wi-Fi
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&(wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT()));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Confi WiFi
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    // init del ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());

    // Añadir receptor
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, receptor_mac, 6);
    peer.channel = 1;
    peer.ifidx = ESP_IF_WIFI_STA;
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    // Configurar ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POT, ADC_ATTEN_DB_11);
    int adc_raw;

    while (true) {
        adc_raw = adc1_get_raw(POT); 
        uint8_t duty = (adc_raw * 255) / 4095;

        // enviar duty cycle al receptor
        esp_err_t result = esp_now_send(receptor_mac, &duty, sizeof(duty));
        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Enviado: %d", duty);
        } else {
            ESP_LOGE(TAG, "Error al enviar: %d", result);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
