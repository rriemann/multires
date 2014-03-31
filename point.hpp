/***************************************************************************************
 * Copyright (c) 2014 Robert Riemann <robert@riemann.cc>                                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef POINT_HPP
#define POINT_HPP

#include <strings.h> // provides the function ffs()
#include <boost/iterator/iterator_facade.hpp>

#include "settings.h"

/*!
   \brief The point_t class represents one point in the grid—independently of the grid type.
 */
class point_t
{
public:
    point_t() {} //!< empty constructor provided for default-constructing elements in a std container type

    /*!
       \brief constructor that initalizes \ref m_index and the physical position in space \ref m_x
       \param index of this node evaluated in the level level_max
       \param level_max

       \sa g_x0, g_span in settings.h
     */
    point_t(index_t index, const u_char level_max) :
        m_index(index)
    {
        for (u_char i = 0; i < g_dimension; ++i) {
            m_x[i] = g_x0[i] + g_span[i]/(1 << level_max)*m_index[i];
            // m_x[i] = g_x0[i] + g_lb::Cl*m_index[i];
        }
    }

    /*!
       \brief constructor that initializes \ref m_index, the physical position in space \ref m_x and a field value \ref m_phi
       \param index of this node evaluated in the level level_max
       \param level_max
       \param f_eval field generator which is used to initalize \ref m_phi

       \sa point_t(index_t, const u_char)
     */
    point_t(index_t index, u_char level_max, field_generator_t f_eval) :
        point_t(index, level_max)
    {
        updateValue(f_eval);
    }

    /*!
       \brief constructor that initializes \ref m_index, he physical position in space \ref m_x and a field value \ref m_phi
       \param index of this node evaluated in the level level_max
       \param level_max
       \param value to initialize \ref m_phi

       \sa point_t(index_t, const u_char)
     */
    point_t(index_t index, u_char level_max, const field_t &U) :
        point_t(index, level_max)
    {
        m_U = U;
    }

    void updateValue(field_generator_t f_eval)
    {
        f_eval(m_x, 0, m_U, &m_rho);
        equilibriumHelper();
        m_f = m_feq; // c++11 way if m_f and m_feq are std::array
        // std::copy(std::begin(m_feq), std::end(m_feq), std::begin(m_f)); // c++11 way for c-arrays
        // memcpy(m_f, m_feq, sizeof(m_feq)); // pure C way for c-arrays
    }

    real getUmag() const
    {
        return sqrt(pow(m_U[dimX],2)+pow(m_U[dimY],2));
    }

    void setNext(point_t *point)
    { m_next = point; }

    const point_t *getNext() const
    { return m_next; }

    void setIndex(index_t index)
    { m_index = index; }

    /*!
       \brief determines the first level where this point might be part of
       \param level_max is the maximum level on which indexing is based on
       \return the level of this node

       This function is mostly provided for debugging purposes.
     */
    int getLevel(const u_char level_max) const
    {
        int min = (1 << level_max);
        for (const size_t &ind: m_index) {
            int i = ffs(ind);
            if(i < min) min = i;
        }
        if (min == 0) {
            return 0;
        } else {
            return level_max +1 - min;
        }
    }

    index_t m_index; //!< index with respect to level_max in \ref point_t()
    location_t m_x; //!< point location in physical space

    g_lb::stancel_t m_f; //!< Particle distribution function.
    g_lb::stancel_t m_feq; //!< Particle equilibrium distribution function.
    real m_fbak;

    /*
    real Ux; //!< X-component of the fluid velocity in the computational domain.
    real Uy; //!< Y-component of the fluid velocity in the computational domain.
    */
    field_t m_U;
    real m_rho; //!< Fluid density in the computational domain.

    point_t *m_next; //!< part of the forward-only linked list throughout all points in the grid

    void equilibriumHelper() {
        real U2 = pow(m_U[dimX],2)+pow(m_U[dimY],2);
        for (u_char k = 0; k < g_lb::Nl; ++k) {
            real term1 = g_lb::ex[k]*m_U[dimX]+g_lb::ey[k]*m_U[dimY];
            m_feq[k] = g_lb::weight[k]*m_rho*(1+3*term1+4.5*term1*term1-1.5*U2);
        }
    }

    static inline void derivateMacroVariables(const g_lb::stancel_t f, field_t &value, real *rho) {
        real rsum = 0; //!< Fluid density counter.
        real usum = 0; //!< Counter of the x-component of the fluid velocity.
        real vsum = 0; //!< Counter of the y-component of the fluid velocity.
        for (u_char k = 0; k < g_lb::Nl; ++k) {
            rsum += f[k];
            usum += f[k]*g_lb::ex[k];
            vsum += f[k]*g_lb::ey[k];
        }
        *rho = rsum;
        value[dimX] = usum/rsum;
        value[dimY] = vsum/rsum;

    }

    void derivateMacroVariables() {
        derivateMacroVariables(m_f, m_U, &m_rho);
    }

    inline void collision(const u_char k) {
        m_fbak = m_f[k]*(1-g_lb::omega)+g_lb::omega*m_feq[k];
    }
};
#endif // POINT_HPP
