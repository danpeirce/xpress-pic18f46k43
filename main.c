/**
  File Name:
    main.c
**/
//	File has been modified from Code Configurator generated file by Dan Peirce B.Sc. Sept 4, 2019
//     File has undergone more revision to the point that the code here was
//     written by Dan Peirce B.Sc. (comment added June10, 2020)
/**
  Description:
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.77
        Device            :  PIC18F46K42
        Driver Version    :  2.00
*/

#include "mcc_generated_files/mcc.h"
#include <stdio.h>

unsigned int readbatteryvoltage(void);
unsigned int* readsensors(void);
void sendbatteryvoltage(void);
void send_hyphen(void);
void send_APSC1299(void);
void display_signature(void);
void LCD_print(char *str, char length);
void LCD_line2(void);
void sendchar(char a_char);
void calibrate(void);
void go_pd(void);
void stop_pd(void);
void print_sensors(void);


/*
                         Main application
 */
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();
   
    // the delay caused by these printf statements ensures 3Pi has time to
    // be ready for LCD commands
    {
        printf("\tKPU APSC1299\n\n\r");
        printf("\t\t  Menu\n\r");
        printf("\t\t--------\r\n");  
        printf("\t\t@. Pololu Signature?\r\n"); 
        printf("\t\t1. Display mV reading\r\n"); // sent to PuTTY only
        printf("\t\t2. Display mV reading in LCD\r\n");  // also send to LCD
        printf("\t\tc. Clear LCD\r\n");
        printf("\t\ts. Print Sensor Values\r\n");
        printf("\t\t-. Send hyphen to LCD\r\n");
        printf("\t\t~. LCD message APSC1299\r\n");
        printf("\t\treturn. LCD go to start of line two\r\n");
        printf("\t\t?. LCD display error and hex 3F\r\n");
        printf("\t\t' '. LCD display error and hex 20\r\n");
        printf("\t\t--------\r\n\n");
    }

    sendbatteryvoltage(); // sends battery voltage to both LCD and USB
    LCD_line2();
    if (roam_PORT) LCD_print("Roam", 4);
    else LCD_print("No Roam", 7);
    TMR1_StartTimer();  // short times and clock source for timer 3
    TMR2_StartTimer();  // PWM clock source (not used yet)
    TMR3_StartTimer();  // long times
    //printf("Timer Value = %u\r\n",TMR1_ReadTimer());
    //printf("Timer Value = %u\r\n",TMR1_ReadTimer());
    if (roam_PORT) 
    {
        unsigned int * sensorvalues;
        unsigned int time1, tmr3read;
        const unsigned int time1_inc = 57; // 57 for about 10 seconds
        calibrate();
        time1 = TMR3_ReadTimer()+time1_inc;
        go_pd();    // tell slave to start PID mode
        while(1)
        {
            tmr3read = TMR3_ReadTimer();
            if ((tmr3read>time1)&&((0xFFFF-tmr3read)>time1_inc))
            {
                stop_pd(); // tell slave to stop PID mode
                while(1);
            }
            sensorvalues = readsensors();
            if ((*sensorvalues > 250) && (*(sensorvalues+4)>250))
            {
                stop_pd(); // tell slave to stop PID mode
                while(1);
            }
        }
    }
    
    while (1)
    {
        char rxData;
            // Logic to echo received data
        test1_PORT = 1;
        //printf("Timer Value = %u\r\n",TMR1_ReadTimer());
        if(UART2_is_rx_ready())
        {
            test2_PORT = 1;
            rxData = UART2_Read();
            if(UART2_is_tx_ready()) // for USB echo
            {
                UART2_Write(rxData);
                if(rxData == '\r') UART2_Write('\n'); // add newline to return
            }
            // if(UART1_is_tx_ready()) // out RC6
            if (rxData == '1') readbatteryvoltage();   // read battery voltage 
                                                       //  and send to PuTTY
            else if (rxData == '2') sendbatteryvoltage();   // send battery voltage to LCD
                                                       //  and send to PuTTY
            else if (rxData == '@') display_signature();
            else if (rxData == 'c') UART1_Write(0xB7);      // clear LCD on 3Pi
            else if (rxData == 's') print_sensors();     // print values loop
            else if (rxData == '-') send_hyphen();     // send hyphen to LCD
            else if (rxData == '~') send_APSC1299();  // send APSC1299  msg to LCD
            else if (rxData == '\r') LCD_line2();     // move courser to start of line 2
            else if (rxData >= ' ') sendchar(rxData);       // send the character to the display

            test2_PORT = 0;
        }
        test1_PORT = 0; 
    }
}

