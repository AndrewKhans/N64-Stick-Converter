#ifndef POINT_MATH_H
#define POINT_MATH_H

#include <avr/io.h>
// #include <stdint.h>

#define FLOAT_COMPARE_EPS 1e-5f

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
} uint16_pair_t;

typedef struct {
	uint8_t x;
	uint8_t y;
} uint8_pair_t;

AffineMat getAffineMat(fpair_t p1, fpair_t p2, fpair_t p3);
void matInvert(AffineMat *m);
AffineMat matMultiply(AffineMat m1, AffineMat m2);
fpair_t matPointMultiply(AffineMat m, fpair_t p);

#endif