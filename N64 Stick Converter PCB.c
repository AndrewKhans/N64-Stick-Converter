/*
 * N64 Stick Converter PCB.c
 *
 * Created: 25.02.2015 17:48:54
 * Original Author: Jakob Schäfer
 * Affine Transform-Based Calibration: Andrew K.
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
 * 10 (PortA3)		|	calibration button 1 (Z) (active low)	|
 * -------------------------------------------------------------|
 * 11 (PortA2)		|	calibration button 2 (R) (active low)	|
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

#define GATE_NORTH     {0,100}
#define GATE_NORTHEAST {75,75}
#define GATE_EAST      {100,0}
#define GATE_SOUTHEAST {75,-75}
#define GATE_SOUTH     {0,-100}
#define GATE_SOUTHWEST {-75,-75}
#define GATE_WEST      {-100,0}
#define GATE_NORTHWEST {-75,75}

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
#include "point_math.h"

/******************************************************************************
Prototypes
******************************************************************************/

// returns a 16 bit ADC value of the potentiometer stick's x axis (0 - 1023)
uint16_t GetX(void);

// returns a 16 bit ADC value of the potentiometer stick's y axis (0 - 1023)
uint16_t GetY(void);

// scales the 16 bit ADC value down to 8 bits: result = raw16 * factor c / 256
uint8_pair_t ApplyTransform(uint16_pair_t raw, uint16_pair_t neutral);

// rotates a byte left by one bit
uint8_t RotateLeft(uint8_t cData);

// rotates a byte left by one bit
uint8_t RotateRight(uint8_t cData);

void Calibration(void);

/******************************************************************************
EEPROM Variables
******************************************************************************/

// for detecting first power on
uint8_t	EEMEM firstPowerOn = 1;

// stores the position of the calibration slider switch
uint8_t EEMEM calibSwitch;

AffineMat EEMEM eeprom_transformMats[8];
uint16_pair_t EEMEM eeprom_cardinals[8];

/******************************************************************************
Global Variables
******************************************************************************/

/* Stores the original calibration inputs */
fpair_t cardinals[8];

