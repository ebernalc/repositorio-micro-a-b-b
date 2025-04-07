/*
Santiago Alvarez Pino;
Iván Barboza de los reyes;
Esteban Bernal Chiquillo.

1. Se puede calcular el cuadrado de un número N al sumar los N primeros números impares, así 7 al cuadrado es igual a 1 + 3 + 5 + 7 + 9 + 11 + 13 = 49, donde la serie del 1 al 13 son los primeros 7 números impares.

2. Describir una solución para el microcontrolador que calcule el cuadrado de un número recibido por puerto serial e imprima el resultado por serial.

3. Si se recibe algo diferente a un número entero se debe ignorar.
*/

#include <stdio.h>          // Biblioteca para entrada/salida estándar
#include <string.h>         // Biblioteca para manejo de cadenas de caracteres
#include <ctype.h>          // Biblioteca para funciones de caracteres (isdigit, etc.)
#include "freertos/FreeRTOS.h"    // Biblioteca de FreeRTOS para tareas y control de tiempo
#include "freertos/task.h"         // Biblioteca para crear y manejar tareas en FreeRTOS
#include "driver/uart.h"   // Biblioteca para manejar el UART (puerto serial)

#define BUF_SIZE 1024      // Definición del tamaño del buffer para la recepción de datos

// Función para verificar si un string es un número entero positivo
bool es_entero(const char* str) {
    int len = strlen(str);   // Calcula la longitud de la cadena
    if (len == 0) return false;  // Si la cadena está vacía, no es un número entero
    for (int i = 0; i < len; i++) {
        // Recorre cada carácter de la cadena y verifica si es un dígito
        if (!isdigit((unsigned char)str[i])) return false;  // Si hay un carácter no dígito, retorna false
    }
    return true;  // Si todos los caracteres son dígitos, es un número entero positivo
}

// Función que calcula el cuadrado de un número sumando los números impares
int calcular_cuadrado(int N) {
    int suma = 0;    // Variable para almacenar la suma
    int impar = 1;   // Empezamos con el primer número impar
    for (int i = 0; i < N; i++) {
        suma += impar;    // Sumamos el número impar a la suma
        impar += 2;       // Aumentamos el número impar para el siguiente ciclo
    }
    return suma;   // Retorna el resultado de la suma, que es el cuadrado de N
}

void app_main() {
    // Configuración de UART (Puerto serie) por defecto, se usa UART0
    const uart_port_t uart_num = UART_NUM_0;

    uart_config_t uart_config = {
        .baud_rate = 115200,          // Velocidad de transmisión
        .data_bits = UART_DATA_8_BITS,    // Bits de datos por paquete
        .parity    = UART_PARITY_DISABLE, // Sin paridad
        .stop_bits = UART_STOP_BITS_1,    // Un bit de parada
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE  // Deshabilita control de flujo por hardware
    };

    // Configura los parámetros del puerto UART y los instala
    uart_param_config(uart_num, &uart_config);
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);

    char data[BUF_SIZE];  // Buffer para almacenar los datos recibidos del UART

    printf("Esperando número entero para calcular su cuadrado...\n");  // Mensaje inicial

    while (1) {
        // Limpiar el buffer para asegurarse de que no hay datos antiguos
        memset(data, 0, sizeof(data));

        // Lee datos del UART hasta el tamaño del buffer o hasta 5 segundos de espera
        int len = uart_read_bytes(uart_num, (uint8_t *)data, BUF_SIZE - 1, pdMS_TO_TICKS(5000));
        if (len > 0) {  // Si se recibieron datos
            data[len] = '\0';  // Asegura que la cadena termine correctamente

            // Eliminar saltos de línea o retorno de carro (\r o \n) al final de la cadena
            char *newline = strpbrk(data, "\r\n");
            if (newline) *newline = '\0';

            if (es_entero(data)) {  // Verifica si la entrada es un número entero
                int numero = atoi(data);  // Convierte la cadena a un número entero
                int resultado = calcular_cuadrado(numero);  // Calcula el cuadrado del número
                printf("El cuadrado de %d es: %d\n", numero, resultado);  // Imprime el resultado
            } else {
                printf("Entrada inválida. Ignorada.\n");  // Si no es un número entero, muestra un mensaje de error
            }
        }

        // Retardo de 100ms antes de la siguiente iteración
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}