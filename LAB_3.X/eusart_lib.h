/*
 * Librería EUSART
 * AUTOR: JUAN PABLO VALENZUELA
 * DESCRIPCION:
 * ES LA LIBRERIA PARA CONTORLAR FACILMENTE EL ADC EN ESTE LABORATORIO
 * INCLUYE CONFIGURACION Y LECTURA DE 2 CANALES 
 */



void recepcion(void){
    //CONFIGURACION DE EUSART
    //RECEPCION
    RCSTAbits.CREN = 1; //encendemos el módulo de recepción
    TXSTAbits.SYNC = 0; //modo asincrono
    RCSTAbits.SPEN = 1; //configura los pines rx y tx para operacion
    RCSTAbits.RX9 = 0; //trabajamos con 8 bits de recepción
    return;
}

void transmision(void){
    //MODULO DE TRANSMISION
    TXSTAbits.TXEN = 1; //SE ENCIENDE EL TRANSMISOR
    TXSTAbits.TX9 = 0; //transmision de 8 bits
    return;
}

void baud_rate_config(void){
    //CONFIGURAR BAUD RATE ASINCRONA
    //BRGH = 1; BRGH16=0
    SPBRG = 12;
    TXSTAbits.BRGH=0;
    BAUDCTLbits.BRG16 =0;
    //Baud rate de 10417 con 0% error 9600
    return;
}

void eusart_config(void){
    recepcion();
    transmision();
    baud_rate_config();
    return;
}

//Función que envía caracteres
char envio_char(char u){
    //ENVIO DE DATOS
    PIR1bits.TXIF = 0;
    TXREG = u;
    return 0;
}
