#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "f73-rncontrol-lib/uart.h"
#include "f73-rncontrol-lib/adc.h"
#include "f73-rncontrol-lib/button.h"
#include "f73-rncontrol-lib/led.h"
#include "f73-rncontrol-lib/counter.h"




// Überlaufzähler - global damit sowohl von der ISR wie aus den Funktionen
//                  darauf zugegriffen werden kann
volatile uint32_t OvCnt = 0;



// Interrupt-Service-Routine für den Interrupt bei Vergleich des Timer0
// ISR: Schlüsselwort für Compiler, dass dies eine ISR ist
// TIMER0_COMP_vect: Information an den Compiler, mit welchem Interrupt
//                   diese ISR verknüpft werden soll. Der Bezeichner "TIMER0_COMP_vect"
//                   ist wie alle anderen ISR-Bezeichner in "avr/interrupt.h" definiert.
ISR(TIMER0_COMP_vect)
{

	// Überlaufzähler inkrementieren
	OvCnt++;
	
	
}

// Init Timer0 Compare ISR
static void Timer0_COMP_ISR_init()
{
	// init Timer0

	// Konfigurationsregister
	//  WGM1:0 = CTC Mode
	//  COM1:0 = normaler Betrieb
	//  CS02:0 = Vorteiler 64
	TCCR0 |= (1 << WGM01); // CTC Mode aktivieren
	counter0Start(ATMEGA32_COUNTER_0_PRESCALER_64);

	// Vergleichregister setzen
	// 1 Zählschritt tSTEP => 64x62.5ns=4us
	// 1 Überlauf => 256x4us=1.024ms => 1024us
	// tOVF = 1ms
	// TCNT Prelod = 256 - tOVF/tSTEP = 1ms/4us = 1 x10x-3s / 4x10-6s = 250
	// 250x4us = 1ms
	counter0SetCompare(250);

	// Zählerregisters zurücksetzen
	counter0SetValue(0);

	// Interruptmaskenregister setzen:
	// TOIE0: INT auslösen bei Überlauf Timer0 inaktiv
	// OCIE0: INT auslösen bei Vergleich Timer0 aktiv
	counter0EnableCompareMatchInterrupt();

	// Interrupts global freigeben
	sei();
}

int main()
{
	// init LED
	ledInit();

	// init Buttons
	buttonInit();

	//init UART 
	uartInit(9600,8,'N',1);

	// Init Timer0 Compare ISR
	Timer0_COMP_ISR_init();
	
	printf("\r\nStoppuhr\r\n");
	printf("Start Button 1 druecken\r\n");
	printf("Stopp Button 2 druecken\r\n\r\n");
    
	uint32_t totalTime=0; // Laufzeit 2^32 => ca. 49 Tage

	while (1)
	{
		while (buttonRead()!=1)
		{
			//warten
		}
		ledSet(8);
		counter0SetValue(0); // Zählregister zurücksetzten
		OvCnt=0;			// Overflow Zähler zurücksetzten

		while (buttonRead()!=5)
		{
			// Warte auf Tastendruck
		}
		totalTime=OvCnt; // Zwischenspeichern des Overflow Zählers in ms
		ledClear(8);
		printf("\r\nZeitmessung: %32lu ms\r\n",(unsigned long)totalTime);// Ausgabe
			

	}
	return 0;
}
