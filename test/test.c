#include <stdio.h>
#include <stdint.h>

// +/- minimum range that will be achieved for each axis in standard range mode
#define MIN_RANGE_STD 101

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


// Loop: Press button, point to cardinal
void Calibration() {
	uint16_pair_t neutral, reading;

	// reset firstPowerOn variable in EEPROM
	eeprom_update_byte(&firstPowerOn, 0x00);
	// store the calibration slider switch's position
	eeprom_update_byte(&calibSwitch, (PINB&(1<<PORTB2)) );

	// Get neutral
	neutral.x = GetX();
	neutral.y = GetY();

	for (uint8_t i = 0; i < 8; i++) {
		_delay_ms(50); // Debounce previous press
		// Wait for Z button release
		while (!(PINA&(1<<PORTA3)));
		_delay_ms(50); // Debounce
		// Wait for Z button press
		while ((PINA&(1<<PORTA3)));

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
	}

	// Write it all to eeprom
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

int main() {
	printf("Running calibration\n");
	Calibration();

	for (uint8_t i = 0; i < 4; i++)
		printf("quadrantLimits[%u]: (%u, %u)\n", i, quadrantLimits[i].x,quadrantLimits[i].y);
	for (uint8_t i = 0; i < 4; i++)
		printf("quadrantScalingFactors[%u]: (%u, %u)\n", i, quadrantScalingFactors[i].x,quadrantScalingFactors[i].y);
	printf("extraScalingFactors.north: (%u)\n", extraScalingFactors.north);
	printf("extraScalingFactors.east: (%u)\n", extraScalingFactors.east);
	printf("extraScalingFactors.south: (%u)\n", extraScalingFactors.south);
	printf("extraScalingFactors.west: (%u)\n", extraScalingFactors.west);

	printf("\n");

	uint16_pair_t neutral;
	neutral.x = 512;
	neutral.y = 512;

	uint16_pair_t reading;
	reading.x = 580;
	reading.y = 640;

	uint8_pair_t sf = GetScalingFactor(reading, neutral);
	printf("Scaling factor: (%u, %u)\n", sf.x, sf.y);
	uint8_pair_t scaledDownReading;
	scaledDownReading.x = ScaleDown(reading.x, sf.x);
	scaledDownReading.y = ScaleDown(reading.y, sf.y);

	uint8_pair_t scaledDownNeutral;
	scaledDownNeutral.x = ScaleDown(neutral.x, sf.x);
	scaledDownNeutral.y = ScaleDown(neutral.y, sf.y);
	printf("scaledDownReading: (%u, %u)\n", scaledDownReading.x, scaledDownReading.y);
	int16_t dx = (int16_t)scaledDownReading.x - scaledDownNeutral.x;
	int16_t dy = (int16_t)scaledDownReading.y - scaledDownNeutral.y;

	printf("delta: (%d, %d)\n", dx, dy);
}
