/*
 * File:   main.c
 * Author: JUAN PABLO VALENZUELA
 *
 * Created on 5 de febrero de 2021, 08:31 AM
 * Description: Laboratorio de electrónica digital II manejo de LCDs
 */
#define _XTAL_FREQ 8000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#include <xc.h>
#include "lcd.h"
#include <stdio.h> //se necesita para usar sprintf y formatear strings
#include "eusart_lib.h"
#include "adc_lib.h"

// CONFIGURATION BITS
// ESTOY USANDO UN CRISTAL EXTERNO Y TENGO ACTIVADO EL MASTER CLEAR RESET
#pragma config FOSC = EXTRC_CLKOUT// Oscillator Selection bits (RC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF // Flash Program Memory Code Protection bit (Code protection off)
//END CONFIG

//Prototipos de funciones
void mostrar_datos(void);
void leer_datos(void);
void count_update(void);

//Variables
unsigned char count=0;
unsigned int a; //es necesaria para que la libreria funciones
float f = 0;
char s[20]; //empty array
char pot_select = 1; //selecciona cual potenciometro leer
unsigned int pot1 = 15; //voltaje pot1 
unsigned int pot2 = 250; //voltaje pot2
unsigned int receive = 0; //almacena el valor que entra al pic
int pot1_string;
int pot2_string;

//INTERRUPCIONES
void __interrupt() ISR(void) {
    //la interrupcion del EUSART va primero
    if (PIR1bits.RCIF==1){
        PIR1bits.RCIF = 0;
        //Si se desea poner un detector de bits de error para reiniciar si hay error
        //se recupera el dato
        receive = RCREG;
    }
    //primero leemos el pot1 y luego el pot2    
    if (PIR1bits.ADIF==1){
        PIR1bits.ADIF = 0; //reiniciar la bandera
        if (pot_select==3){
            pot1 = ADRESH;
            pot_select=2;
            pot1 = pot1<<1;
        }
        if (pot_select==4){
            pot2 = ADRESH;
            pot_select=1;
            pot2 = pot2<<1;
        }
    }
    
}



//CONFIGURACION GENERAL DEL PIC
void setup(void){
    //CONFIGURACION DE I/O
    TRISD = 0x00; //El puerto D es el puerto que controla la pantalla (salida)
    TRISA = 0x03; //CONFIGURAMOS RA0 Y RA1 COMO ENTRADAS ANALOGICAS
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 1;
    ANSEL = 0X03;
    ANSELH = 0;
    
    //CONFIGURACION DE PANTALLA
    Lcd_Init(); //Inicializamos la pantalla
        
    
    //CONFIGURACION DE ADC
    config_adc();
    
    
    //CONFIGURACION DE EUSART
    eusart_config();
    
    
    //CONFIGURACION DE INTERRUPCIONES
    ei();//activamos global interrupt enable
    //Interrupciones perifericas
    INTCONbits.PEIE = 1;//activamos las interrupciones perifericas
    //interrupción del ADC
    PIE1bits.ADIE = 1; //activar la interrupción del adc
    PIR1bits.ADIF = 0;//apagamos la bandera
    //interrupcion de eusart
    PIE1bits.RCIE = 1;//activamos la interrupción de recepción
    PIR1bits.RCIF = 0;//APAGAMOS LA BANDERA
    PIE1bits.TXIE = 0; //NO DESEAMOS INTERRUPCIONES DE TRANSMISION
}

//CODIGO PRINCIPAL
void main(void)
{    
    setup();
    while(1)
    {
        leer_datos(); //lee los datos del adc
        Lcd_Clear(); //limpia la pantalla
        Lcd_Set_Cursor(1,1); //poner el cursor en fila 1 caracter 1
        Lcd_Write_String("POT_1 POT_2 CONT");
        mostrar_datos(); //mostrar los datos del adc ordenados
        count_update(); //revisar si entró algún comando
        __delay_ms(500); //para que se aprecie en la simulación        

    }
    return;
}

void mostrar_datos(void){    
    sprintf(s,"%u",pot1);
    Lcd_Set_Cursor(2,2);
    Lcd_Write_Char(s[0]);
    Lcd_Write_Char('.');
    Lcd_Write_Char(s[1]);
    //se envian los valores de voltaje por comunicación serial
    envio_char(s[0]);
    envio_char(0x56); //V
    //**************************************************************************
    sprintf(s,"%u",pot2); //el contador es únicamente positivo, contador de 8 bits
    Lcd_Set_Cursor(2,8);
    Lcd_Write_Char(s[0]);
    Lcd_Write_Char(0x2e);
    Lcd_Write_Char(s[1]);
    //se envian los valores de voltaje por comunicación serial
    envio_char(s[0]);
    envio_char(0x56); //V
    //**************************************************************************
    Lcd_Set_Cursor(2,13); //escribimos el valor del contador
    sprintf(s,"%u",count);
    Lcd_Write_String(s);
    return;
}

void leer_datos(void){
    if (pot_select==1){
        //ponemos el canal 0
        ADCON0bits.ADON = 0;
        ADCON0bits.CHS0 = 0;
        ADCON0bits.ADON = 1;
        pot_select=3;
    }
    if (pot_select==2){
        //PONEMOS EL CANAL 1
        ADCON0bits.ADON = 0;
        ADCON0bits.CHS0 = 1;
        ADCON0bits.ADON = 1;
        pot_select=4;
    }
    if (ADCON0bits.GO_DONE==0){
        __delay_us(20); //se debe esperar al menos 5us segun el datasheet
        ADCON0bits.GO_DONE = 1; //inicia la conversión
    }
    
    return;
}

void count_update(void){
    if (receive=='+'){ //signo +
            count++; //solo lo ejecuta 1 vez
            receive = 0;
        }
    if (receive==0x2D){ //signo -
        count--;
        receive = 0; //solo lo ejecuta 1 vez
    }
    //ENVIO DE DATOS
    envio_char('C');
    envio_char(count);
    if (RCSTAbits.OERR==1){
        //si se ejecuta esto es que hay un error y simplemente se reinicia
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
    }
    return;
}