/*
 * Detector de Color con 1 LDR y LEDs RGB
 * PIC18F45K50 - MPLAB XC8
 * 
 * Principio de funcionamiento:
 * 1. Iluminar objeto con LED ROJO y medir reflexión con LDR
 * 2. Iluminar objeto con LED VERDE y medir reflexión con LDR
 * 3. Iluminar objeto con LED AZUL y medir reflexión con LDR
 * 4. Comparar valores y determinar color predominante
 * 5. Encender LED del color detectado
 * 
 * Conexiones:
 * LDR        -> AN0 (RA0) con resistencia pull-down 10kΩ
 * LED Rojo   -> RC0 (para iluminación)
 * LED Verde  -> RC1 (para iluminación) 
 * LED Azul   -> RC2 (para iluminación)
 * 
 * LEDs Indicadores (Puerto B completo):
 * RB0 -> LED Rojo     (Color primario)
 * RB1 -> LED Verde    (Color primario)
 * RB2 -> LED Azul     (Color primario)
 * RB3 -> LED Amarillo (Color secundario)
 * RB4 -> LED Cyan     (Color secundario)
 * RB5 -> LED Magenta  (Color secundario)
 * RB6 -> LED Blanco   (Todos los colores)
 * RB7 -> LED Negro    (Sin reflexión)
 */

#include "mcc_generated_files/system/system.h"
#include <xc.h>

// Definiciones para LEDs de iluminación
#define LED_ILUM_ROJO    LATCbits.LATC0
#define LED_ILUM_VERDE   LATCbits.LATC1  
#define LED_ILUM_AZUL    LATCbits.LATC2

// Definiciones para LEDs indicadores (resultado)
#define LED_IND_ROJO     LATBbits.LATB0
#define LED_IND_VERDE    LATBbits.LATB1
#define LED_IND_AZUL     LATBbits.LATB2
#define LED_IND_AMARILLO LATBbits.LATB3
#define LED_IND_CYAN     LATBbits.LATB4
#define LED_IND_MAGENTA  LATBbits.LATB5
#define LED_IND_BLANCO   LATBbits.LATB6
#define LED_IND_NEGRO    LATBbits.LATB7

// Canal ADC para el LDR
#define CANAL_LDR  0

// Parámetros de medición
#define TIEMPO_ESTABILIZACION 50  // ms para estabilizar iluminación
#define NUM_MUESTRAS 10           // Número de muestras para promedio
#define UMBRAL_MINIMO 50          // Umbral mínimo de reflexión
#define DIFERENCIA_MINIMA 30      // Diferencia mínima para detectar color

// Variables globales
uint16_t reflexion_roja, reflexion_verde, reflexion_azul;
uint16_t reflexion_ambiente;
uint8_t color_detectado;

// Enumeración para colores
typedef enum {
    COLOR_INDEFINIDO = 0,
    COLOR_ROJO,
    COLOR_VERDE, 
    COLOR_AZUL,
    COLOR_AMARILLO,   // Refleja bien rojo y verde
    COLOR_CYAN,       // Refleja bien verde y azul
    COLOR_MAGENTA,    // Refleja bien rojo y azul
    COLOR_BLANCO,     // Refleja bien todos los colores
    COLOR_NEGRO       // Refleja poco todos los colores
} color_t;

// Prototipos de funciones
void inicializar_sistema(void);
uint16_t leer_adc_promedio(uint8_t canal, uint8_t muestras);
void medir_reflexiones(void);
color_t detectar_color(void);
void mostrar_resultado(color_t color);
void apagar_todos_leds(void);
void secuencia_inicio(void);
void delay_ms(uint16_t ms);

void main(void) {
    // Inicialización del sistema MCC
    SYSTEM_Initialize();
    ANSELB = 0x00; // Desactiva modo analógico en todo PORTB
    TRISB = 0x00;  // 0 = salida, 1 = entrada
    LATB = 0x00;
    TRISBbits.TRISB0 = 0; // RB0 como salida
    while(1) {
        LATBbits.LATB0 = 1; // Encender LED
        __delay_ms(500);
        LATBbits.LATB0 = 0; // Apagar LED
        __delay_ms(500);
    }
    
    // Inicialización adicional
    inicializar_sistema();
    
    // Secuencia de inicio
    secuencia_inicio();
    
    // Bucle principal
    while(1) {
        // Medir reflexiones con cada color
        medir_reflexiones();
        
        // Detectar color basado en las reflexiones
        color_detectado = detectar_color();
        
        // Mostrar resultado
        mostrar_resultado(color_detectado);
        
        // Pausa entre mediciones
        delay_ms(1000);
    }
}

