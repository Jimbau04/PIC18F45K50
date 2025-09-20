/*
 * Proyecto: Demostración de Salida de Bits
 * Microcontrolador: PIC18F45k50 
 * IDE: MPLAB X IDE
 * Compilador: XC8
 * 
 * Descripción: Muestra diferentes patrones de salida de bits
 * en el Puerto B para controlar LEDs individuales
 */

// Configuración del microcontrolador
         // PORTB<4:0> pines como I/O digitales

#include <xc.h>
#include <stdint.h>

// Definición de frecuencia para delays
#define _XTAL_FREQ 8000000  // 8MHz oscilador interno

// Variables globales
uint8_t patron_leds = 0;
uint8_t contador = 0;

// Función para inicializar el microcontrolador
void init_sistema(void) {
    // Configurar oscilador interno a 8MHz
    OSCCON = 0b01110000;  // 8MHz
    
    // Configurar Puerto B como salidas digitales
    TRISB = 0x00;         // Todos los pines como salida
    ADCON1 = 0x0F;        // Todos los pines digitales (no analógicos)
    PORTB = 0x00;         // Inicializar todas las salidas en LOW
    
    // Configurar Puerto D para display adicional (opcional)
    TRISD = 0x00;         // Puerto D como salida
    PORTD = 0x00;         // Inicializar en LOW
}

// Función para mostrar patrón básico de bits
void patron_secuencial(void) {
    // Encender LEDs uno por uno (bit por bit)
    for(uint8_t i = 0; i < 8; i++) {
        PORTB = (1 << i);    // Encender solo el bit i
        __delay_ms(1000);      // Espera 300ms
    }
    
    // Apagar todos
    PORTB = 0x00;
    __delay_ms(500);
}

// Función para patrón de corrimiento
void patron_corrimiento(void) {
    patron_leds = 0x01;  // Empezar con el primer bit
    
    for(uint8_t i = 0; i < 8; i++) {
        PORTB = patron_leds;
        patron_leds <<= 1;    // Correr hacia la izquierda
        __delay_ms(1000);
    }
    
    // Corrimiento hacia la derecha
    patron_leds = 0x80;  // Empezar con el último bit
    
    for(uint8_t i = 0; i < 8; i++) {
        PORTB = patron_leds;
        patron_leds >>= 1;    // Correr hacia la derecha
        __delay_ms(1000);
    }
}






// Función principal
void main(void) {
    // Inicializar el sistema
    init_sistema();
    
    // Patrón de inicio: parpadeo rápido para indicar que está funcionando
    for(uint8_t i = 0; i < 6; i++) {
        PORTB = 0xFF;    // Todo encendido
        __delay_ms(100);
        PORTB = 0x00;    // Todo apagado
        __delay_ms(100);
    }
    
    __delay_ms(1000);    // Pausa antes de empezar
    
    // Bucle principal con diferentes patrones
    while(1) {
        // Patrón 1: Secuencial
        patron_secuencial();
        __delay_ms(500);
        
        // Patrón 2: Corrimiento
        patron_corrimiento();
        __delay_ms(500);
        
        
        // Separador entre ciclos completos
        for(uint8_t i = 0; i < 3; i++) {
            PORTB = 0xFF;
            __delay_ms(200);
            PORTB = 0x00;
            __delay_ms(200);
        }
        __delay_ms(2000);  // Pausa larga entre ciclos
    }
}

/*
 * NOTAS PARA MPLAB X IDE:
 * 
 * 1. Crear nuevo proyecto:
 *    - File > New Project > Microchip Embedded > Standalone Project
 *    - Seleccionar PIC18F4550 o compatible
 *    - Seleccionar XC8 compiler
 * 
 * 2. Conexiones de hardware sugeridas:
 *    - RB0-RB7 conectados a LEDs con resistencias de 220Ω a GND
 *    - Fuente de 5V para VDD y VSS a tierra
 * 
 * 3. Para depurar:
 *    - Usar MPLAB Simulator o hardware debugger
 *    - Observar el valor de PORTB en la ventana de registros
 * 
 * 4. Modificaciones posibles:
 *    - Cambiar _XTAL_FREQ según tu cristal
 *    - Ajustar delays según necesidades
 *    - Cambiar puerto de salida si usas diferente al B
 */