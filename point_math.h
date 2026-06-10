/*
Todo:
*/

#ifndef POINT_MATH_H
#define POINT_MATH_H

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef struct AffineMat {
    float a, b, tx, c, d, ty;
} AffineMat;

typedef struct {
	float x;
	float y;
} fpair_t;

typedef struct {
	uint16_t x;
	uint16_t y;
} pair16_t;

typedef struct {
	uint8_t x;
	uint8_t y;
} pair8_t;

#endif