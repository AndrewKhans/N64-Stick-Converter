# Calibration
Point your stick to the North gate, then press Z. Point to the Northeast gate, then press Z. Continue clockwise for all 8 gates, then you're ready to game

# Potential Issues
1. Calculating cardinals in one shot instead of continuous readings that look for a max
2. Runtime, the N64 may be expecting controller updates within a specific timeframe
3. EEPROM space: ATtiny24A has 128 bytes

# Planned features
1. Pizza (affine) transform
2. Cardinal snapping: Snap to corner when within some range of corner, using dist2d

# Potential features
1. Snapback filtering
2. Spin the stick to get cardinals. Easy for North/South/East/West, hard for diagonals
3. Artificial deadzone