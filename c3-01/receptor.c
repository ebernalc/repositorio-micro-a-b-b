#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define LED GPIO_NUM_16      //gpio16 
#define PWM_FREQ 5000
#define PWM_CHANNEL LEDC_CHANNEL_0
#define PWM_RESOLUTION LEDC_TIMER_8_BIT  // resolución 256 niveles

static const char* TAG = "RECEPTOR";

// callback cuando se recibe un mensaje ESP-NOW
void on_receive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
    if (len == sizeof(uint8_t)) {  // verifica tamaño=1 byte (duty cycle)
        uint8_t duty = data[0];
        ESP_LOGI(TAG, "Duty recibido: %d", duty);

        // Actualiza el PWM
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
    } else {
        ESP_LOGE(TAG, "Error: Mensaje de tamaño incorrecto (%d bytes)", len);
    }
}

void app_main() {
    // init del NVS y WiFi
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&(wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT()));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    //Confi WiFi (mismo que el emisor, canal 1)
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    // init del ESP-NOW y registrar callback
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_receive));

    // configuración PWM para el LED
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {
        .gpio_num = LED,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = PWM_CHANNEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,  // Inicia con el duty cycle al 0%
        .hpoint = 0
    };
    ledc_channel_config(&pwm_channel);

    ESP_LOGI(TAG, "Receptor listo. Esperando datos...");
}
