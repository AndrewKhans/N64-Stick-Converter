/*
 * N64 Stick Converter PCB.c
 *
 * Created: 25.02.2015 17:48:54
 * Original Author: Jakob Schäfer
 * Cardinal-Based Scaling Edit: Andrew K.
 *
 * ONLY FOR YOUR OWN PERSONAL USE! COMMERCIAL USE PROHIBITED!
 * NUR FÜR DEN EIGENGEBRAUCH! GEWERBLICHE NUTZUNG VERBOTEN!
 *
 * fusebyte low:	0x42
 * fusebyte high:	0xDF
 *
 * Note: Using -o1 optimization level for the AVR/GNU C compiler
 *		 is recommended.
 *
 * --------------------------------------------------------------
 * ATtiny24A pin	|	function			 					|
 * (PDIP / SOIC)	|											|
 * -------------------------------------------------------------|
 * 1				|	VCC = 									|
 * 					|	N64 controller PCB pin no. 5.			|
 * 					|	Bypass to GND with 100 nF capacitor		|
 * -------------------------------------------------------------|
 * 2				|	N64 controller PCB pin no. 6			|
 * -------------------------------------------------------------|
 * 3				|	N64 controller PCB pin no. 3			|
 * -------------------------------------------------------------|
 * 4				|	RESET for programming					|
 * 					|	Connect with 10 kOhm resistor to VCC	|
 * -------------------------------------------------------------|
 * 5				|	calibration slider switch				|
 * 					|	(leave floating or short to GND)		|
 * 					|	change switch positon for calibration	|
 * -------------------------------------------------------------|
 * 6				|	N64 controller PCB pin no. 2			|
 * -------------------------------------------------------------|
 * 7				|	N64 controller PCB pin no. 1			|
 * 					|	MOSI for programming					|
 * -------------------------------------------------------------|
 * 8				|	extended range mode button (active low)	|
 *					|	short to GND to use extended range mode	|
 * -------------------------------------------------------------|
 * 9				|	SCK for programming						|
 * -------------------------------------------------------------|
 * 10 (PortA3)		|	calibration button 1 (R) (active low)	|
 * -------------------------------------------------------------|
 * 11 (PortA2)		|	calibration button 2 (Z) (active low)	|
 *					|	short both buttons to GND for calibr.	|
 * -------------------------------------------------------------|
 * 12				|	X axis of the stick potentiometer		|
 * -------------------------------------------------------------|
 * 13				|	Y axis of the stick potentiometer		|
 * -------------------------------------------------------------|
 * 14				|	GND =									|
 * 					|	N64 controller PCB pin no. 4			|
 * -------------------------------------------------------------|
 *
 *
 * If you want to increase/decrease the range of the stick, then try
 * out new values for the MIN_RANGE and MAX_RANGE constants below:
 */


/******************************************************************************
Macros & Defines
******************************************************************************/
// clock frequency
#define F_CPU 1000000UL

// +/- minimum range that will be achieved for each axis in standard range mode
#define MIN_RANGE_STD 101
// max. range limit in standard range mode; higher values will be clipped
#define MAX_RANGE_STD 105

// +/- minimum range that will be achieved for each axis in extended range mode
#define MIN_RANGE_XTD 101
// max. range limit in extended range mode, higher values will be clipped
#define MAX_RANGE_XTD 105

/******************************************************************************
Includes
******************************************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

/******************************************************************************
Prototypes
******************************************************************************/

typedef struct {
	uint16_t x;
	uint16_t y;
} pair16_t;

typedef struct {
	uint8_t x;
	uint8_t y;
} pair8_t;

typedef struct {
	uint8_t north; // Y factor for the triangle area to the north
	uint8_t east;  // X factor for the triangle area to the east
	uint8_t south; // Y factor for the triangle area to the south
	uint8_t west;  // X factor for the triangle area to the west
} extraScalingFactors_t;

// returns a 16 bit ADC value of the potentiometer stick's x axis (0 - 1023)
uint16_t GetX(void);

// returns a 16 bit ADC value of the potentiometer stick's y axis (0 - 1023)
uint16_t GetY(void);

// scales the 16 bit ADC value down to 8 bits: result = raw16 * factor c / 256
pair8_t ScaleDown(pair16_t raw, pair16_t neutral);

