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


/**
 * @brief regular_base::timeStepHelper
 * @param ee element to calculate new value for
 * @param el element to the left
 * @param er element to the right
 * @return new value for element
 */
inline real timeStepHelper(const real &ee, const real &el, const real &er, const real &dx, const real &dt)
{
#ifdef BURGER
    // u_j+0.5
    const real ujp = 0.5*(er+ee)-(er+ee)*dt/(4*dx)*(er-ee);
    // u_j-0.5
    const real ujm = 0.5*(el+ee)-(el+ee)*dt/(4*dx)*(ee-el);

    // http://www.exp.univie.ac.at/cp1/cp1-6/node72.html (closed form)
    // time step with Lax-Wendroff method
    // const real property = ee - (ujp+ujm)*dt/(4*dx)*(ujp-ujm);

    // Godunov method (finite volume), visoucous burgers eqn
    static const real viscosity = 0.005;
    const real property = ee + dt*(viscosity*(er-2*ee+el)/(dx*dx)+(-pow(er+ee,2)+pow(el+ee,2))/(8*dx));

#else
    const real alpha = g_velocity*dt/dx;
    const real property = ee - alpha/2*(er-el-alpha*(er-2*ee+el));
#endif
    return property;
}

#endif // SETTINGS_H