void print_sensors(void)
{
    unsigned int * sensorvalues;
    calibrate();
    while(1)
    {
        sensorvalues = readsensors();
        // __delay_ms(80);
        printf("\rsensor values = %4u, ", *sensorvalues);
        printf("%4u, ", *(sensorvalues+1));
        printf("%4u, ", *(sensorvalues+2));
        printf("%4u, ", *(sensorvalues+3));
        printf("%4u", *(sensorvalues+4));
        printf(" | Timer Value = %5u",TMR3_ReadTimer());
    }
    
}

unsigned int readbatteryvoltage(void)
{
    unsigned char lbyte, ubyte;
    
    printf("\r\n\tBattery Voltage = ");
    
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xB1);
    while (!UART1_is_rx_ready()) continue;
    lbyte = UART1_Read();
    while (!UART1_is_rx_ready()) continue;
    ubyte = UART1_Read();
    printf("%d mV\r\n", ubyte*256 + lbyte);
    return (unsigned int)(ubyte*256 + lbyte);
}

unsigned int* readsensors(void)
{
    unsigned char lbyte[5], ubyte[5], i;
    static unsigned int values[5];
    
    // printf("\r\n\tSensor Readings =  ");
    
    while(!UART1_is_tx_ready()) continue;
    test2_PORT = 1;
    UART1_Write(0x87);
    for (i=0;i<5;i++)
    {
        while (!UART1_is_rx_ready()) continue;
        lbyte[i] = UART1_Read();
        while (!UART1_is_rx_ready()) continue;
        ubyte[i] = UART1_Read();
        values[i] = ubyte[i]*256 + lbyte[i];
    }
    test2_PORT = 0;

    return values;
}

// sends battery voltage to LCD
void sendbatteryvoltage(void)
{
    unsigned int voltage;
    char bat_volt[9];
    unsigned char msg_length, i=0;
    
    voltage = readbatteryvoltage();
    msg_length = sprintf(bat_volt, "%u mV", voltage);
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xB8);   // print LCD command to slave
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(msg_length);     // send eight characters
    while (i<msg_length)
    {
        if(UART1_is_tx_ready())
        {
            UART1_Write(bat_volt[i]);
            i++;
        }
    }
    
}

void sendchar(char a_char)
{
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xB8);   // print LCD command to slave
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(1);     // send one character
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(a_char);     // send one character
}

void LCD_print(char *str, char length)
{
    char i=0;
    
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xB8);   // print LCD command to slave
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(length);     // send eight characters
    while (i<length)
    {
        if(UART1_is_tx_ready())
        {
            UART1_Write(str[i]);
            i++;
        }
    }
}

void LCD_line2(void)
{
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xB9);   // goto LCD position
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0x00);   // column 0
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0x01);   // row 1
}

void display_signature(void)
{
    char signature[7], i = 0;
    int sig_length = 6;
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0x81);
    while (i < sig_length)
    {
        if (UART1_is_rx_ready())
        {
            signature[i] = UART1_Read();
            i++;
        }
    }
    signature[sig_length] = '\0';  // terminate string
    printf("\r\n\tThe Signature from 3Pi is: %s\r\n", signature);
    LCD_print(signature, sig_length);
}

// just to test that printing to the LCD is working
void send_hyphen(void)
{
    // see https://www.pololu.com/docs/0J21/10.a 
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xB8);   // print LCD command to slave
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(1);     // send one character
    while(!UART1_is_tx_ready()) continue;
    UART1_Write('-');   // send a hyphen
}

void send_APSC1299(void)
{
    char  msg_length=8; // to send 8 characters
    char msg[] = "APSC1299";
    LCD_print(msg, msg_length); 
}

void calibrate(void)
{
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xBA);   // autocalibration command to slave
    while(!UART1_is_rx_ready()) continue;
    while(UART1_Read() != 'c')
    {
        while(!UART1_is_rx_ready()) continue;
    }
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xB7);   // clear LCD
    LCD_print("Cal Done", 8); // LCD msg
}

void go_pd(void)
{
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xBB);   // start PD control
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(50);   // set speed to 100
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(1);   // set a = 1
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(20);   // set b = 20
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(3);   // set c = 3
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xBA);   // set d = 2
}


void stop_pd(void)
{
    while(!UART1_is_tx_ready()) continue;
    UART1_Write(0xBC);   // stop PD control
}
/**
 End of File
*/