#include "mcc_generated_files/system/system.h"

#define _XTAL_FREQ 16000000

void main(void) {
    SYSTEM_Initialize();

    ANSELB = 0x00;  // Fuerza digital
    TRISB  = 0x00;  // Todo PORTB como salida
    LATB   = 0x00;  // Inicializa apagado

    uint8_t value = 0;

    while (1) {
        // Mostrar los 7 bits en RB0-RB6
        LATB = (value & 0x7F); // RB7 puede quedar apagado o usarlo para otro fin

        __delay_ms(200); // retardo visible

        // Incrementa valor de 0 a 127
        value++;
        if (value > 127) {
            value = 0;
        }
    }
}
