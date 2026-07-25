
#define F_CPU 1000000UL
#define MIN_RANGE_STD 101
#define MAX_RANGE_STD 105
#define MIN_RANGE_XTD 101
#define MAX_RANGE_XTD 105

#include "point_math.h"
#include "stdio.h"

/******************************************************************************
Prototypes
******************************************************************************/

// Mock
uint16_t GetX() {
    static int count = 0;
	uint16_t x_readings[] = {
		512, // Neutral x
		512, // Neutral x again for second neutral grab
		512, // North
		630, // NE
		650, // East
		630, // SE
		512, // South
		400, // SW
		380, // West
		400, // NW
		540, // Test point 1 (in northeast quadrant)
	};
	return x_readings[count++];
}

// Mock
uint16_t GetY() {
    static int count = 0;
	uint16_t y_readings[] = {
		512, // Neutral y
		512, // Neutral y again for second neutral grab
		650, // North
		630, // NE
		512, // East
		400, // SE
		380, // South
		400, // SW
		512, // West
		630, // NW
		530, // Test point 1 (in northeast quadrant)
	};
	return y_readings[count++];
}

// scales the 16 bit ADC value down to 8 bits: result = raw16 * factor c / 256
uint8_pair_t ApplyTransform(uint16_pair_t raw, uint16_pair_t neutral);


void Calibration(void);

/******************************************************************************
EEPROM Variables
******************************************************************************/

// Todo: Make these nicer to edit, from -100, 100, instead of 0, 255
#define GATE_NORTH     {0,100}
#define GATE_NORTHEAST {75,75}
#define GATE_EAST      {100,0}
#define GATE_SOUTHEAST {75,-75}
#define GATE_SOUTH     {0,-100}
#define GATE_SOUTHWEST {-75,-75}
#define GATE_WEST      {-100,0}
#define GATE_NORTHWEST {-75,75}

/* Stores the original calibration inputs */
fpair_t cardinals[8];

/* Stores the affine matricies used to transform stick readings from real to ideal */
AffineMat transformMats[8];


void Calibration(void) {
	fpair_t neutral;

	neutral.x = (float)GetX();
	neutral.y = (float)GetY();

	static const fpair_t idealGates[8] = {GATE_NORTH, GATE_NORTHEAST, GATE_EAST, GATE_SOUTHEAST, \
                      					  GATE_SOUTH, GATE_SOUTHWEST, GATE_WEST, GATE_NORTHWEST };

	for (uint8_t i = 0; i < 8; i++) {
		printf("\nCalibration i = %d\n", i);
		cardinals[i].x = GetX();
		cardinals[i].y = GetY();

		if (i == 0) {
			// With just one cardinal, we can't calculate any matricies
			continue;
		} else {
			// Build S = unit->real using neutral,p2,p3
			fpair_t p2 = cardinals[i];
			fpair_t p3 = cardinals[i-1];
			printf("neutral, p2, p3: (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)\n", neutral.x, neutral.y, p2.x, p2.y, p3.x, p3.y);
			AffineMat S = getAffineMat(neutral, p2, p3);

			fpair_t origin = {0.0f, 0.0f};
			fpair_t d2 = idealGates[i];
			fpair_t d3 = idealGates[i-1];
			printf("origin, d2, d3: (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)\n", origin.x, origin.y, d2.x, d2.y, d3.x, d3.y);
			AffineMat D = getAffineMat(origin, d2, d3);

			// M = D * S^-1 maps real -> ideal
			AffineMat S_inv = S;
			matInvert(&S_inv);
			transformMats[i-1] = matMultiply(D, S_inv);

			// After getting the 8th cardinal, calculate the 8th matrix
			if (i == 7) {
				printf("Bonus final calculation\n");
				p2 = cardinals[0];
				p3 = cardinals[7];

				printf("neutral, p2, p3: (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)\n", neutral.x, neutral.y, p2.x, p2.y, p3.x, p3.y);
				S = getAffineMat(neutral, p2, p3);
				S_inv = S;
				matInvert(&S_inv);

				printf("origin, d2, d3: (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)\n", origin.x, origin.y, d2.x, d2.y, d3.x, d3.y);
				d2 = idealGates[0];
				d3 = idealGates[7];
				D = getAffineMat(origin, d2, d3);
				transformMats[7] = matMultiply(D, S_inv);
				printf("\n");
			}
		}
	}
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


uint8_pair_t ApplyTransform(uint16_pair_t raw, uint16_pair_t neutral){
	fpair_t rawf = {(float)raw.x, (float)raw.y};
	fpair_t neutralf = {(float)neutral.x, (float)neutral.y};
	printf("ApplyTransform with point (%.2f, %.2f)\n", rawf.x, rawf.y);

	uint8_t octant = GetOctant(rawf, neutralf);
	printf("Selected octant %d\n", octant);

	AffineMat transformMat = transformMats[octant];
	fpair_t pointT = matPointMultiply(transformMat, rawf);
	printf("Result before shift and uint8 cast: (%.2f, %.2f)\n", pointT.x, pointT.y);

	// Map ideal gate coords (centered at 0) to 8-bit space with center at 128
	pointT.x += 128.0f;
	pointT.y += 128.0f;

	if (pointT.x < 0.0f)   pointT.x = 0.0f;
	if (pointT.x > 255.0f) pointT.x = 255.0f;
	if (pointT.y < 0.0f)   pointT.y = 0.0f;
	if (pointT.y > 255.0f) pointT.y = 255.0f;

	// Todo: To improve accuracy, round before casting
	uint8_pair_t ret = {(uint8_t)pointT.x, (uint8_t)pointT.y};
	printf("Final result: (%d, %d)\n", ret.x, ret.y);
	return ret;
}

int main(void)
{
	uint8_t maxRange;

	uint16_pair_t neutral16, raw;
	uint8_pair_t neutral8, old, pos;

	// get neutral position
	neutral16.x = GetX();
	neutral16.y = GetY();


	Calibration();

	// scale down neutral reading. it's fine to use the neutral reading to
	// pick the scaling factors, as they will all yield a similar result
	neutral8 = ApplyTransform(neutral16, neutral16);

	old = neutral8;

	// Actual conversion loop
		raw.x = GetX();
		raw.y = GetY();
		pos = ApplyTransform(raw, neutral16);
		printf("Transformed (%d, %d), to (%d, %d)\n", raw.x, raw.y, pos.x, pos.y);
}

