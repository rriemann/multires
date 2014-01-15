#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstddef>
#include <array>

#define DIMENSION 1
#define EPSILON 0.001

const size_t g_dimension = DIMENSION;

typedef double real;
typedef std::array<real,g_dimension> realarray;

const size_t g_level = 14;
const real g_velocity = 0.5;

const real x0    = -1.0;
const real x1    = +1.0;

const real g_cfl  = 0.2;
const real g_timestep = 0.005;

#endif // SETTINGS_H
