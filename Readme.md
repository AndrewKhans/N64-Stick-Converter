# Big goal:
- top right corner is perfectly a 45 degree angle, it's set to those values that you defined to be what you want
- Likewise, top is 0, Ymax. 
# Pseudocode
## Calibration

Variables we need to calculate and store:
```c
uint16_t cardinals[8][2] = {
	{0, 0}, // X, Y reading for the north cardinal
	{0, 0}, // X, Y reading for the northeast cardinal
	{0, 0}, // etc, clockwise
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
}

uint8_t quadrantScalingFactors[4][2] = {
	{0, 0}, // X, Y scaling factors for yellow quadrant
	{0, 0}, // X, Y scaling factors for orange quadrant
	{0, 0}, // X, Y scaling factors for grey quadrant
	{0, 0}, // X, Y scaling factors for green quadrant
}

uint8_t extraScalingFactors[4] = {
	0, // Y factor for the triangle area to the north
	0, // X factor for the triangle area to the east
	0, // Y factor for the triangle area to the south
	0, // X factor for the triangle area to the west
}
```

1. Wait for the first button press, then save the current stick X, Y to `cardinals[0]`
2. Fill in `cardinals` iteratively with each button press
3. Calculate scaling factors
	1. Calculate each factor of `quadrantScalingFactors` based on the cardinal at the corner of that quadrant
	2. Calculate each factor of `extraScalingFactors` based on the north, east, south, and west cardinals

## Applying the Calibration
1. Read the raw 16 bit stick X and Y
2. Subtract 16 bit X and Y values from the reading, to give us X and Y coordinates that can be used with the diagram:
- Case 1: X is positive, Y is positive:
	- Start with the factors from `quadrantScalingFactors` the yellow quadrant
	- If X is outside the yellow quadrant, the X scaling factor is the **east** element of `extraScalingFactors`
	- If Y is outside the yellow quadrant, the Y scaling factor is the **north** element of `extraScalingFactors`
- Case 2: X is positive, Y is negative:
	- Start with the factors from `quadrantScalingFactors` the orange quadrant
	- If X is outside the orange quadrant, the X scaling factor is the **east** element of `extraScalingFactors`
	- If Y is outside the orange quadrant, the Y scaling factor is the **south** element of `extraScalingFactors`
- etc, for the 4 other cases


## Potential Issues
1. Calculating cardinals in one shot instead of continuous readings that look for a max
2. Runtime
3. EEPROM space: ATtiny24A has 128 bytes
	- `quadrantLimits` is (2\*2)\*4
		- If needed, we could make these values 8 bit instead of 16 bit. We would get our 16 bit reading, scale down the reading, see if the 8 bit scaled down reading is outside of the 8 bit limit value, and if it is re-scale the original 16 bit reading.
	- `quadrantScalingFactors` is 2\*4


