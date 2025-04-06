/*
Santiago Alvarez Pino;
Iván Barboza de los reyes;
Esteban Bernal Chiquillo.

Desarrolla un sistema de autenticación basado en un patrón táctil.

1. El usuario debe tocar la secuencia en un pin táctil:
(a) 3 toques largos.
(b) 3 toques cortos.
(c) 3 toques largos.

2. Luego de hacer esa secuencia, se debe tocar otro pin táctil para validar la secuencia.

3. Imprimir por serial "APROBADO" o "NO APROBADO" si la secuencia ingresada es correcta o no.

4. Por grupo definir el tiempo a su criterio para determinar que es "toque largo" y por "toque corto".
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_timer.h"

uint16_t touch_value;
uint64_t tiempo_inicial;
uint64_t tiempo_final;

void app_main() {
    touch_pad_init();
    touch_pad_config(TOUCH_PAD_NUM3, 0);  // Pin para ingresar
    touch_pad_config(TOUCH_PAD_NUM0, 0);  // Pin para validar

    int clave[9]= {3,3,3,1,1,1,3,3,3};
    int ingresado[9]= {0};
    int j=0;
    int tocando1= 0;
    int tocando2= 0;

    while (1) {
        touch_pad_read(TOUCH_PAD_NUM3, &touch_value); //Lectura pin tactil de datos
        if (touch_value<=120 && tocando1==0) {
            tiempo_inicial= esp_timer_get_time();
            tocando1= 1;
        } else if (touch_value>120 && tocando1==1) {
            tiempo_final = esp_timer_get_time();
            uint64_t duracion = tiempo_final - tiempo_inicial;
            if (j < 9) {
                if (duracion >= 1000000) {
                    printf("toque largo\n");
                    ingresado[j] = 3;
                } else {
                    printf("toque corto\n");
                    ingresado[j] = 1;
                }
                j++;
            }
            tocando1 = 0;
        }
        touch_pad_read(TOUCH_PAD_NUM0, &touch_value); //Lectura pin de confirmación
        if(touch_value<= 120 && tocando2==0)tocando2=1;
        if(touch_value>120 && tocando2==1){
            int aprobado = 1;
            for (int i=0; i < 9; i++) {
                if (ingresado[i] != clave[i]) {
                    aprobado= 0;
                    break;
                }
            }
            if (aprobado==1) {
                printf("APROBADO\n");
            } else {
                printf("NO APROBADO\n");
            }
            for (int i = 0; i < 9; i++) {
                ingresado[i] = 0;
            }
            j = 0;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            tocando2=0;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);  
    }
}
