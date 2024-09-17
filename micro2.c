#define F_CPU 16500000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h> // Para _delay_ms()

// Prototipos de funciones
void parpadear_leds_simultaneo(void);
void generar_secuencia(void);
void mostrar_secuencia(void);
void leer_secuencia(void);
void comparar_secuencias(void);
void manejar_exito(void);
void manejar_error(void);
int boton_presionado(void);

// Definición de estados
#define ESTADO_INICIO 0
#define ESTADO_PARPADEO 1
#define ESTADO_SECUENCIA 2
#define ESTADO_LECTURA 3
#define ESTADO_EXITO 4
#define ESTADO_ERROR 5

volatile uint8_t estado = ESTADO_INICIO;
volatile uint8_t secuencia[8];
volatile uint8_t lectura[8];
volatile uint8_t longitudSecuencia = 4;
volatile uint16_t tiempoLedEncendido = 2000; // 2 segundos al inicio

volatile uint16_t temporizadorLeds = 0; // Tiempo para encender los LEDs
volatile uint8_t parpadeo = 0; // Flag para controlar el parpadeo de LEDs

void inicializar() {
    // Configuración de pines
    DDRB = 0x0F; // PB0, PB1, PB2 y PB3 como salida (LEDs)
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
    for (uint8_t i = 0; i < longitudSecuencia; i++) {
        secuencia[i] = rand() % 4;
    }
}

void mostrar_secuencia() {
    for (uint8_t i = 0; i < longitudSecuencia; i++) {
        PORTB = (1 << secuencia[i]); // Encender el LED correspondiente
        temporizadorLeds = tiempoLedEncendido / 10; // Tiempo de encendido en 10 ms
        parpadeo = 1; // Activar parpadeo
        
        // Esperar el tiempo de encendido
        while (temporizadorLeds > 0); 
        
        PORTB = 0x00; // Apagar LEDs
        
        temporizadorLeds = 250 / 10; // Tiempo de espera entre LEDs en 10 ms
        parpadeo = 1; // Activar parpadeo
        
        // Esperar el tiempo de espera entre LEDs
        while (temporizadorLeds > 0); 
    }
}

void leer_secuencia() {
    for (uint8_t i = 0; i < longitudSecuencia; i++) {
        while (!boton_presionado()); // Esperar a que se presione un botón

        for (uint8_t j = 0; j < 4; j++) {
            if (!(PIND & (1 << j))) {
                lectura[i] = j;
                break;
            }
        }

        _delay_ms(200); // Debounce
    }
}

void comparar_secuencias() {
    for (uint8_t i = 0; i < longitudSecuencia; i++) {
        if (lectura[i] != secuencia[i]) {
            estado = ESTADO_ERROR;
            return;
        }
    }
    estado = ESTADO_EXITO;
}

void manejar_exito() {
    for (uint8_t i = 0; i < 2; i++) {
        PORTB = 0x0F; // Encender todos los LEDs
        _delay_ms(500); // Esperar
        PORTB = 0x00; // Apagar todos los LEDs
        _delay_ms(500); // Esperar
    }

    if (longitudSecuencia < 8) {
        longitudSecuencia++;
        if (tiempoLedEncendido > 200) {
            tiempoLedEncendido -= 200;
        }
    }
}

void manejar_error() {
    for (uint8_t i = 0; i < 3; i++) {
        PORTB = 0x0F; // Encender todos los LEDs
        _delay_ms(500); // Esperar
        PORTB = 0x00; // Apagar todos los LEDs
        _delay_ms(500); // Esperar
    }

    longitudSecuencia = 4;
    tiempoLedEncendido = 2000; // Reiniciar tiempo de encendido a 2 segundos
}

ISR(TIMER0_COMPA_vect) {
    if (parpadeo && temporizadorLeds > 0) {
        temporizadorLeds--; // Contar el tiempo en 10ms
    }
}

int boton_presionado() {
    if (!(PIND & (1 << PD0)) || !(PIND & (1 << PD1)) || !(PIND & (1 << PD2)) || !(PIND & (1 << PD3))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PD0)) || !(PIND & (1 << PD1)) || !(PIND & (1 << PD2)) || !(PIND & (1 << PD3))) {
            return 1; // Botón presionado
        }
    }
    return 0; // Ningún botón presionado
}

void parpadear_leds_simultaneo(void) {
    // Función para parpadear LEDs simultáneamente
    PORTB = 0x0F; // Encender todos los LEDs
    _delay_ms(500); // Esperar
    PORTB = 0x00; // Apagar todos los LEDs
    _delay_ms(500); // Esperar
    PORTB = 0x0F; // Encender todos los LEDs
    _delay_ms(500); // Esperar
    PORTB = 0x00; // Apagar todos los LEDs
    _delay_ms(500); // Esperar
}

void maquina_estados() {
    switch (estado) {
        case ESTADO_INICIO:
            if (boton_presionado()) {
                while (boton_presionado()); // Esperar a que se suelte el botón
                estado = ESTADO_PARPADEO;
            }
            break;

        case ESTADO_PARPADEO:
            parpadear_leds_simultaneo();
            estado = ESTADO_SECUENCIA;
            break;

        case ESTADO_SECUENCIA:
            generar_secuencia();
            _delay_ms(1000); // Esperar antes de mostrar la secuencia
            mostrar_secuencia();
            estado = ESTADO_LECTURA;
            break;

        case ESTADO_LECTURA:
            leer_secuencia();
            comparar_secuencias();
            break;

        case ESTADO_EXITO:
            manejar_exito();
            estado = ESTADO_INICIO; // Volver al estado inicial
            break;

        case ESTADO_ERROR:
            manejar_error();
            estado = ESTADO_INICIO; // Volver al estado inicial
            break;

        default:
            estado = ESTADO_INICIO; // En caso de estado desconocido
            break;
    }
}

int main(void) {
    inicializar();

    while (1) {
        maquina_estados(); // Controlar el flujo de la máquina de estados
    }
}
