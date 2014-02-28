/***************************************************************************************
 * Copyright (c) 2013 Robert Riemann <robert@riemann.cc>                                *
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

#ifndef NODE_HPP
#define NODE_HPP

#include <memory>

#include "settings.h"

class multires_grid_t;
class point_t;

class node_t
{
public:
    enum position_t {
        // according to numbering of quadrants, starting with 0
        // http://en.wikipedia.org/wiki/Octant_%28solid_geometry%29
          posRoot             =-1
        // 1D
        , posLeft             = 0
        , posRight            = 1
        , posTopRightFront    = 0
        , posTopRightBack     = 1
        // 2D extension
        /*
        , posTopLeftBack      = 2
        , posTopLeftFront     = 3
        */
        // 3D extension
        /*
        , posBottomRightFront = 4
        , posBottomRightBack
        , posBottomLeftBack
        , posBottomLeftFront
        */
        , posSW = 0
        , posSE = 1
        , posNW = 2
        , posNE = 3
    };
    enum level_t {
          lvlNoChilds =  0
        , lvlRoot     =  0
        , lvlFirst    = 1
    };

    // this is designed to allow: if(type>typeVirtual) { eval(); }
    enum flag_t {
          flUnset      = 0
        , flCached     = 1 << 0
        , flDeletable  = 1 << 1
        , flVirtual    = 1 << 2
        , flSavetyZone = 1 << 3
        , flActive     = 1 << 4
    };

    inline bool has(flag_t flag) const
    { return m_flags & flag; }

    inline void set(flag_t flag)
    { m_flags = m_flags | flag; }

    node_t();

    static void setGrid(multires_grid_t *grid)
    { c_grid = grid; }

    static void setEpsilon(real epsilon)
    { c_epsilon = epsilon; }

    void initialize(node_t *parent, u_char level, char position, const index_t &index);


    static const short c_childs = (1 << g_dimension);
    typedef std::array<node_t, c_childs> node_array_t;

    const node_t *getNeighbour(const char position) const;

    node_t *getChild(const char position) const;
    node_array_t *getChilds() const;

    inline bool isLeaf() const
    { return !m_childs; }

    void branch(size_t level = 1);

    void debranch();

    bool remesh_analyse();
    void remesh_savety();
    bool remesh_clean();

    real interpolation() const;
    inline real residual() const;

    void timeStep();

    const point_t *getPoint() const
    { return m_point; }

    inline u_char getLevel() const
    { return m_level; }

    inline u_char getFlags() const
    { return m_flags; }

    inline void setFlags(u_char flags)
    { m_flags = flags; }

    point_t *getPoint()
    { return m_point; }

    void setPoint(point_t *point)
    { m_point = point; }

    const index_t &getIndex() const
    { return m_index; }

    ~node_t();

private:
    node_t *m_parent;
    u_char m_level;
    char m_position;
    index_t m_index; // index at m_level
    u_char m_flags;
    // std::unique_ptr<point_t> m_point;
    point_t *m_point;
    node_array_t *m_childs;
    static multires_grid_t *c_grid;
    static real c_epsilon;
};

#endif // NODE_HPP
