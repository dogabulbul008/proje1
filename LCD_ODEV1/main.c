#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "inc/hw_uart.h"
#include "driverlib/adc.h"
#include "inc/tm4c123gh6pm.h"

#define RS GPIO_PIN_0
#define E  GPIO_PIN_1
#define D4 GPIO_PIN_4
#define D5 GPIO_PIN_5
#define D6 GPIO_PIN_6
#define D7 GPIO_PIN_7

void SetInitSettings();
void timerkesmefonksiyonu();
void delayMs(int n);
void lcd_command(unsigned char command);
void lcd_data(unsigned char data);
void lcd_init(void);
void lcd_print(char *str);
void format_time(char *buffer, int saat, int dakika, int saniye);
void uart_init(void);
void uart_send_data(char *data);  // Seri port üzerinden veri gönderme fonksiyonu
void uart_receive_time(void);
void read_temperature(void);  // Sýcaklýk okuma fonksiyonu

// Zaman deðiþkenleri (kesme fonksiyonu için volatile)
volatile int saat = 0;
volatile int dakika = 0;
volatile int saniye = 0;
volatile uint32_t temperature = 0; // Sýcaklýk verisi

int main(void)
{
    lcd_init();               // LCD'yi baþlat
    SetInitSettings();        // Timer ve kesmeleri ayarla
    uart_init();              // Seri portu baþlat

    uart_receive_time();      // Seri port üzerinden saati al

    while (1) {
        // Sýcaklýk verisini oku ve LCD'ye yazdýr
        read_temperature();

        // LCD'nin 1. satýrýna sýcaklýk bilgisini yazdýr
        char temp_buffer[16];
        sprintf(temp_buffer, "Temp: %d C", temperature);
        lcd_command(0x80); // 1. satýra git
        lcd_print(temp_buffer);

        // Seri port üzerinden sýcaklýk verisini gönder
        uart_send_data(temp_buffer);

        SysCtlDelay(SysCtlClockGet()/3);  // 1 saniye bekle
    }
}

// UART baþlatma
void uart_init(void) {
    // UART0'ý aç
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

// Seri port üzerinden veri gönderme
void uart_send_data(char *data) {
    int i = 0;
    while (data[i] != '\0') {
        UARTCharPut(UART0_BASE, data[i]);
        i++;
    }
}

// Seri port üzerinden saati al
void uart_receive_time(void) {
    char time_buffer[9]; // "HH:mm:ss\0" þeklinde bir format
    int i = 0;

    // Saat bilgisi gelene kadar veri oku
    while (i < 8) {
        while (UARTCharsAvail(UART0_BASE)) {
            time_buffer[i] = UARTCharGet(UART0_BASE);
            i++;
        }
    }
    time_buffer[8] = '\0'; // Null terminator ekle

    // Saat bilgilerini çözümle
    sscanf(time_buffer, "%2d:%2d:%2d", &saat, &dakika, &saniye);
}

// Zaman ayarlarý
void SetInitSettings()
{
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN); // 40 MHz

    // GPIO ayarlarý
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3); // PF1, PF2, PF3 çýkýþ

    // Timer ayarlarý
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC); // Periyodik mod
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()); // 40 MHz -> 1 saniye

    // Kesme ayarlarý
    IntMasterEnable();                             // Genel kesmeleri aç
    IntEnable(INT_TIMER0A);                        // Timer0A kesmesini etkinleþtir
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Kesme kaynaðýný seç
    TimerIntRegister(TIMER0_BASE, TIMER_A, timerkesmefonksiyonu); // Kesme iþleyicisini kaydet
    TimerEnable(TIMER0_BASE, TIMER_A);             // Timer'ý baþlat

    // ADC ayarlarý
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // ADC0'ý aç
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1);
}

// Kesme iþleyicisi
void timerkesmefonksiyonu()
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Kesme bayraðýný temizle

    // Zamaný güncelle
    saniye++;
    if (saniye == 60) {
        saniye = 0;
        dakika++;
    }
    if (dakika == 60) {
        dakika = 0;
        saat++;
    }
    if (saat == 24) {
        saat = 0;
    }

    // Zamaný formatla ve LCD'ye yazdýr
    char buffer[9];
    format_time(buffer, saat, dakika, saniye);
    lcd_command(0xC8); // 2. satýra git
    lcd_print(buffer);

    // PF2 LED'in durumunu deðiþtir (görsel test için)
    uint32_t deger = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, ~deger);
}

// LCD baþlatma
void lcd_init() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, (RS | E | D4 | D5 | D6 | D7));

    delayMs(20);
    lcd_command(0x03);
    delayMs(5);
    lcd_command(0x03);
    delayMs(5);
    lcd_command(0x03);
    delayMs(1);
    lcd_command(0x02);
    delayMs(1);

    lcd_command(0x28); // 4-bit mod, 2 satýr
    delayMs(1);
    lcd_command(0x0C); // Ekraný aç, imleç kapalý
    delayMs(1);
    lcd_command(0x06); // Yazma yönü saða
    delayMs(1);
    lcd_command(0x01); // Ekraný temizle
    delayMs(2);
}

// LCD komut gönderme
void lcd_command(unsigned char command) {
    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (command & 0xF0));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, 0);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);

    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (command << 4));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, 0);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);
}

// LCD veri gönderme
void lcd_data(unsigned char data) {
    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (data & 0xF0));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, RS);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);

    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (data << 4));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, RS);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);
}

// LCD'ye string yazdýrma
void lcd_print(char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

// Zamaný formatlama
void format_time(char *buffer, int saat, int dakika, int saniye) {
    buffer[0] = '0' + (saat / 10);        //saatin onlar basamaðý hesaplanýr ve 0 ile ascii olarak toplanýr
    buffer[1] = '0' + (saat % 10);        //saatin birler basamaðý hesaplanýr ve 0 ile ascii olarak toplanýr
    buffer[2] = ':';
    buffer[3] = '0' + (dakika / 10);
    buffer[4] = '0' + (dakika % 10);
    buffer[5] = ':';
    buffer[6] = '0' + (saniye / 10);
    buffer[7] = '0' + (saniye % 10);
    buffer[8] = '\0';
}

// Gecikme fonksiyonu
void delayMs(int n) {
    SysCtlDelay((SysCtlClockGet()/3000)*n);
}

// Sýcaklýk verisini okuma
void read_temperature(void) {
    ADCProcessorTrigger(ADC0_BASE, 1); // Yazýlýmla tetikleme
    while (!ADCIntStatus(ADC0_BASE, 1, false)) {} // ADC dönüþü bekle
    ADCIntClear(ADC0_BASE, 1); // ADC dönüþü temizle

    uint32_t gelendata[4];
    ADCSequenceDataGet(ADC0_BASE, 1, gelendata); // Veriyi al

    uint32_t ortalamadeger = (gelendata[0] + gelendata[1] + gelendata[2] + gelendata[3]) / 4;
    temperature = (1475 - ((2475 * ortalamadeger) / 4096)) / 10; // Sýcaklýk hesaplama
}
