# Compilation
avr-gcc -mmcu=attiny24a -O1 -g point_math.c "N64 Stick Converter PCB.c"

# Calibration
Point your stick to the North gate, then press Z. Point to the Northeast gate, then press Z. Continue clockwise for all 8 gates, then you're ready to game

# Potential Issues
1. Calculating cardinals in one shot instead of continuous readings that look for a max
2. Runtime, the N64 may be expecting controller updates within a specific timeframe
3. EEPROM space: ATtiny24A has 128 bytes. Possible optimizations:
    - Have matrix functions take in a matrix pointer to perform operations in-place

# Affine Version
## Planned features
1. Pizza (affine) transform
2. Cardinal snapping: Snap to corner when within some range of corner, using dist2d
3. Manually have the user define each of the cardinals in the ideal triangle for the transforms

## Potential features
1. Snapback filtering
2. Spin the stick to get cardinals. Easy for North/South/East/West, hard for diagonals
    - Maybe it is finding the largest vector that isn't north?
3. Artificial deadzone

## Questions
1. Can we use it to scale 16 bit number to 8 bit?
2. How to get the corners of the real triangle?
    - Corner by corner calibration
3. How to determine which quadrant we're in?
    - We need to save raw cardinals

# Scaling Factor Version
## Issues
- Right after calibration ends, we read neutral. Their stick is still pointing at the last gate.
    - Maybe just use have calibration function return the original neutral, so the main code can use that
- make it so the corners can be set to a value like 82
