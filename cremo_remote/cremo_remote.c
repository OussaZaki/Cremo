/*
To debug data, plug into a USB port and open a virtual COM port
at 9600 baud.
*/

#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <radio_queue.h>

#include <stdio.h>

#define TX_PERIOD 1

uint16 lastTxTime = 0;
static uint8 pins[] = { 0, 1, 2 };

void updateLEDs()
{
	LED_YELLOW((getMs() & 0x3FF) <= 20);	// heartbeat
	LED_RED(0);
	usbShowStatusWithGreenLed();
}

// for printf
void putchar(char c)
{
	usbComTxSendByte(c);
}

uint8 isStableCommand(uint8 command)
{
	return (command == 0);
}

// returns a byte (1,2,3) representing the instruction
uint8 readArduino()
{
	uint8 pin;
	uint8 size = sizeof(pins);
	// loop through all the Input Pins
	for ( pin = 0; pin < size; pin++)
	{

		// USB debugging output
		if (usbPowerPresent() && usbComTxAvailable() >= 45)
		{
			printf("pin: %02u, value: %02u\r\n", pin, isPinHigh(pins[pin]));
		}

		// test which pin is activated
		if( isPinHigh(pins[pin]) )

			// the resulting command is stored in a single byte
			return (pin + 1);
	}
	return 0;
}

// transmits a the instruction byte via radio
void rcService()
{
	uint8 XDATA *txPacket;
	uint8 command;

	// this delay is currently negligible, but we'll keep it in case we need it in the future
	if((uint16)(getMs() - lastTxTime) >= TX_PERIOD)
	{
		// read the instruction and prepare the TX packet
		command = readArduino();

		// there's no need to send stable instruction
		if(!isStableCommand(command))
		{
			txPacket = radioQueueTxCurrentPacket();

			// ensure that we have a free packet to transmit
			if(txPacket)
			{
				lastTxTime = getMs();
				txPacket[0] = 1;
				txPacket[1] = command;
				radioQueueTxSendPacket();
			}

		}
	}
}

void setup()
{
	uint8 pin;
	uint8 size = sizeof(pins);
	for ( pin = 0; pin < size; pin++)
	{
		setDigitalInput(pin,0);
	}
}

void main()
{
	systemInit();
	usbInit();
	radioQueueInit();
	setup();
	while(1)
	{
		updateLEDs();
        boardService();
        usbComService();			// required for programming
        rcService();				// controls motors using remote control commands
    }
}
