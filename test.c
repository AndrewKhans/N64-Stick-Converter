#include <stdio.h>

#define uint8_t  unsigned char
#define uint16_t unsigned short

typedef struct {
    uint8_t x;
    uint8_t y;
} scalingFactor_t;

typedef struct {
	uint8_t north;
	uint8_t east;
	uint8_t south;
	uint8_t west;
} extraScalingFactors_t;

/*
	Return the scaling factor that should be used, based on our current x and y.
	This assumes 0 is the center.
*/
scalingFactor_t getScalingFactor(uint16_t xReading,
								 uint16_t yReading,
								 uint16_t xNeutral,
								 uint16_t yNeutral,
								 const scalingFactor_t *quadrantScalingFactors,
								 const extraScalingFactors_t *extraScalingFactors,
								 const uint16_t *cardinals) {
	uint16_t limits[2];
	int16_t x, y;
	scalingFactor_t sf;
	scalingFactor_t extraSf;
	uint8_t quadrant;

	// x and y are our coordinates on the diagram
	// Todo: Ensure these values fit into a 16 bit signed int
	x = xReading - xNeutral;
	y = yReading - yNeutral;
	uint8_t key = ((x > 0) << 1) | (y > 0); // left bit is x, right is y. 1=pos, 0=neg
	switch (key) {
	    case 0b11:   // x > 0, y > 0
			quadrant = 0;
			extraSf.x = extraScalingFactors->east;
			extraSf.y = extraScalingFactors->north;
	        break;

	    case 0b10:   // x > 0, y <= 0
			quadrant = 1;
			extraSf.x = extraScalingFactors->east;
			extraSf.y = extraScalingFactors->south;
	        break;

	    case 0b00:   // x <= 0, y <= 0
			quadrant = 2;
			extraSf.x = extraScalingFactors->west;
			extraSf.y = extraScalingFactors->south;
	        break;

	    case 0b01:   // x <= 0, y > 0
			quadrant = 3;
			extraSf.x = extraScalingFactors->west;
			extraSf.y = extraScalingFactors->north;
	        break;
	}

	sf      = quadrantScalingFactors[quadrant];
	// Every other cardinal is the corner of a quadrant
	limits  = cardinals[(quadrant+1)*2];

	// If we're outside of our quadrant in either dimension, use the 
	// extra scaling factor instead of the quadrant scaling factor
	if (abs(x) > abs(limits[0])) sf.x = extraSf.x;
	if (abs(y) > abs(limits[1])) sf.y = extraSf.y;

	return sf;
}

int main() {
	printf("Hello!\n");
}