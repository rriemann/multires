#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstddef>
#include <cmath>
#include <array>
#include <vector>
#include <functional>
#include <limits>
#include <cassert>
#ifdef _OPENMP
#include <omp.h>
#endif

constexpr u_char g_dimension = 2;
constexpr short  g_childs = (1 << g_dimension);
#ifdef _OPENMP
const u_char g_level_fork = log(omp_get_num_procs())/log(g_childs);
#endif

typedef double real;
typedef std::array<real, g_dimension> location_t;
typedef std::vector<real> real_vector;
typedef std::array<real, g_childs> environment_t;

typedef std::array<size_t, g_dimension> index_t;

const real g_epsilon  = 4e-3;
const size_t g_level  = 7;
const real g_velocity = 0.5;
const real g_eps = std::numeric_limits<real>::epsilon();

const real g_cfl  = 0.1;
const real g_timestep = 0.0001;

const location_t g_x0 = {{0, 0}};
const location_t g_x1 = {{1, 1}};
const location_t g_span = {{g_x1[0] - g_x0[0],
                            g_x1[1] - g_x0[1]}};

enum dimension_t {
      dimX = 0
    , dimY = 1
    , dimZ = 2
};

typedef std::function<real(location_t)> field_generator_t;

/**
 * @brief regular_base::timeStepHelper
 * @param ee element to calculate new value for
 * @param el element to the left
 * @param er element to the right
 * @return new value for element
 */
inline real timeStepHelper(const real &ee, const real &el, const real &er,
                           const real &dx, const real &dt)
{
    const real alpha = g_velocity*dt/dx;
    const real property = ee - alpha/2*(er-el-alpha*(er-2*ee+el));

    assert(property > -2 && property < 2);

    return property;
}

inline real timeStepHelperFlow(const real &flow, const real &flow_left, const real &dx, const real &dt) {
    return - (dt/dx)*g_velocity*(flow - flow_left);
}

inline real minmod(const real a, const real b)
{
    if (a*b > 0) {
        if (fabs(a) < fabs(b)){
            return a;
        } else {
            return b;
        }
    } else {
        return 0;
    }
}

inline real flowHelper(const real &ee, const real &el, const real &er,
                            const real &dx, const real &dt)
{
#ifdef LIMITER
   const real derivative = minmod(ee - el, er - ee);
#else
   const real derivative = (er-el)/2;
#endif
   real a_L = ee + 0.5*(1-dt/dx*g_velocity)*derivative;
   return a_L;
}

#endif // SETTINGS_H