void inicializar_sistema(void) {
    // Configurar pines de LEDs de iluminación como salidas
    TRISCbits.TRISC0 = 0; // LED iluminación Rojo
    TRISCbits.TRISC1 = 0; // LED iluminación Verde
    TRISCbits.TRISC2 = 0; // LED iluminación Azul
    
    // Configurar pines de LEDs indicadores como salidas
    TRISBbits.TRISB0 = 0; // LED indicador Rojo
    TRISBbits.TRISB1 = 0; // LED indicador Verde
    TRISBbits.TRISB2 = 0; // LED indicador Azul
    
    // Configurar pin analógico para LDR
    TRISAbits.TRISA0 = 1; // LDR
    
    // Apagar todos los LEDs inicialmente
    apagar_todos_leds();
}

uint16_t leer_adc_promedio(uint8_t canal, uint8_t muestras) {
    uint32_t suma = 0;
    
    // Seleccionar canal ADC
    ADCON0bits.CHS = canal;
    
    for(uint8_t i = 0; i < muestras; i++) {
        // Tiempo de adquisición
        __delay_us(20);
        
        // Iniciar conversión
        ADCON0bits.GO_nDONE = 1;
        
        // Esperar a que termine la conversión
        while(ADCON0bits.GO_nDONE);
        
        // Acumular resultado
        suma += ((ADRESH << 8) + ADRESL);
        
        // Pequeña pausa entre muestras
        __delay_us(100);
    }
    
    return (uint16_t)(suma / muestras);
}

void medir_reflexiones(void) {
    // Primero medir luz ambiente (sin iluminación)
    apagar_todos_leds();
    delay_ms(TIEMPO_ESTABILIZACION);
    reflexion_ambiente = leer_adc_promedio(CANAL_LDR, NUM_MUESTRAS);
    
    // Medir reflexión con luz roja
    LED_ILUM_ROJO = 1;
    delay_ms(TIEMPO_ESTABILIZACION);
    reflexion_roja = leer_adc_promedio(CANAL_LDR, NUM_MUESTRAS);
    LED_ILUM_ROJO = 0;
    
    // Medir reflexión con luz verde
    LED_ILUM_VERDE = 1;
    delay_ms(TIEMPO_ESTABILIZACION);
    reflexion_verde = leer_adc_promedio(CANAL_LDR, NUM_MUESTRAS);
    LED_ILUM_VERDE = 0;
    
    // Medir reflexión con luz azul
    LED_ILUM_AZUL = 1;
    delay_ms(TIEMPO_ESTABILIZACION);
    reflexion_azul = leer_adc_promedio(CANAL_LDR, NUM_MUESTRAS);
    LED_ILUM_AZUL = 0;
    
    // Compensar luz ambiente
    if(reflexion_roja > reflexion_ambiente) {
        reflexion_roja -= reflexion_ambiente;
    } else {
        reflexion_roja = 0;
    }
    
    if(reflexion_verde > reflexion_ambiente) {
        reflexion_verde -= reflexion_ambiente;
    } else {
        reflexion_verde = 0;
    }
    
    if(reflexion_azul > reflexion_ambiente) {
        reflexion_azul -= reflexion_ambiente;
    } else {
        reflexion_azul = 0;
    }
}

