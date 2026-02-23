# Big goal:
- top right corner is perfectly a 45 degree angle, it's set to those values that you defined to be what you want
- Likewise, top is 0, Ymax. 
# Pseudocode
## Calibration

Variables used:
```c
`uint18_t cardinals[8][2] = {
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0}
};`

`uint8_t quadrantScalingFactors[4][2]`
`uint8_t extraScalingFactors[4] = {
	Y factor for north
	X factor for east
	Y factor for south
	X factor for west

}
```

1. Wait for the button press. Once it's pressed, save the current stick X, Y to `Cardinals`
2. Calculate scaling factors
	1. We need 4 x,y scaling factor pairs, for the yellow/orange/grey/green quadrants
	2. We need 4 extra x or y scaling factors, for when we're in the purple/blue zones
		- Note: In these zones, we can keep using the scaling factor from the nearest quadrant, for the smaller of x or y. Ex: When we are in the North purple zone, we still use the yellow quadrant's X scaling factor, but we switch to a different Y one

## Applying the Calibration
+x, +y:
	- Use X from scalingFactors[0]
	- if y < y from cardinals[0], use y from scalingFactors[0] else use extraScalingFactors[0]
+x, -y

## Testing
- Diff my changes with the original
- Make mocks for the missing libs


## Questions
