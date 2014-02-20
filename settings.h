#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstddef>
#include <cmath>
#include <array>
#include <vector>
#include <functional>

const size_t g_dimension = 1;

typedef double real;
typedef std::array<real,g_dimension> realarray;
typedef std::vector<real> realvector;


const real g_epsilon  = 1e-4;
const size_t g_level  = 7;
const real g_velocity = 0.5;

const real x0    = -1.0;
const real x1    = +1.0;
const real g_span  = x1 - x0;

const real g_cfl  = 0.1;
const real g_timestep = 0.002;

enum dimension_t {
      dimX = 0
};

enum boundaryCondition_t {
      bcNone        = 0
    , bcIndependent = 1
    , bcPeriodic    = 2
};

typedef std::function<real(realarray)> propertyGenerator_t;

inline size_t level2N(const size_t level) {
    return (2 << level) + 1;
}

static real inDomain(const real x)
{
    return std::fmod(std::fabs(x - x0 + g_span), g_span) + x0;
}

#endif // SETTINGS_H
