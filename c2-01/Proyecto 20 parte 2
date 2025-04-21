#include <stdio.h>
#include <stdlib.h>
#include "driver/touch_pad.h"
#include "driver/adc.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define touch TOUCH_PAD_NUM0 // GPIO-4 o pin 26
#define potenciometro ADC_CHANNEL_6
#define ADC_WIDTH ADC_WIDTH_BIT_12  // Resolución de 12 bits (0-4095)
#define ADC_ATTEN ADC_ATTEN_DB_11   // Rango hasta 3.3V

float ADC_VOLT_MAX = 3.3;
uint16_t touch_value;

void app_main(){
    // Configurar ADC
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(potenciometro, ADC_ATTEN);

    // Esperar comando '1' + Enter
    char ch;
    printf("Esperando comando '1' + Enter para iniciar...\n");

    while (1) {
        ch = getchar();
        if (ch == '1') {
            // Esperar a que se presione Enter después del '1'
            ch = getchar();
            if (ch == '\r' || ch == '\n') {
                printf("Comando correcto recibido. Iniciando lectura táctil y ADC...\n");
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Pequeño retardo para evitar saturación
    }

    // Iniciar pin táctil
    touch_pad_init();
    touch_pad_config(touch, 0);

    // Bucle principal
    while (1)
    {
        touch_pad_read(touch, &touch_value);

        if (touch_value < 500) {
            int raw = adc1_get_raw(potenciometro);
            float porcentaje = (raw / 4095.0f) * 100.0f;
            printf("ADC: %d - %.2f%%\n", raw, porcentaje);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo
    }
}
