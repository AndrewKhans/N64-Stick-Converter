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
	0, // Y factor for the north area above the yellow/green quadrants
	0, // X factor for east
	0, // Y factor for south
	0, // X factor for west
}
```

1. Wait for the first button press, then save the current stick X, Y to `cardinals[0]`
2. Fill in `cardinals` iteratively with each button press
3. Calculate scaling factors
	1. Calculate each factor of `quadrantScalingFactors` based on the cardinal at the corner of that quadrant
	2. Calculate each factor of `extraScalingFactors` based on the north, east, south, and west cardinals

## Applying the Calibration
Read the current stick X and Y, and select scaling factors like this:
- Case 1: X is positive, Y is positive:
	- Start with the factors from `quadrantScalingFactors` the yellow quadrant
	- If X is outside the yellow quadrant, the X scaling factor is the **east** element of `extraScalingFactors`
	- If Y is outside the yellow quadrant, the Y scaling factor is the **north** element of `extraScalingFactors`
- Case 2: X is positive, Y is negative:
	- Start with the factors from `quadrantScalingFactors` the orange quadrant
	- If X is outside the orange quadrant, the X scaling factor is the **east** element of `extraScalingFactors`
	- If Y is outside the orange quadrant, the Y scaling factor is the **south** element of `extraScalingFactors`
- etc
