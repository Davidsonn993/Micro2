
#define F_CPU 8000000UL  // 8 MHz
#include <avr/io.h>
#include <util/delay.h>

#include <stdlib.h>  // Para generar números aleatorios

// Definir pines de LEDs y botones
#define LED0 PB0
#define LED1 PB1
#define LED2 PB2
#define LED3 PB3
#define BTN0 PB4
#define BTN1 PB5
#define BTN2 PB6
#define BTN3 PB7

// Función para hacer parpadear todos los LEDs
void parpadeo_todos_leds(int veces, uint16_t delay_ms) {
    for (int i = 0; i < veces; i++) {
        PORTB |= (1 << LED0) | (1 << LED1) | (1 << LED2) | (1 << LED3);
        for (uint16_t j = 0; j < delay_ms; j++) _delay_ms(1);  // Bucle para retraso variable
        PORTB &= ~((1 << LED0) | (1 << LED1) | (1 << LED2) | (1 << LED3));
        for (uint16_t j = 0; j < delay_ms; j++) _delay_ms(1);  // Bucle para retraso variable
    }
}

// Función para encender un LED
void encender_led(uint8_t led, uint16_t delay_ms) {
    PORTB |= (1 << led);
    for (uint16_t i = 0; i < delay_ms; i++) _delay_ms(1);  // Bucle para retraso variable
    PORTB &= ~(1 << led);
}


// Función para leer el estado de un botón
uint8_t leer_boton(uint8_t boton) {
    return PINB & (1 << boton);
}



int main(void) {
    // Configurar LEDs como salida
    DDRB |= (1 << LED0) | (1 << LED1) | (1 << LED2) | (1 << LED3);

    // Configurar botones como entrada (sin pull-up)
    DDRB &= ~((1 << BTN0) | (1 << BTN1) | (1 << BTN2) | (1 << BTN3));
    PORTB &= ~((1 << BTN0) | (1 << BTN1) | (1 << BTN2) | (1 << BTN3));  // Sin pull-up

    int secuencia[10];  // Para almacenar la secuencia del juego
    int nivel = 4;  // Comienza con una secuencia de 4 LEDs
    int tiempo_encendido = 2000;  // 2000 ms (2 segundos) inicialmente

    // Esperar a que se presione un botón para iniciar el juego
    while (leer_boton(BTN0) == 0 && leer_boton(BTN1) == 0 && leer_boton(BTN2) == 0 && leer_boton(BTN3) == 0);

    // Parpadeo inicial para indicar que el juego ha comenzado
    parpadeo_todos_leds(2, 500);



    // Ciclo infinito para controlar los LEDs
    while (1) {
        // Generar secuencia aleatoria
        for (int i = 0; i < nivel; i++) {
            secuencia[i] = rand() % 4;  // Generar un número aleatorio entre 0 y 3
        }

        // Mostrar la secuencia al jugador
        for (int i = 0; i < nivel; i++) {
            encender_led(secuencia[i], tiempo_encendido);  // Encender LED con retardo variable
            _delay_ms(500);  // Pausa entre LEDs
        }	   
	   
	   
	   
	   
	 // Leer el estado de los botones en PB4, PB5, PB6, PB7
        if (PINB & (1 << PB4)) {  // Si el botón en PB4 está presionado
            PORTB |= (1 << PB0);  // Encender LED en PB0
        } else {
            PORTB &= ~(1 << PB0);  // Apagar LED en PB0
        }

        if (PINB & (1 << PB5)) {  // Si el botón en PB5 está presionado
            PORTB |= (1 << PB1);  // Encender LED en PB1
        } else {
            PORTB &= ~(1 << PB1);  // Apagar LED en PB1
        }

        if (PINB & (1 << PB6)) {  // Si el botón en PB6 está presionado
            PORTB |= (1 << PB2);  // Encender LED en PB2
        } else {
            PORTB &= ~(1 << PB2);  // Apagar LED en PB2
        }

        if (PINB & (1 << PB7)) {  // Si el botón en PB7 está presionado
            PORTB |= (1 << PB3);  // Encender LED en PB3
        } else {
            PORTB &= ~(1 << PB3);  // Apagar LED en PB3
        }

        _delay_ms(50);  // Pequeño retardo para evitar rebotes mecánicos de los botones
    }
}