// rotates a byte left by one bit
uint8_t RotateLeft(uint8_t cData);

// rotates a byte left by one bit
uint8_t RotateRight(uint8_t cData);

// helper function for Calibration
uint8_t CalculateScalingFactor(uint16_t reading, uint16_t neutral);

// returns scaling factor pair to be used for scaling `raw`
pair8_t GetScalingFactor(pair16_t raw, pair16_t neutral);

// calculates the c factors and saves them into EEPROM
void Calibration(void);

/******************************************************************************
EEPROM Variables
******************************************************************************/
// factors for x & y axis in standard range mode
uint8_t EEMEM cx_std = 0;
uint8_t EEMEM cy_std = 0;

// factors for x & y axis in extended range mode
uint8_t EEMEM cx_xtd = 0;
uint8_t EEMEM cy_xtd = 0;

// for detecting first power on
uint8_t	EEMEM firstPowerOn = 1;

// stores the position of the calibration slider switch
uint8_t EEMEM calibSwitch;

pair16_t EEMEM eeprom_quadrantLimits[4];
pair8_t  EEMEM eeprom_quadrantScalingFactors[4];
extraScalingFactors_t EEMEM eeprom_extraScalingFactors;

/******************************************************************************
Global Variables
******************************************************************************/

/*
	Stores the cardinal reading that defines the limit of each quadrant, in clockwise order
	- quadrantLimits[0]: Top-right corner of yellow quadrant
	- quadrantLimits[1]: Bottom-right corner of orange quadrant
	- quadrantLimits[2]: Bottom-left corner of grey quadrant
	- quadrantLimits[3]: Top-left corner of green quadrant
*/
pair16_t quadrantLimits[4];

/*
	Stores the X and Y scaling factors to be used when stick reading is in each quadrant
	- quadrantScalingFactors[0]: factors for yellow quadrant
	- quadrantScalingFactors[1]: factors for orange quadrant
	- quadrantScalingFactors[2]: factors for grey quadrant
	- quadrantScalingFactors[3]: factors for green quadrant
*/
pair8_t quadrantScalingFactors[4];

/*
	Stores the scaling factor to be used when an X or Y coordinate is outside of our quadrants,
	in the blue/purple triangle area
*/
extraScalingFactors_t extraScalingFactors;

/******************************************************************************
Fuses
******************************************************************************/
__fuse_t __fuse __attribute__((section (".fuse"))) = {	.low		= 0x42,
														.high		= HFUSE_DEFAULT,
														.extended	= EFUSE_DEFAULT};

