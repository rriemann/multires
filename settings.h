#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstddef>
// #include <cstdint>

typedef double real;
#define DIMENSION 1
#define EPSILON 0.001

const real velocity = 1.0;
const real timestep = 0.1;

const size_t level = 16;
const real x0    = -1.0;
const real x1    = +1.0;

#endif // SETTINGS_H
