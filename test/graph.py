import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
import math

# --- Editable test points (top of script for easy editing) ---
# Raw test point plotted on the ADC/raw graph
TEST_POINT_RAW = (540, 530)  # <-- edit this value to try different test points

# Ideal test point plotted on the ideal graph (origin at 0,0)
IDEAL_TEST_POINT = (18, 11)  # <-- edit this value to try different ideal points

# Ideal gate definitions (from N64 Stick Converter PCB macros)
# #define GATE_NORTH     {0,100}
# #define GATE_NORTHEAST {75,75}
# #define GATE_EAST      {100,0}
# #define GATE_SOUTHEAST {75,-75}
# #define GATE_SOUTH     {0,-100}
# #define GATE_SOUTHWEST {-75,-75}
# #define GATE_WEST      {-100,0}
# #define GATE_NORTHWEST {-75,75}
IDEAL_CARDINALS = [
    (0, 100),  # N
    (75, 75),  # NE
    (100, 0),  # E
    (75, -75), # SE
    (0, -100), # S
    (-75, -75),# SW
    (-100, 0), # W
    (-75, 75), # NW
]

# Raw readings (ADC values)
# Note: test point removed from the end and placed at TEST_POINT_RAW above
x_readings = [
    512,  # Neutral x
    512,  # Neutral x again for second neutral grab
    512,  # North
    630,  # NE
    650,  # East
    630,  # SE
    512,  # South
    400,  # SW
    380,  # West
    400,  # NW
]

y_readings = [
    512,  # Neutral y
    512,  # Neutral y again for second neutral grab
    650,  # North
    630,  # NE
    512,  # East
    400,  # SE
    380,  # South
    400,  # SW
    512,  # West
    630,  # NW
]

# Build point tuples
points = [(x_readings[i], y_readings[i]) for i in range(min(len(x_readings), len(y_readings)))]

# Neutral is the first reading
neutral_raw = points[0]
# Cardinals are indices 2..9 (N, NE, E, SE, S, SW, W, NW)
cardinal_raw = points[2:10]

# Use raw readings directly (no normalization)
neutral = neutral_raw
cardinals = cardinal_raw

# ----------------- Raw ADC graph -----------------
fig, ax = plt.subplots(figsize=(6, 6))
ax.set_aspect('equal')

# Compute axis limits from raw data with a small padding so the scale looks good
all_x = [p[0] for p in [neutral] + cardinals + [TEST_POINT_RAW]]
all_y = [p[1] for p in [neutral] + cardinals + [TEST_POINT_RAW]]
min_x, max_x = min(all_x), max(all_x)
min_y, max_y = min(all_y), max(all_y)

# Add 6% padding or at least 1 unit
pad_x = max(1.0, (max_x - min_x) * 0.06)
pad_y = max(1.0, (max_y - min_y) * 0.06)
ax.set_xlim(min_x - pad_x, max_x + pad_x)
ax.set_ylim(min_y - pad_y, max_y + pad_y)
ax.set_title('Octagon with triangular slices (raw ADC values)')

# Draw triangular slices (filled)
colors = plt.cm.tab10
for i in range(len(cardinals)):
    a = cardinals[i]
    b = cardinals[(i + 1) % len(cardinals)]
    tri = Polygon([neutral, a, b], closed=True, facecolor=colors(i % 10), alpha=0.45, edgecolor='k')
    ax.add_patch(tri)

# Draw octagon outline connecting cardinals
octagon = Polygon(cardinals, closed=True, fill=False, edgecolor='black', linewidth=1.5)
ax.add_patch(octagon)

# Mark neutral and cardinals
ax.scatter([neutral[0]], [neutral[1]], c='black', s=40, zorder=10, label=f'neutral {neutral}')
cxs, cys = zip(*cardinals)
ax.scatter(cxs, cys, c='red', s=30, zorder=10, label='cardinals')

# Plot the editable test point on the raw graph
ax.scatter([TEST_POINT_RAW[0]], [TEST_POINT_RAW[1]], c='blue', s=80, marker='X', zorder=20, label=f'test point {TEST_POINT_RAW}')
ax.annotate('TEST', TEST_POINT_RAW, textcoords='offset points', xytext=(6, 6), color='blue')

# Annotate cardinal directions with their raw coordinates
labels = ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW']
for (x, y), lab in zip(cardinals, labels):
    ax.annotate(f"{lab}\n({x},{y})", (x, y), textcoords='offset points', xytext=(4, 4))

ax.legend(loc='upper right')

# ----------------- Ideal graph (origin at 0,0) -----------------
fig2, ax2 = plt.subplots(figsize=(6, 6))
ax2.set_aspect('equal')

origin = (0, 0)
ideal_cardinals = IDEAL_CARDINALS

# Compute axis limits for ideal graph with padding
all_x2 = [p[0] for p in [origin] + ideal_cardinals + [IDEAL_TEST_POINT]]
all_y2 = [p[1] for p in [origin] + ideal_cardinals + [IDEAL_TEST_POINT]]
min_x2, max_x2 = min(all_x2), max(all_x2)
min_y2, max_y2 = min(all_y2), max(all_y2)
pad_x2 = max(1.0, (max_x2 - min_x2) * 0.06)
pad_y2 = max(1.0, (max_y2 - min_y2) * 0.06)
ax2.set_xlim(min_x2 - pad_x2, max_x2 + pad_x2)
ax2.set_ylim(min_y2 - pad_y2, max_y2 + pad_y2)
ax2.set_title('Ideal octagon (origin at 0,0)')

# Draw triangular slices from origin to adjacent ideal cardinals
for i in range(len(ideal_cardinals)):
    a = ideal_cardinals[i]
    b = ideal_cardinals[(i + 1) % len(ideal_cardinals)]
    tri = Polygon([origin, a, b], closed=True, facecolor=colors(i % 10), alpha=0.35, edgecolor='k')
    ax2.add_patch(tri)

# Draw ideal octagon outline
oct2 = Polygon(ideal_cardinals, closed=True, fill=False, edgecolor='black', linewidth=1.5)
ax2.add_patch(oct2)

# Mark origin and ideal cardinals
ax2.scatter([origin[0]], [origin[1]], c='black', s=40, zorder=10, label='origin (0,0)')
icxs, icys = zip(*ideal_cardinals)
ax2.scatter(icxs, icys, c='green', s=30, zorder=10, label='ideal cardinals')

# Plot editable ideal test point
ax2.scatter([IDEAL_TEST_POINT[0]], [IDEAL_TEST_POINT[1]], c='magenta', s=80, marker='D', zorder=20, label=f'ideal test {IDEAL_TEST_POINT}')
ax2.annotate('IDEAL_TEST', IDEAL_TEST_POINT, textcoords='offset points', xytext=(6, 6), color='magenta')

# Annotate ideal labels
for (x, y), lab in zip(ideal_cardinals, labels):
    ax2.annotate(lab, (x, y), textcoords='offset points', xytext=(4, 4))

ax2.legend(loc='upper right')

plt.show()
