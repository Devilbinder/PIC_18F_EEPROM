#include <xc.h>
#include <p18f4520.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "conbits.h"
#include "uart_layer.h"

uint8_t data = 0;
bool got_data_bool = false;
uint8_t print_buffer[256] = {0};

uint8_t run = 1;
uint8_t led_flash = 0;


uint8_t Eeprom_read(uint8_t addr){
    EEADR = addr;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    while(EECON1bits.RD);
    return EEDATA;
}

void Eeprom_write(uint8_t addr,uint8_t data){
    EEADR = addr;
    EEDATA = data;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    INTCONbits.GIEH = 0; 
    
    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;
    while(EECON1bits.WR);
    EECON1bits.WREN = 0;
    INTCONbits.GIEH = 1;
    
     
}

void interrupt high_isr(void);
void interrupt low_priority low_isr(void);

void main(void) {
    ADCON1bits.PCFG = 0x0E;
    OSCCONbits.IRCF = 0x07;
    OSCCONbits.SCS = 0x03;
    while(OSCCONbits.IOFS!=1);
    
    TRISB=0;
    LATB=0;
    uart_init(51,0,1,0);//baud 9600
    led_flash = Eeprom_read(0x00);
    sprintf(print_buffer,"\n\rNumber of Blinks: %c,%d\n\r",led_flash,led_flash);
    uart_send_string(print_buffer);
    
    led_flash = led_flash - '0';
    
    if(led_flash != 0){
        LATB = run;
        __delay_ms(200);
        if((led_flash < 10) && (led_flash >= 0)){
            for(uint8_t i = 0;i < (led_flash-1);i++){
                if(run >= 0x80){
                    run = 1;
                }else{
                    run *= 2;
                }
                LATB |= run;
                __delay_ms(200);
            }
        }else{
            sprintf(print_buffer,"EEPROM read out of range\n\r");
            uart_send_string(print_buffer);
        }
    }
    
   
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1; 
    INTCONbits.GIEL = 1;
    
    sprintf(print_buffer,"Save to EEPROM: ");
    uart_send_string(print_buffer);
    
    while(1){
        
        if(got_data_bool){
            if(data == '\r'){
                Reset();
            }
            sprintf(print_buffer,"\rSave to EEPROM: %c",data);
            uart_send_string(print_buffer);
            
            Eeprom_write(0x00,data);
            got_data_bool = false;
        } 
    } 
}

void interrupt high_isr(void){
    INTCONbits.GIEH = 0;
    if(PIR1bits.RCIF){
        uart_receiver(&data,&got_data_bool);
       PIR1bits.RCIF=0;
    }
    
    INTCONbits.GIEH = 1;
}

void interrupt low_priority low_isr(void){
    INTCONbits.GIEH = 0;
    
    INTCONbits.GIEH = 1;
}



