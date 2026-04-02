# Calibration
Point your stick to the North gate, then press Z. Point to the Northeast gate, then press Z. Continue clockwise for all 8 gates, then you're ready to game

# Potential Issues
1. Calculating cardinals in one shot instead of continuous readings that look for a max
2. Runtime, the N64 may be expecting controller updates within a specific timeframe
3. EEPROM space: ATtiny24A has 128 bytes


# Notes
- Z and R appear to be switched
    - After calibration, press R again
- Switching to the extraScalingFactor seems to make the stick jump downwards
- Right after calibration ends, we read neutral. Their stick is still pointing at the last gate
- make it so the corners can be set to a value like 82



- We want it so when we point at a corner, its (x, y) is (MAX_CORNER, MAX_CORNER)
- The function should take x1, y1 from top left and turn them into (-MAX_CORNER, MAX_CORNER)
- The function should take x2, y2 from top right and turn them into (MAX_CORNER, MAX_CORNER)
- Values in-between should move smoothly
- Two separate functions: xTranslator and yTranslator

Center is a value, but it gets translated to 0,0
- Each corner is another value that gets translated to (MAX_CORNER, MAX_CORNER) with the appropriate sign. All values between are smooth. Your distance from each corner affects how much that corner's factors matter, so when you're in a corner, it's just that corner's values