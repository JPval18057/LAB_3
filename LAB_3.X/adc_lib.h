/*
 LIBRERÍA PARA USAR EL ADC
 * AUTOR: JUAN PABLO VALENZUELA
 * Descripción: Librería con las configuraciones del modulo
 */



void config_adc(void){
    ADCON0 = 0b10000001; //inicialmente se configuro el canal 0
    //para configurar el canal se modifican los bits 5-2 de adcon0
    //CANAL 0 = 0000
    //CANAL 1 = 0001
    //Datos justificados a la izquierda
    ADCON1bits.ADFM = 0;
    //Valores VCC y GND de referencia
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0; //Recordatorio esperar 10us antes de iniciar conversions
    return;
}