color_t detectar_color(void) {
    uint16_t total_reflexion = reflexion_roja + reflexion_verde + reflexion_azul;
    
    // Si la reflexión total es muy baja, es negro
    if(total_reflexion < UMBRAL_MINIMO) {
        return COLOR_NEGRO;
    }
    
    // Si todas las reflexiones son altas y similares, es blanco
    if(reflexion_roja > (UMBRAL_MINIMO * 2) && 
       reflexion_verde > (UMBRAL_MINIMO * 2) && 
       reflexion_azul > (UMBRAL_MINIMO * 2)) {
        
        uint16_t max_dif = 0;
        if(reflexion_roja > reflexion_verde) {
            max_dif = reflexion_roja - reflexion_verde;
        } else {
            max_dif = reflexion_verde - reflexion_roja;
        }
        
        uint16_t dif_temp;
        if(reflexion_roja > reflexion_azul) {
            dif_temp = reflexion_roja - reflexion_azul;
        } else {
            dif_temp = reflexion_azul - reflexion_roja;
        }
        if(dif_temp > max_dif) max_dif = dif_temp;
        
        if(reflexion_verde > reflexion_azul) {
            dif_temp = reflexion_verde - reflexion_azul;
        } else {
            dif_temp = reflexion_azul - reflexion_verde;
        }
        if(dif_temp > max_dif) max_dif = dif_temp;
        
        if(max_dif < DIFERENCIA_MINIMA) {
            return COLOR_BLANCO;
        }
    }
    
    // Encontrar el color con mayor reflexión
    uint16_t max_reflexion = reflexion_roja;
    color_t color_principal = COLOR_ROJO;
    
    if(reflexion_verde > max_reflexion) {
        max_reflexion = reflexion_verde;
        color_principal = COLOR_VERDE;
    }
    
    if(reflexion_azul > max_reflexion) {
        max_reflexion = reflexion_azul;
        color_principal = COLOR_AZUL;
    }
    
    // Verificar si hay colores secundarios (dos componentes altos)
    uint8_t componentes_altos = 0;
    if(reflexion_roja > (max_reflexion * 0.7)) componentes_altos++;
    if(reflexion_verde > (max_reflexion * 0.7)) componentes_altos++;
    if(reflexion_azul > (max_reflexion * 0.7)) componentes_altos++;
    
    if(componentes_altos >= 2) {
        // Detectar colores secundarios
        if(reflexion_roja > (max_reflexion * 0.7) && reflexion_verde > (max_reflexion * 0.7)) {
            return COLOR_AMARILLO;
        }
        if(reflexion_verde > (max_reflexion * 0.7) && reflexion_azul > (max_reflexion * 0.7)) {
            return COLOR_CYAN;
        }
        if(reflexion_roja > (max_reflexion * 0.7) && reflexion_azul > (max_reflexion * 0.7)) {
            return COLOR_MAGENTA;
        }
    }
    
    // Retornar color principal si hay diferencia significativa
    if(max_reflexion > UMBRAL_MINIMO) {
        return color_principal;
    }
    
    return COLOR_INDEFINIDO;
}

void mostrar_resultado(color_t color) {
    // Apagar todos los LEDs indicadores
    apagar_todos_leds();
    
    // Encender solo el LED correspondiente
    switch(color) {
        case COLOR_ROJO:
            LED_IND_ROJO = 1;
            break;
            
        case COLOR_VERDE:
            LED_IND_VERDE = 1;
            break;
            
        case COLOR_AZUL:
            LED_IND_AZUL = 1;
            break;
            
        case COLOR_AMARILLO:
            LED_IND_AMARILLO = 1;
            break;
            
        case COLOR_CYAN:
            LED_IND_CYAN = 1;
            break;
            
        case COLOR_MAGENTA:
            LED_IND_MAGENTA = 1;
            break;
            
        case COLOR_BLANCO:
            LED_IND_BLANCO = 1;
            break;
            
        case COLOR_NEGRO:
            LED_IND_NEGRO = 1;
            break;
            
        default: // COLOR_INDEFINIDO
            // Parpadeo lento en todos los LEDs
            LATB = 0xFF;
            delay_ms(500);
            LATB = 0x00;
            delay_ms(500);
            break;
    }
}


void apagar_todos_leds(void) {
    // LEDs de iluminación
    LED_ILUM_ROJO = 0;
    LED_ILUM_VERDE = 0;
    LED_ILUM_AZUL = 0;
    
    // LEDs indicadores (RB0–RB7)
    LATB = 0x00;
}


void secuencia_inicio(void) {
    // Secuencia de prueba al inicio
    for(int i = 0; i < 2; i++) {
        LED_IND_ROJO = 1;
        delay_ms(200);
        LED_IND_ROJO = 0;
        
        LED_IND_VERDE = 1;
        delay_ms(200);
        LED_IND_VERDE = 0;
        
        LED_IND_AZUL = 1;
        delay_ms(200);
        LED_IND_AZUL = 0;
    }
    
    // Todos encendidos brevemente
    LED_IND_ROJO = LED_IND_VERDE = LED_IND_AZUL = 1;
    delay_ms(500);
    apagar_todos_leds();
    delay_ms(1000);
}

void delay_ms(uint16_t ms) {
    for(uint16_t i = 0; i < ms; i++) {
        __delay_ms(1);
    }
}

/*
 * NOTAS IMPORTANTES:
 * 
 * 1. El LDR debe colocarse cerca del objeto a medir
 * 2. Los LEDs de iluminación deben apuntar al objeto
 * 3. Evitar luz ambiente fuerte que interfiera
 * 4. El sistema funciona mejor en ambiente controlado
 * 5. Calibrar umbrales según tu setup específico
 * 
 * PRINCIPIO DE FUNCIONAMIENTO:
 * - Un objeto ROJO refleja más luz roja que verde o azul
 * - Un objeto VERDE refleja más luz verde que roja o azul
 * - Un objeto AZUL refleja más luz azul que roja o verde
 * - Un objeto BLANCO refleja bien todos los colores
 * - Un objeto NEGRO refleja poco todos los colores
 */