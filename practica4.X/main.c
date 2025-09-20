#include "mcc_generated_files/system/system.h"

#define _XTAL_FREQ 16000000

void main(void) {
    SYSTEM_Initialize();

    // Asegurarse de que PORTB esté configurado correctamente
    ANSELB = 0x00;  // Digital
    TRISB  = 0x00;  // Salida
    LATB   = 0x00;  // Inicializa apagado

    while (1) {
        LATB = 0xFF;          // Enciende los 8 LEDs
        __delay_ms(1000);     // Espera 1 segundo

        LATB = 0x00;          // Apaga los 8 LEDs
        __delay_ms(1000);     // Espera 1 segundo
    }
}
