#define F_CPU 16500000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

// Definición de estados
#define ESTADO_INICIO 0
#define ESTADO_PARPADEO 1
#define ESTADO_SECUENCIA 2
#define ESTADO_LECTURA 3

volatile uint8_t estado = ESTADO_INICIO;
volatile uint8_t secuencia[4];       // Secuencia generada aleatoriamente
volatile uint8_t secuencia_usuario[4]; // Secuencia ingresada por el usuario
volatile uint8_t numparpa = 5;       // Número de parpadeos, valor inicial 5
volatile uint8_t indice_lectura = 0; // Índice para la secuencia del usuario

void inicializar() {
    // Configuración de pines
    DDRB = 0x0F;   // PB0, PB1, PB2 y PB3 como salida (LEDs)
    DDRD &= ~(0x0F); // PD0 a PD3 como entradas (botones)
    PORTD |= 0x0F; // Activar pull-up en PD0 a PD3

    // Configuración de temporizador
    TCCR0A = (1 << WGM01); // Modo CTC
    TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler 1024
    OCR0A = 156; // Valor de comparación para 10ms con prescaler 1024
    TIMSK = (1 << OCIE0A); // Habilitar interrupción por comparación
    
    // Habilitar interrupciones globales
    sei();
}

void generar_secuencia() {
    for (int i = 0; i < 4; i++) {
        secuencia[i] = rand() % 4; // Generar número aleatorio entre 0 y 3
    }
}

void parpadear_leds_simultaneo() {
    for (uint8_t i = 0; i < numparpa; i++) { // Parpadear según valor de numparpa
        PORTB = 0x0F; // Encender todos los LEDs
        _delay_ms(500); // Esperar
        PORTB = 0x00; // Apagar todos los LEDs
        _delay_ms(500); // Esperar
    }
}

void mostrar_secuencia() {
    for (int i = 0; i < 4; i++) {
        PORTB = (1 << secuencia[i]); // Encender el LED correspondiente
        _delay_ms(500); // Esperar
        PORTB = 0x00; // Apagar LEDs
        _delay_ms(250); // Esperar
    }
}

uint8_t leer_boton() {
    if (!(PIND & (1 << PD0))) return 0;
    if (!(PIND & (1 << PD1))) return 1;
    if (!(PIND & (1 << PD2))) return 2;
    if (!(PIND & (1 << PD3))) return 3;
    return 255; // Ningún botón presionado
}

void leer_secuencia_usuario() {
    uint8_t boton = leer_boton();
    if (boton != 255 && indice_lectura < 4) {
        secuencia_usuario[indice_lectura] = boton;
        indice_lectura++;
        _delay_ms(300); // Delay para evitar rebotes
    }
}

ISR(TIMER0_COMPA_vect) {
    if (estado == ESTADO_PARPADEO) {
        parpadear_leds_simultaneo();
        estado = ESTADO_SECUENCIA;
    }
}

int boton_presionado() {
    // Retorna 1 si alguno de los botones PD0 a PD3 es presionado
    if (!(PIND & (1 << PD0)) || !(PIND & (1 << PD1)) || !(PIND & (1 << PD2)) || !(PIND & (1 << PD3))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PD0)) || !(PIND & (1 << PD1)) || !(PIND & (1 << PD2)) || !(PIND & (1 << PD3))) {
            return 1; // Botón presionado
        }
    }
    return 0; // Ningún botón presionado
}

int main(void) {
    inicializar();
    
    while (1) {
        switch (estado) {
            case ESTADO_INICIO:
                if (boton_presionado()) {
                    while (boton_presionado()); // Esperar a que se suelte el botón
                    estado = ESTADO_PARPADEO;
                    generar_secuencia();
                }
                break;

            case ESTADO_PARPADEO:
                // El parpadeo se maneja en la interrupción TIMER0_COMPA_vect
                break;

            case ESTADO_SECUENCIA:
                mostrar_secuencia();
                estado = ESTADO_LECTURA; // Cambiar al estado de lectura de la secuencia
                indice_lectura = 0; // Reiniciar el índice de lectura
                break;

            case ESTADO_LECTURA:
                leer_secuencia_usuario(); // Leer la secuencia ingresada por el usuario
                if (indice_lectura >= 4) {
                    estado = ESTADO_INICIO; // Volver al estado inicial cuando se ingresan los 4 valores
                }
                break;

            default:
                estado = ESTADO_INICIO;
                break;
        }
    }
}
