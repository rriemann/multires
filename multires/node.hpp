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

/*!
   \brief The node_t class is the base object the multi resolution tree is build upon providing the core functionality
 */
class node_t
{
public:

    /*!
       \brief The position_t enum

       Positions in the cell are ordered like this:

           NW(2) | NE(3)
           -------------
           SW(0) | SE(1)

       Orientations with respect to one cell are ordered like this:

               N(3)
                |
           W(0) - E(1)
                |
               S(2)

       There is a difference between positions and orientations. Position is
       what gives you the place in one cell. Orientation is the perspective
       if you want to query for neighbouring cells.


     */
    enum position_t {
        // according to numbering of quadrants, starting with 0
        // http://en.wikipedia.org/wiki/Octant_%28solid_geometry%29
          posRoot             =-1
        // 1D
        , posLeft             = 0
        , posRight            = 1
        , posW                = 0
        , posE                = 1
        // 2D extension
        , posTop              = 3
        , posBottom           = 2
        , posNorth            = 3
        , posSouth            = 2
        , posN                = 3
        , posS                = 2
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

    /*!
       \brief has checks is flag is set for this node
       \param flag
       \return if the flag is set
     */
    inline bool has(flag_t flag) const
    { return m_flags & flag; }

    /*!
       \brief set a specific flag for this node
       \param flag
     */
    inline void set(flag_t flag)
    { m_flags = m_flags | flag; }

    /*!
       \brief node_t constructor used for default-construction by std containers
     */
    node_t();

    /*!
       \brief setGrid sets the corresponding grid for all nodes in the grid (class/static function!)
       \param grid
     */
    static void setGrid(multires_grid_t *grid)
    { c_grid = grid; }

    /*!
       \brief setEpsilon sets the epsilon used by all nodes in the grid
       \param epsilon

       \sa g_epsilon
     */
    static void setEpsilon(real epsilon)
    { c_epsilon = epsilon; }

    /*!
       \brief initialize is actually the setup function of this object
       \param parent pointer
       \param level of this node
       \param position of this node relative to its parent
       \param index of this node counted in the level of this node
       \param point which represents the value of this node

       This functions would otherwise be integrated in the constructor, but for
       technical reasons the constructor cannot accept arguments. That's why we
       call initalize of the construction of the node object.
     */
    void initialize(node_t *parent, u_char level, char position, const index_t &index, point_t *point);

    typedef std::array<node_t, g_childs> node_array_t; //!< a number of childs, depends on g_dimension

    /*!
       \brief getNeighbour gets you the neighbour in the direction/orientation relative to this node
       \param orientation
       \return pointer to neighbouring node

       This function uses recursion and might be computational expensive.
       Worst case is probably: log(number of nodes)
     */
    const node_t *getNeighbour(const char orientation) const;
    const node_t *getParent() const
    { return m_parent; }
    /*!
       \brief getPoint returns the point associated to this node
       \return point of this node
     */
    const point_t *getPoint() const
    { return m_point; }

    /*!
       \brief getPoint returns the closes point to index which is a child of this node
       \param index
       \return point

       This function uses recursion and might be computational expensive. As only
       children of this node are considered, call this function on the root node
       to start a global search.
     */
    const point_t *getPoint(const index_t &index);

    /*!
       \brief getChild returns the child at position of this node
       \param position
       \return child node

       Make sure first that this node has children at all.

       \sa getChilds(), isLeaf()
     */
    node_t *getChild(const char position) const;

    /*!
       \brief getChilds return the pointer to the container object keeping all children
       \return pointer to children

       \note This function may be used to determine if the node has children. In this
             case the return value will be not zero (0).
     */
    node_array_t *getChilds() const;

    inline bool isLeaf() const //!< is true if this node doesn't have children
    { return !m_childs; }

    /*!
       \brief branch creates adds level more generations of children
       \param level
     */
    void branch(size_t level = 1);

    /*!
       \brief debranch removes the children from memory while keeping the linked list of all points in good order
     */
    void debranch();

    /*!
       \brief remesh_analyse recursively sets the approiate flags for all child nodes
       \return if this node got the flag **active** set
     */
    bool remesh_analyse();

    /*!
       \brief remesh_savety makes sure that all child nodes with flActive flag set have children with flSavetyZone flag set
     */
    void remesh_savety();

    /*!
       \brief remesh_clean removes all nodes from this grid which have not the flActive nor the flSavetyZone flag set
       \return if this node can be deleted by its parent
     */
    bool remesh_clean();


    /*!
       \brief interpolation
       \return field value for the center position of this node
     */
    // real interpolation() const;
    /*!
       \brief residual
       \return difference between the interpolated center of its parent and the current value of this node

       \note This function is meant to be called only by nodes in the center position
     */
    // inline real residual() const;

    void collision(const u_char k);
    void streaming(const u_char k);
    void derivation();

    inline u_char getLevel() const
    { return m_level; }

    inline u_char getFlags() const
    { return m_flags; }

    inline char getPosition() const
    { return m_position; }

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
    node_t *m_parent; //!< parent node
    u_char m_level; //!< level of this node
    char m_position; //!< position of this node relative to parent
    index_t m_index; //!< index at m_level
    u_char m_flags; //!< bunch of flags of this node
    // std::unique_ptr<point_t> m_point;
    point_t *m_point; //!< corresponding point of this node
    node_array_t *m_childs; //!< children of this node, might be null (0)
    static multires_grid_t *c_grid; //!< static pointer to multires_grid_t
    static real c_epsilon; //!< epsilon, see \ref g_epsilon
};

#endif // NODE_HPP
