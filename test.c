#include <stdio.h>  // TODO: Remove
#include <stdint.h> // TODO: Remove


// +/- minimum range that will be achieved for each axis in standard range mode
#define MIN_RANGE_STD 101

typedef struct {
	uint16_t x;
	uint16_t y;
} pair16_t;

typedef struct {
	uint8_t x;
	uint8_t y;
} pair8_t;

// This test data says that neutral is 10,10, and the stickbox has a height and length of 20
// Mock
uint16_t GetX() {
    static int count = 0;
	uint16_t x_readings[] = {
		512, // Neutral x
		512, // North
		630, // NE
		650, // East
		630, // SE
		512, // South
		400, // SW
		380, // West
		400, // NW
	};
	return x_readings[count++];
}

// Mock
uint16_t GetY() {
    static int count = 0;
	uint16_t y_readings[] = {
		512, // Neutral y
		650, // North
		630, // NE
		512, // East
		400, // SE
		380, // South
		400, // SW
		512, // West
		630, // NW
	};
	return y_readings[count++];
}

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
struct {
	uint8_t north; // Y factor for the triangle area to the north
	uint8_t east;  // X factor for the triangle area to the east
	uint8_t south; // Y factor for the triangle area to the south
	uint8_t west;  // X factor for the triangle area to the west
} extraScalingFactors;


uint8_t CalculateScalingFactor(uint16_t reading, uint16_t neutral) {
	uint16_t temp, sf;

	if (reading > neutral){
		temp = reading - neutral;
	} else if (reading < neutral) {
		temp = neutral - reading;
	} else {
		// This would occur if you didn't move your stick from neutral during calibration
		temp = 0;
	}

	printf("reading %u, neutral %u, temp %u\n", reading, neutral, temp);

	// calculate factor (standard mode)
	sf = ((MIN_RANGE_STD*256)/temp);
	// if remainder, add one
	if ( ((MIN_RANGE_STD*256)%temp) > 0  ) sf++;

	printf("sf: %u\n", sf);
	return (uint8_t)sf;
}

// TODO: Should we calculate + save limits as we go instead of saving each in `cardinals`? We might run out of RAM!
// Loop: Press button, point to cardinal
void Calibration() {
	pair16_t neutral, reading;

	// // reset firstPowerOn variable in EEPROM
	// eeprom_update_byte(&firstPowerOn, 0x00);
	// // store the calibration slider switch's position
	// eeprom_update_byte(&calibSwitch, (PINB&(1<<PORTB2)) );

	// Get neutral
	neutral.x = GetX();
	neutral.y = GetY();

	printf("neutral x: %u y: %u\n", neutral.x, neutral.y);


	for (uint8_t i = 0; i < 8; i++) {
		// Wait for Z button press
		// while (!(PINA&(1<<PORTA3)));

		reading.x = GetX();
		reading.y = GetY();

		switch (i) {
			case 0: // North
				printf("Calculating north extra scaling factor\n");
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
		// _delay_ms(500);
	}


	for (uint8_t i = 0; i < 4; i++) {
		printf("quadrantLimits[%u]: (%u, %u)\n", i, quadrantLimits[i].x,quadrantLimits[i].y);
	}
	for (uint8_t i = 0; i < 4; i++) {
		printf("quadrantScalingFactors[%u]: (%u, %u)\n", i, quadrantScalingFactors[i].x,quadrantScalingFactors[i].y);
	}

	printf("extraScalingFactors.north: (%u)\n", extraScalingFactors.north);
	printf("extraScalingFactors.east: (%u)\n", extraScalingFactors.east);
	printf("extraScalingFactors.south: (%u)\n", extraScalingFactors.south);
	printf("extraScalingFactors.west: (%u)\n", extraScalingFactors.west);

	// Write it all to eeprom
		// Save quadrantLimits
		// Save quadrantScalingFactors
		// Save extraScalingFactors
}

uint8_t ScaleDown(uint16_t raw16, uint8_t c){
	return  (uint8_t) ( (raw16*c) >> 8);
}

/*
	Return the scaling factor pair that should be used to scale down a raw stick reading to an 8 bit value.
*/
pair8_t GetScalingFactor(pair16_t raw, pair16_t neutral) {
	pair16_t limits;
	pair8_t sf;

	// left bit is x, right is y. 1=pos, 0=neg
	uint8_t key = ((raw.x > neutral.x) << 1) | (raw.y > neutral.y);
	switch (key) {
	    case 0b11: // x > 0, y > 0
			printf("Case 1\n");
			sf     = quadrantScalingFactors[0];
			limits = quadrantLimits[0];
			if (raw.x > limits.x) sf.x = extraScalingFactors.east;
			if (raw.y > limits.y) sf.y = extraScalingFactors.north;
	        break;

	    case 0b10: // x > 0, y <= 0
			printf("Case 2\n");
			sf     = quadrantScalingFactors[1];
			limits = quadrantLimits[1];
			if (raw.x > limits.x) sf.x = extraScalingFactors.east;
			if (raw.y < limits.y) sf.y = extraScalingFactors.south;
	        break;

	    case 0b00: // x <= 0, y <= 0
			printf("Case 3\n");
			sf     = quadrantScalingFactors[2];
			limits = quadrantLimits[2];
			if (raw.x < limits.x) sf.x = extraScalingFactors.west;
			if (raw.y < limits.y) sf.y = extraScalingFactors.south;
	        break;

	    case 0b01: // x <= 0, y > 0
			printf("Case 4\n");
			sf     = quadrantScalingFactors[3];
			limits = quadrantLimits[3];
			if (raw.x < limits.x) sf.x = extraScalingFactors.west;
			if (raw.y > limits.y) sf.y = extraScalingFactors.north;
	        break;
	}

	return sf;
}

int main() {

	printf("Running calibration\n");
	Calibration();
	printf("\n");

	pair16_t neutral;
	neutral.x = 512;
	neutral.y = 512;

	pair16_t reading;
	reading.x = 580;
	reading.y = 640;

	pair8_t sf = GetScalingFactor(reading, neutral);
	printf("Scaling factor: (%u, %u)\n", sf.x, sf.y);
	pair8_t scaledDownReading;
	scaledDownReading.x = ScaleDown(reading.x, sf.x);
	scaledDownReading.y = ScaleDown(reading.y, sf.y);

	pair8_t scaledDownNeutral;
	scaledDownNeutral.x = ScaleDown(neutral.x, sf.x);
	scaledDownNeutral.y = ScaleDown(neutral.y, sf.y);
	printf("scaledDownReading: (%u, %u)\n", scaledDownReading.x, scaledDownReading.y);
	int16_t dx = (int16_t)scaledDownReading.x - scaledDownNeutral.x;
	int16_t dy = (int16_t)scaledDownReading.y - scaledDownNeutral.y;

	printf("delta: (%d, %d)\n", dx, dy);
}
