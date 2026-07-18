import csv

import matplotlib.pyplot as plt

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


f1 = plt.figure()
ax1 = f1.add_subplot(111)
x, y = generateReal()
ax1.set_xlim(0, 1)
ax1.set_ylim(0, 1)
ax1.set_title("Real points")
ax1.scatter(x, y)
write_csv("real.csv", x, y)

f2 = plt.figure()
ax2 = f2.add_subplot(111)
x, y = generateIdeal()
ax2.set_xlim(0, 1)
ax2.set_ylim(0, 1)
ax2.set_title("Ideal points")
ax2.scatter(x, y)


f3 = plt.figure()
ax3 = f3.add_subplot(111)
x, y = read_csv("out.csv")
ax3.set_xlim(0, 1)
ax3.set_ylim(0, 1)
ax3.set_title("Real -> Ideal Transformed Points")
ax3.scatter(x, y)

plt.show()
