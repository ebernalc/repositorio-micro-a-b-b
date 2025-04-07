/*
Santiago Alvarez Pino;
Iván Barboza de los reyes;
Esteban Bernal Chiquillo.

En un proyecto de telemetría se tiene un caudalímetro que envía por puerto serial cada período de tiempo la cantidad de caudal detectada, el cual es un número entero no mayor de 2 dígitos (es decir, se enviará un número entre 00 y 99).
Se recibirá el número por puerto serial, se debe imprimir por serial la siguiente información:

1. El número mínimo recibido.

2. El número mayor recibido.

3. El último número recibido.

4. El promedio de todos los números recibidos.

Un ejemplo de este programa es el siguiente:

Último: 10. Mínimo: 10. Máximo: 10. Promedio: 10.00
Último: 1. Mínimo: 1. Máximo: 10. Promedio: 5.50
Último: 27. Mínimo: 1. Máximo: 27. Promedio: 12.67

En el anterior ejemplo, se han enviado por serial 3 números en orden que son 10, 1 y 27. Donde al recibir el tercer dato (27), el número menor en ese momento es 1, el número mayor es 27, el último número recibido es 27 y el promedio de la sumatoria de todos los números recibidos (10 + 1 + 27) ÷ 3 = 12.667

Si se recibe algo diferente entre 00 y 99 (sea número o letra) se debe ignorar.
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_PORT UART_NUM_0
#define BUF_SIZE 1024
#define RD_BUF_SIZE 3

static int min_val = 99;
static int max_val = 0;
static int ultimo_val = 0;
static int suma_total = 0;
static int contador = 0;

void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_PORT, &uart_config);
    uart_driver_install(UART_PORT, BUF_SIZE, 0, 0, NULL, 0);
}

void process_data(char* data) {
    if (strlen(data) == 2 && 
        isdigit((unsigned char)data[0]) && 
        isdigit((unsigned char)data[1])) {
        
        int numero = (data[0] - '0') * 10 + (data[1] - '0');
        ultimo_val = numero;
        suma_total += numero;
        contador++;

        if (numero < min_val) min_val = numero;
        if (numero > max_val) max_val = numero;

        float promedio = (contador > 0) ? (float)suma_total / contador : 0;
        printf("Último: %d. Mínimo: %d. Máximo: %d. Promedio: %.2f\n",
               ultimo_val, min_val, max_val, promedio);
    }
}

void uart_task(void *pvParameters) {
    uint8_t data[RD_BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_PORT, data, RD_BUF_SIZE - 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            data[len] = '\0';
            process_data((char*)data);
        }
    }
}

void app_main() {
    init_uart();
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);
}