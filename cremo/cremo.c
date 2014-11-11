/*
to debug, plug into a USB port and open a virtual COM port
at 9600 baud.
*/

#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <gpio.h>
#include <radio_queue.h>

#include <stdio.h>

#define RX_PERIOD 32

typedef struct RCCOMMAND
{
	uint8 length;
	uint8 command;
} RCCommand;

uint16 lastRxTime = 0;			// time index of the last time we processed an rc command

BIT motorLeftDirection;
BIT motorRightDirection;

int16 motorLeftDutyCycle = 0;		// integer percentage of duty cycle for left motor
int16 motorRightDutyCycle = 0;		// integer percentage of duty cycle for right motor

int8 motorTime;					// integer percentage of where we are in the motor duty cycle

BIT upsideDown;
uint8 upsideDownCounter = 0;

// our interrupt service routine fires with T3
ISR(T3, 2)
{
	motorTime ++;

	// should the left motor be turned on for this part of the duty cycle?
	if(motorTime < motorLeftDutyCycle)
	{
		if(motorLeftDirection)
		{
			// disable the inactive PWM pin first, so we don't damage the H-bridge
			P1_3 = 0;
			P1_2 = 1;
		}
		else
		{
			// disable the inactive PWM pin first, so we don't damage the H-bridge
			P1_2 = 0;
			P1_3 = 1;
		}
	}
	else	// inactive part of duty cycle
	{
		P1_2 = 0;
		P1_3 = 0;
	}

	// should the right motor be turned on for this part of the duty cycle?
	if(motorTime < motorRightDutyCycle)
	{
		if(motorRightDirection)
		{
			// disable the inactive PWM pin first, so we don't damage the H-bridge
			P1_5 = 0;
			P1_4 = 1;
		}
		else
		{
			// disable the inactive PWM pin first, so we don't damage the H-bridge
			P1_4 = 0;
			P1_5 = 1;
		}
	}
	else	// inactive part of duty cycle
	{
		P1_4 = 0;
		P1_5 = 0;
	}

	// restart the duty cycle
	if(motorTime >= 100)
	{
		motorTime = 0;
	}
}

// enables timer 3 and sets the motor output pins
void pwmInit()
{
   T3IE = 1;					// enable T3 interrupt

   T3CTL = 0x0;					// reset T3 control bits
   T3CTL |= 10;					// T3 runs modulo T3CC0
   T3CTL |= (1 << 3);			// interrupt enabled
   T3CTL |= (1 << 4);			// normal operation (not suspended)
   T3CTL |= (5 << 5);			// tick frequency /32

   T3CC0 = 5;					// T3 maximum count

   P1DIR |= (1 << 2);			// set P1_2 as left motor output
   P1DIR |= (1 << 3);			// set P1_3 as left motor output
   P1DIR |= (1 << 4);			// set P1_4 as right motor output
   P1DIR |= (1 << 5);			// set P1_5 as right motor output

   P1_2 = 0;					// set all motor outputs to zero
   P1_3 = 0;
   P1_4 = 0;
   P1_5 = 0;
}

void updateLEDs()
{
	LED_RED(0);									// we never use the red LED
	LED_YELLOW((getMs() & 0x3FF) <= 20);		// heartbeat
	usbShowStatusWithGreenLed();				// green LED indicates USB activity
}

// grab the most recent byte from the radio RX buffer, or 0x00
// if no command is available (which is still a valid movement
// command, since 0x00 means to stop)
uint8 getMostRecentByteRx()
{
	RCCommand XDATA *rxCurr;
	uint8 command = 0x00;		// stable instruction

	do
	{
		rxCurr = (RCCommand XDATA*)radioQueueRxCurrentPacket();
		if(rxCurr)
		{
			// retain the most recent RC command
			command = rxCurr -> command;
			radioQueueRxDoneWithPacket();
		}
	}
	while(rxCurr);	// keep looping until we run out of packets

	// this will either be the most recent byte, or 0x0 if no
	// hand is on the controller (or out of range)
	return command;
}

// x and y are assumed to be in the range [-8, 7], and the
// resulting left and right variables are in the range [-100, 100]
void getMotorSpeeds(int8 command, int16 *left, int16 *right)
{
	const int16 DEFAULT_SPEED = 75;
	*left = DEFAULT_SPEED;
	*right = DEFAULT_SPEED;

	switch (command)
	{
		case 1 :
			*left =	100;
			*right = 100;
			break;
		case 2 :
			*left =	-60;
			*right = 60;
			break;
		case 3 :
			*left =	60;
			*right = -60;
			break;
	}

}

// for the printf function
void putchar(char c)
{
    usbComTxSendByte(c);
}

// gets a command from the remote and adjusts the PWM duty cycles of the motors
void rcService()
{

	uint8 signal;								// joystick position from remote control

	int16 n_motorLeft, n_motorRight;			// do the RC processing using temporary variables
	BIT n_motorLeftDir, n_motorRightDir;		// so we don't have to disable the ISR for too long

	// a longer delay will reduce response time, but will mean that the
	// robot doesn't stop immediately if it misses a TX'd byte
	if((uint16)(getMs() - lastRxTime) >= RX_PERIOD)
	{
		signal = getMostRecentByteRx();		// grab the last byte the remote sent
		lastRxTime = getMs();

		// and then map those values to motor speeds
		getMotorSpeeds(signal, &n_motorLeft, &n_motorRight);

		// determine the motor directions; negative means forward
		n_motorLeftDir = (n_motorLeft >= 0);
		n_motorRightDir = (n_motorRight >= 0);

		// make sure that we copy over positive duty cycles
		if(!n_motorLeftDir)
			n_motorLeft = -n_motorLeft;
		if(!n_motorRightDir)
			n_motorRight = -n_motorRight;

		// USB debugging output
		if (usbPowerPresent() && usbComTxAvailable() >= 70)
		{
			printf("x:%04d, L:%03d, R:%03d\r\n", signal, n_motorLeft, n_motorRight);
		}

		// avoid disabling our interrupts if we can; only accept new motor speeds
		if(motorLeftDutyCycle != n_motorLeft || motorRightDutyCycle != n_motorRight ||
		   motorLeftDirection != n_motorLeftDir || motorRightDirection != n_motorRightDir)
		{
			// disable T3 interrupts so we can safely change the motor duty cycles and directions
			T3IE = 0;

			// copy over the new duty cycles
			motorLeftDutyCycle = n_motorLeft;
			motorRightDutyCycle = n_motorRight;

			// get the motor directions from the signals we received
			motorLeftDirection = n_motorLeftDir;
			motorRightDirection = n_motorRightDir;

			// re-enable T3 interrupts
			T3IE = 1;
		}

	}
}

// program starts here
void main()
{
    systemInit();
    usbInit();			// required for programming
    radioQueueInit();	// required for RC command receipt
    pwmInit();			// sets output pins for PWM and configures timer 3

    while(1)
    {
		updateLEDs();
        boardService();
        usbComService();			// required for programming
        rcService();				// controls motors using remote control commands
    }
}