int main(void)
{
	int16_t xSteps, ySteps;
	uint8_t xWheel = 0b11001100;
	uint8_t yWheel = 0b00110011;
	uint8_t maxRange;

	pair16_t neutral16, raw;
	pair8_t neutral8, old, pos;

	// set up the ports immediately
	DDRA = (1<<DDA6)|(1<<DDA7);
	DDRB = (1<<DDB0)|(1<<DDB1);
	PORTA = (1<<PORTA2)|(1<<PORTA3)|(1<<PORTA5);
	PORTB = (1<<PORTB2);

	// deactivate timer0, timer1 and USI peripherals for saving power
	PRR |= (1<<PRTIM0)|(1<<PRTIM1)|(1<<PRUSI);

	// now wait a little bit
	_delay_ms(250);

	// ADC setup
	DIDR0 = (1<<ADC0D)|(1<<ADC1D);			// digital input disable for PORTA0+1
	ADMUX = 0x01;							// channel 1
	ADCSRA = (1<<ADPS0)|(1<<ADPS1);			// prescaler = 8 ==> f_ADC = 1 MHz/8 = 125 kHz
	ADCSRA |= (1<<ADEN);					// enable ADC

	// extended range mode if ext. range mode button is pushed
	if ( !(PINA&(1<<PORTA5)) ){
		maxRange = MAX_RANGE_XTD;
	}
	// standard range mode otherwise
	else{
		maxRange = MAX_RANGE_STD;
	}

	// execute calibration if:
	// a) microcontroller is powered on for the first time or
	// b) the calibration switch's position has been changed or
	// c) both calibration button's have been pushed
	if ( (eeprom_read_byte(&firstPowerOn)) ||
		 ((PINB&(1<<PORTB2)) != eeprom_read_byte(&calibSwitch)) ||
		 !(PINA&((1<<PORTA2)|(1<<PORTA3))) ) {
		// The calibration remains in memory after Calibration(), no need for eeprom read
		Calibration();
	} else {
		// Load calibration
		eeprom_read_block(quadrantLimits,
						  eeprom_quadrantLimits,
						  sizeof(quadrantLimits));

		eeprom_read_block(quadrantScalingFactors,
						  eeprom_quadrantScalingFactors,
						  sizeof(quadrantScalingFactors));

		eeprom_read_block(&extraScalingFactors,
						  &eeprom_extraScalingFactors,
						  sizeof(extraScalingFactors));
	}

	// first AD conversion; initialize analog circuitry
	neutral16.x = GetX();

	// get neutral position
	neutral16.x = GetX();
	neutral16.y = GetY();

	// scale down neutral reading. it's fine to use the neutral reading to
	// pick the scaling factors, as they will all yield a similar result
	neutral8 = ScaleDown(neutral16, neutral16);

	old = neutral8;

    while(1)
    {
		// get x axis position
		raw.x = GetX();
		raw.y = GetY();
		// scale down
		pos = ScaleDown(raw, neutral16);

		// limit position to  +/- maxRange (x)
		if ( (pos.x>neutral8.x) && ((pos.x-neutral8.x) > maxRange) ) pos.x = neutral8.x + maxRange;
		if ( (pos.x<neutral8.x) && ((neutral8.x-pos.x) > maxRange) ) pos.x = neutral8.x - maxRange;
		// limit position to  +/- maxRange (y)
		if ( (pos.y>neutral8.y) && ((pos.y-neutral8.y) > maxRange) ) pos.y = neutral8.y + maxRange;
		if ( (pos.y<neutral8.y) && ((neutral8.y-pos.y) > maxRange) ) pos.y = neutral8.y - maxRange;

		// calculate the amount of steps (= increments or decrements) for both axes
		xSteps = (int16_t) pos.x - old.x;
		ySteps = (int16_t) pos.y - old.y;

		// store current stick position for the next cycle
		old = pos;

		// while there are still steps left...
		while ( (xSteps!=0) || (ySteps!=0) ){

			// rotate the x wheel...
			if (xSteps<0){
				xWheel = RotateLeft(xWheel);
				xSteps++;
			}
			if (xSteps>0){
				xWheel = RotateRight(xWheel);
				xSteps--;
			}

			// rotate the y wheel...
			if (ySteps>0){
				yWheel = RotateRight(yWheel);
				ySteps--;
			}
			if (ySteps<0){
				yWheel = RotateLeft(yWheel);
				ySteps++;
			}

			// and put out the new XA/XB and YA/YB values:
			PORTB = (PORTB&0b11111100)|(xWheel & 0b00000011);
			PORTA = (PORTA&0b00111111)|(yWheel & 0b11000000);
		}
    }
}


uint16_t GetX(void){
	// select ADC channel 1
	ADMUX = 0x01;
	// start AD conversion
	ADCSRA |= (1<<ADSC);
	// wait until conversion is finished
	while (ADCSRA & (1<<ADSC));
	return ADC;
}

uint16_t GetY(void){
	// select ADC channel 0
	ADMUX = 0x00;
	// start AD conversion
	ADCSRA |= (1<<ADSC);
	// wait until conversion is finished
	while (ADCSRA & (1<<ADSC));
	return ADC;
}

uint8_t RotateLeft (uint8_t cData){
	uint8_t result;
	if ( cData & (1<<7) )
		result = (cData<<1)|(1<<0);
	else
		result = (cData<<1);
	return result;
}

uint8_t RotateRight (uint8_t cData){
	uint8_t result;
	if ( cData & (1<<0) )
		result = (cData>>1)|(1<<7);
	else
		result = (cData>>1);
	return result;
}