/* Stores the affine matricies used to transform stick readings from real to ideal */
AffineMat transformMats[8];

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

	uint16_pair_t neutral16, raw;
	uint8_pair_t neutral8, old, pos;

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


	if ( !(PINA&(1<<PORTA5)) ){ // extended range mode if ext. range mode button is pushed
		maxRange = MAX_RANGE_XTD;
	} else{ // standard range mode otherwise
		maxRange = MAX_RANGE_STD;
	}

	// first AD conversion; initialize analog circuitry
	neutral16.x = GetX();

	// get neutral position
	neutral16.x = GetX();
	neutral16.y = GetY();

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
		eeprom_read_block(cardinals,
						  eeprom_cardinals,
						  sizeof(cardinals));

		eeprom_read_block(transformMats,
						  eeprom_transformMats,
						  sizeof(transformMats));
	}


	// scale down neutral reading. it's fine to use the neutral reading to
	// pick the scaling factors, as they will all yield a similar result
	neutral8 = ApplyTransform(neutral16, neutral16);

	old = neutral8;

    while(1)
    {
		raw.x = GetX();
		raw.y = GetY();
		// scale down
		pos = ApplyTransform(raw, neutral16);

		// calculate the amount of steps (= increments or decrements) for both axes
		xSteps =  (int16_t) pos.x - old.x;
		ySteps =  (int16_t) pos.y - old.y;

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

void Calibration(void) {
	fpair_t neutral;

	// reset firstPowerOn variable in EEPROM
	eeprom_update_byte(&firstPowerOn, 0x00);
	// store the calibration slider switch's position
	eeprom_update_byte(&calibSwitch, (PINB&(1<<PORTB2)) );

	neutral.x = (float)GetX();
	neutral.y = (float)GetY();

	static const fpair_t idealGates[8] = {GATE_NORTH, GATE_NORTHEAST, GATE_EAST, GATE_SOUTHEAST, \
                      					  GATE_SOUTH, GATE_SOUTHWEST, GATE_WEST, GATE_NORTHWEST };

	for (uint8_t i = 0; i < 8; i++) {
		_delay_ms(50);               // debounce previous press
		while (!(PINA&(1<<PORTA3))); // wait for Z button release
		_delay_ms(50);               // debounce
		while ((PINA&(1<<PORTA3)));  // wait for Z button press

		cardinals[i].x = GetX();
		cardinals[i].y = GetY();

		if (i == 0) {
			// With just one cardinal, we can't calculate any matricies
			continue;
		} else {
			// Build S = unit->real using neutral,p2,p3
			fpair_t p2 = cardinals[i];
			fpair_t p3 = cardinals[i-1];
			AffineMat S = getAffineMat(neutral, p2, p3);

			fpair_t origin = {0.0f, 0.0f};
			fpair_t d2 = idealGates[i];
			fpair_t d3 = idealGates[i-1];
			AffineMat D = getAffineMat(origin, d2, d3);

			// M = D * S^-1 maps real -> ideal
			AffineMat S_inv = S;
			matInvert(&S_inv);
			transformMats[i-1] = matMultiply(D, S_inv);

			// After getting the 8th cardinal, calculate the 8th matrix
			if (i == 7) {
				p2 = cardinals[0];
				p3 = cardinals[7];

				S = getAffineMat(neutral, p2, p3);
				S_inv = S;
				matInvert(&S_inv);

				d2 = idealGates[0];
				d3 = idealGates[7];
				D = getAffineMat(origin, d2, d3);
				transformMats[7] = matMultiply(D, S_inv);
			}
		}
	}

	// write calibration to eeprom
	eeprom_update_block(cardinals,
						eeprom_cardinals,
						sizeof(cardinals));

	eeprom_update_block(transformMats,
						eeprom_transformMats,
						sizeof(transformMats));
}

/* Return the octant (like "quadrant" but for eight) of the stick that `point` is in */
// Simpler approach: determine quadrant by signs, then test against the diagonal cardinal
static inline uint8_t GetOctant(fpair_t point, fpair_t neutral) {
    fpair_t pointVec, cardinalVec;

    // vector from neutral to point
    pointVec.x = point.x - neutral.x;
    pointVec.y = point.y - neutral.y;

    if (pointVec.x == 0.0f && pointVec.y == 0.0f) return 0;

    uint8_t posX = (pointVec.x > 0.0f);
    uint8_t posY = (pointVec.y > 0.0f);

    // choose the diagonal cardinal that splits this quadrant
    // ordering of cardinals: 0=N,1=NE,2=E,3=SE,4=S,5=SW,6=W,7=NW
    uint8_t diag;
    if (posX && posY)      diag = 1; // NE quadrant
    else if (posX && !posY) diag = 3; // SE quadrant
    else if (!posX && !posY)diag = 5; // SW quadrant
    else                    diag = 7; // NW quadrant

    // vector from neutral to that diagonal cardinal
    cardinalVec.x = cardinals[diag].x - neutral.x;
    cardinalVec.y = cardinals[diag].y - neutral.y;

    // cross product tells which side of the diagonal pointVec lies on
    float cross = cardinalVec.x * pointVec.y - cardinalVec.y * pointVec.x;

    // if pointVec is to the left (CCW) of the diagonal, it is in the octant closer to the previous cardinal
    if (cross > FLOAT_COMPARE_EPS) {
        // octant is diag-1 (wrap around)
        return (diag == 0) ? 7 : (diag - 1);
    }

    // otherwise it's the octant at diag
    return diag;
}

// Determine which pizza slice we're in
// Apply transform from that slice
// Convert floats to uint8
uint8_pair_t ApplyTransform(uint16_pair_t raw, uint16_pair_t neutral){
	fpair_t rawf = {(float)raw.x, (float)raw.y};
	fpair_t neutralf = {(float)neutral.x, (float)neutral.y};

	uint8_t octant = GetOctant(rawf, neutralf);

	// Work on a local copy of the affine matrix and invert it to map real->ideal
	AffineMat transformMat = transformMats[octant];
	// Map raw reading into the unit/barycentric coordinates (u,v)
	fpair_t pointT = matPointMultiply(transformMat, rawf);

	// Map ideal gate coords (centered at 0) to 8-bit space with center at 128
	// TODO: Is the output supposed to be centered at 128?
	pointT.x += 128.0f;
	pointT.y += 128.0f;

	if (pointT.x < 0.0f)   pointT.x = 0.0f;
	if (pointT.x > 255.0f) pointT.x = 255.0f;
	if (pointT.y < 0.0f)   pointT.y = 0.0f;
	if (pointT.y > 255.0f) pointT.y = 255.0f;

	// Todo: To improve accuracy, round before casting
	uint8_pair_t ret = {(uint8_t)pointT.x, (uint8_t)pointT.y};
	return ret;
}