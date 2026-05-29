#include "point_math.h"
#include "stdio.h"

/*
    Get an affine matrix that maps the unit triangle to the triangle made
    from p1, p2, and p3, where p1 is treated as the origin
*/
AffineMat getAffineMat(pair16_t p1, pair16_t p2, pair16_t p3) {
    AffineMat m;

    m.a = (float)p2.x - (float)p1.x;
    m.b = (float)p3.x - (float)p1.x;
    m.c = (float)p2.y - (float)p1.y;
    m.d = (float)p3.y - (float)p1.y;
    m.tx = p1.x;
    m.ty = p1.y;

    return m;
}

void matInvert(AffineMat *m) {
    float det = (m->a*m->d) - (m->b*m->c);
    float tmp;

    tmp = m->tx;
    m->tx = (m->b*m->ty - m->d*m->tx)/det;
    m->ty = (m->c*tmp   - m->a*m->ty)/det;

    tmp = m->a;
    m->a = m->d/det;
    m->d = tmp/det;

    m->b = -m->b/det;
    m->c = -m->c/det;
}

AffineMat matMultiply(AffineMat m1, AffineMat m2) {
    AffineMat p;

    p.a  = m1.a*m2.a  + m1.b*m2.c;
    p.b  = m1.a*m2.b  + m1.b*m2.d;
    p.tx = m1.a*m2.tx + m1.b*m2.ty + m1.tx;
    p.c  = m1.c*m2.a  + m1.d*m2.c;
    p.d  = m1.c*m2.b  + m1.d*m2.d;
    p.tx = m1.c*m2.tx + m1.d*m2.ty + m1.ty;

    return p;
}

pair8_t matPointMultiply(AffineMat m, pair16_t p) {
    pair8_t ret;

    ret.x = m.a*p.x + m.b*p.y + m.tx;
    ret.y = m.c*p.x + m.d*p.y + m.ty;

    return ret;
}

void printMat(AffineMat m) {
    printf("%.2f %.2f %.2f\n", m.a, m.b, m.tx);
    printf("%.2f %.2f %.2f\n", m.c, m.d, m.ty);
    printf("%.2f %.2f %.2f\n\n", 0.0f, 0.0f, 1.0f);
}

int main() {
    printf("Beginning point_math tests\n");

    // AffineMat unitMatrix = getAffineMat((pair16_t){0,0}, (pair16_t){1,0}, (pair16_t){0,1});
    // matInvert(unitMatrix);
    // printMat(unitMatrix);

    // AffineMat m1 = {2, 9, 3, 4, 5, 8};
    // printMat(m1);
    // matInvert(&m1);
    // printMat(m1);

    AffineMat m2 = {254, 232, 389, 413, 574, 193};
    printMat(m2);
    matInvert(&m2);
    printMat(m2);
}