uint8_t CalculateScalingFactor(uint16_t reading, uint16_t neutral) {
	uint16_t temp, sf;

	if (reading > neutral){
		temp = reading - neutral;
	} else if (reading < neutral) {
		temp = neutral - reading;
	} else {
		// this would occur if you didn't move your stick from neutral during calibration
		temp = 1;
	}

	// calculate factor (standard mode)
	sf = ((MIN_RANGE_STD*256)/temp);
	// if remainder, add one
	if ( ((MIN_RANGE_STD*256)%temp) > 0  ) sf++;

	return (uint8_t)sf;
}

void Calibration(void){
	pair16_t neutral, reading;

	// reset firstPowerOn variable in EEPROM
	eeprom_update_byte(&firstPowerOn, 0x00);
	// store the calibration slider switch's position
	eeprom_update_byte(&calibSwitch, (PINB&(1<<PORTB2)) );

	// Get neutral
	neutral.x = GetX();
	neutral.y = GetY();

	for (uint8_t i = 0; i < 8; i++) {
		_delay_ms(50); // debounce previous press
		// wait for Z button release
		while (!(PINA&(1<<PORTA3)));
		_delay_ms(50); // debounce
		// wait for Z button press
		while ((PINA&(1<<PORTA3)));

		reading.x = GetX();
		reading.y = GetY();

		switch (i) {
			case 0: // North
				extraScalingFactors.north = CalculateScalingFactor(reading.y, neutral.y);
				break;
			case 2: // East
				extraScalingFactors.east = CalculateScalingFactor(reading.x, neutral.x);
				break;
			case 4: // South
				extraScalingFactors.south = CalculateScalingFactor(reading.y, neutral.y);
				break;
			case 6: // West
				extraScalingFactors.west = CalculateScalingFactor(reading.x, neutral.x);
				break;
			default: // Corners of quadrants
				quadrantLimits[i/2] = reading;
				quadrantScalingFactors[i/2].x = CalculateScalingFactor(reading.x, neutral.x);
				quadrantScalingFactors[i/2].y = CalculateScalingFactor(reading.y, neutral.y);
				break;
		}
	}

	// write it all to eeprom
	eeprom_update_block(quadrantLimits,
						eeprom_quadrantLimits,
						sizeof(quadrantLimits));

	eeprom_update_block(quadrantScalingFactors,
						eeprom_quadrantScalingFactors,
						sizeof(quadrantScalingFactors));

	eeprom_update_block(&extraScalingFactors,
						&eeprom_extraScalingFactors,
						sizeof(extraScalingFactors));
}

pair8_t GetScalingFactor(pair16_t raw, pair16_t neutral) {
	pair16_t limits;
	pair8_t sf;

	// left bit is x, right is y. 1=pos, 0=neg
	uint8_t key = ((raw.x > neutral.x) << 1) | (raw.y > neutral.y);
	switch (key) {
	    case 0b11: // x > 0, y > 0
			sf     = quadrantScalingFactors[0];
			limits = quadrantLimits[0];
			if (raw.x > limits.x) sf.x = extraScalingFactors.east;
			if (raw.y > limits.y) sf.y = extraScalingFactors.north;
	        break;

	    case 0b10: // x > 0, y <= 0
			sf     = quadrantScalingFactors[1];
			limits = quadrantLimits[1];
			if (raw.x > limits.x) sf.x = extraScalingFactors.east;
			if (raw.y < limits.y) sf.y = extraScalingFactors.south;
	        break;

	    case 0b00: // x <= 0, y <= 0
			sf     = quadrantScalingFactors[2];
			limits = quadrantLimits[2];
			if (raw.x < limits.x) sf.x = extraScalingFactors.west;
			if (raw.y < limits.y) sf.y = extraScalingFactors.south;
	        break;

	    case 0b01: // x <= 0, y > 0
			sf     = quadrantScalingFactors[3];
			limits = quadrantLimits[3];
			if (raw.x < limits.x) sf.x = extraScalingFactors.west;
			if (raw.y > limits.y) sf.y = extraScalingFactors.north;
	        break;
	}

	return sf;
}

pair8_t ScaleDown(pair16_t raw, pair16_t neutral){
	pair8_t sf = GetScalingFactor(raw, neutral);

	pair8_t ret;
	ret.x = (uint8_t) ((raw.x*sf.x) >> 8);
	ret.y = (uint8_t) ((raw.y*sf.y) >> 8);

	return ret;
}