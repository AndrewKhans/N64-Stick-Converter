import csv

import matplotlib.pyplot as plt

resolution = 0.03


def generateReal():
    x_vals = []
    y_vals = []
    x = 0
    while x <= 0.39:
        y = 0

        # At this x, generate Y points down to zero
        y = (-1.3) * x + 0.9
        while y >= x:
            y -= resolution

            x_vals.append(x)
            y_vals.append(y)
        x += resolution

    return x_vals, y_vals


def generateIdeal():
    x_vals = []
    y_vals = []
    x = 0
    while x <= 0.5:
        y = 0

        # At this x, generate Y points down to zero
        y = -x + 1
        while y >= x:
            y -= resolution

            x_vals.append(x)
            y_vals.append(y)
        x += resolution
    return x_vals, y_vals


def write_csv(filename, x_vals, y_vals):
    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        for x, y in zip(x_vals, y_vals):
            writer.writerow([x, y])


def read_csv(filename):
    x_vals = []
    y_vals = []

    with open(filename, "r", newline="") as f:
        reader = csv.reader(f)
        for row in reader:
            x_vals.append(float(row[0]))
            y_vals.append(float(row[1]))

    return x_vals, y_vals


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
