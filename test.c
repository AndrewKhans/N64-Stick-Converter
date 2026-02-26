#include <stdio.h>  // TODO: Remove
#include <stdint.h> // TODO: Remove

typedef struct {
	uint16_t x;
	uint16_t y;
} pair16_t;

typedef struct {
	uint8_t x;
	uint8_t y;
} pair8_t;

static inline int16_t abs16(int16_t v) {
    return v >= 0 ? v : -v;
};

// Mock
void GetX() {

}

// Mock
void GetY {

}


// TODO: In the main program, don't define this as a function, just paste the contents inside Calibration()
// TODO: Should we calculate limits as we go instead of saving each in `cardinals`? We might run out of RAM!
void Calibration() {
	uint16_t xNeutral16, yNeutral16;

	// Get neutral


	// Every other cardinal is the corner of a quadrant
	quadrantLimits[0] = cardinals[1]; // Top-right corner of yellow quadrant
	quadrantLimits[1] = cardinals[3]; // Bottom-right corner of orange quadrant
	quadrantLimits[2] = cardinals[5]; // Bottom-left corner of grey quadrant
	quadrantLimits[3] = cardinals[7]; // Top-left corner of green quadrant

	// Save calibration to eeprom
	// Save quadrantLimits
	// Save quadrantScalingFactors
	// save extraScalingFactors
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

/*
	Return the scaling factor pair that should be used to scale down a stick reading to an 8 bit value.
*/
pair8_t GetScalingFactor(pair16_t rawStickReading, pair16_t neutral) {
	int16_t x, y;
	pair16_t limits;
	pair8_t sf;
	pair8_t extraSf;

	// TODO: How can we guarantee that these values fit into a 16 bit signed int?
	// x and y are our coordinates on the diagram
	x = (int16_t)rawStickReading.x - (int16_t)neutral.x;
	y = (int16_t)rawStickReading.y - (int16_t)neutral.y;
	uint8_t key = ((x > 0) << 1) | (y > 0); // left bit is x, right is y. 1=pos, 0=neg
	switch (key) {
	    case 0b11: // x > 0, y > 0: yellow quadrant
			sf        = quadrantScalingFactors[0];
			limits    = quadrantLimits[0];
			extraSf.x = extraScalingFactors.east;
			extraSf.y = extraScalingFactors.north;
	        break;

	    case 0b10: // x > 0, y <= 0: orange quadrant
			sf        = quadrantScalingFactors[1];
			limits    = quadrantLimits[1];
			extraSf.x = extraScalingFactors.east;
			extraSf.y = extraScalingFactors.south;
	        break;

	    case 0b00: // x <= 0, y <= 0: grey quadrant
			sf        = quadrantScalingFactors[2];
			limits    = quadrantLimits[2];
			extraSf.x = extraScalingFactors.west;
			extraSf.y = extraScalingFactors.south;
	        break;

	    case 0b01: // x <= 0, y > 0: green quadrant
			sf        = quadrantScalingFactors[3];
			limits    = quadrantLimits[3];
			extraSf.x = extraScalingFactors.west;
			extraSf.y = extraScalingFactors.north;
	        break;
	}

	// If we're outside of our quadrant in either dimension, use the 
	// extra scaling factor instead of the quadrant scaling factor
	if (abs16(x) > abs16(limits.x)) sf.x = extraSf.x;
	if (abs16(y) > abs16(limits.y)) sf.y = extraSf.y;

	return sf;
}

int main() {

	pair16_t neutral;
	neutral.x = 10;
	neutral.y = 10;

	printf("Filling cardinals with test data, assuming neutral is 10,10, "
		   "and the stickbox has a max length/height of 20\n");
	
	pair16_t cardinals[8];
	cardinals[0].x = 10, cardinals[0].y = 20;
	cardinals[1].x = 15, cardinals[1].y = 15;
	cardinals[2].x = 20, cardinals[2].y = 10;
	cardinals[3].x = 15, cardinals[3].y = 5;
	cardinals[4].x = 10, cardinals[4].y = 0;
	cardinals[5].x = 5,  cardinals[5].y = 5;
	cardinals[6].x = 0,  cardinals[6].y = 10;
	cardinals[7].x = 5,  cardinals[7].y = 15;
	for (int i = 0; i < sizeof(cardinals)/sizeof(cardinals[0]); i++) {
		printf("cardinals[%u]: (%u, %u)\n", i, cardinals[i].x, cardinals[i].y);
	}

	printf("Calculating scaling factors from the cardinals");
	CalculateScalingFactors();



}
