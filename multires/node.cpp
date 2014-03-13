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

#include <iostream>

#include "node.hpp"
#include "multires_grid.hpp"
#include "point.hpp"

node_t::node_t()
{
}

void node_t::initialize(node_t *parent, u_char level, char position, const index_t &index, point_t *point)
{
    m_parent = parent;
    m_level = level;
    m_position = position;
    m_index = index;
    m_flags = flUnset;
    m_childs = nullptr;
    m_point = point;
    /*
    std::cerr << "this pos " << int(position)
              << " this index " << m_point->m_index[0]
              << " this level " << int(m_level)
              << std::endl;
    */

    if(m_level == c_grid->m_level_max) {
        assert(m_index == m_point->m_index);
    }
}
/*!
   \brief node_t::getNeighbour
   \param position to look for the neighbour
   \return the neighbour at position with the same or smaller (coarser) one

   The implementation follows:
   https://github.com/dkolom/GALA2D/blob/master/QuadNode.cpp#L187
*/
const node_t *node_t::getNeighbour(const char direction) const
{

    static const std::array<char, 16> mm = {{
                                             /*west  0*/ 1, 0, 3, 2,
                                             /*east  1*/ 0, 1, 2, 3,
                                             /*south 2*/ 2, 0, 3, 1,
                                             /*north 3*/ 0, 2, 1, 3,
                                            }};

    // Check the parent cell's children
    if (m_position == posRoot) {
        return this;
    }

    const char off = direction*4; // offset

    if (m_position == mm[off+0]) {
        return m_parent->getChild(mm[off+1]);
    }
    if (m_position == mm[off+2]) {
        return m_parent->getChild(mm[off+3]);
    }

    const node_t* cnode = m_parent->getNeighbour(direction);

    if (cnode->isLeaf()) {
        return cnode;
    } else {
        if (m_position == mm[off+1]) {
            return cnode->getChild(mm[off+0]);
        } else {
            return cnode->getChild(mm[off+2]);
        }
    }
}

const point_t *node_t::getPoint(const index_t &index)
{
    const index_t &index_origin = m_point->m_index;
    if (!m_childs || (index == index_origin)) {
        assert(m_point);
        return m_point;
    }

    // const index_t &index_center = getChild(g_childs-1)->m_point->m_index;

    // index offset to get in the center of the cell in finest level
    const size_t offset = pow(2, c_grid->m_level_max - m_level);

    char position = 0;
    if (index[dimX] >= index_origin[dimX] + offset) position += 1;
    if (g_dimension > 1 && (index[dimY] >= index_origin[dimY] + offset)) position += 2;

    return getChild(position)->getPoint(index);
}

/*!
   \brief node_t::getChild
   \param position of the child relative to its parent
   \return the child of this node at position

   \note please make sure yourself that this is not a leaf
   \sa isLeaf()
*/
node_t *node_t::getChild(const char position) const
{
    assert(m_childs);
    return &(*m_childs)[position];
}

/*!
   \brief node_t::getChilds
   \return m_childs pointer (might be a nullptr)
*/
node_t::node_array_t *node_t::getChilds() const
{
    // assert(m_childs);
    return m_childs;
}

/*!
   \brief node_t::branch
   \param level gives the number of relative layers to append

   \note this function doesn't check if m_level_max is reached or not
*/
void node_t::branch(size_t level)
{
    if(level > 0) {
        // check if memory is not yet allocated in memory
        if(!m_childs) {
            // allocate memory for all child nodes
            m_childs = new node_array_t;

            for (size_t pos = 0; pos < g_childs; ++pos) {
                // construct node index
                index_t index_child = m_index;
                for (auto &ind: index_child) {
                    ind = 2*ind;
                }

                point_t *point;
                if (pos > 0) {
                    // create new point for position > 0
                    index_t index_point = m_point->m_index;
                    const size_t stepsize = pow(2, c_grid->m_level_max - (m_level+1));
                    const node_t *node_inter = this;
                    if (pos % 2 == 1) {
                        ++index_child[dimX];
                        index_point[dimX] += stepsize;
                        node_inter = node_inter->getNeighbour(posRight);
                    }
                    if (pos > 1) {
                        ++index_child[dimY];
                        index_point[dimY] += stepsize;
                        node_inter = node_inter->getNeighbour(posTop);
                    }
                    // phi-value interpolation
                    real phi = (m_point->m_phi + node_inter->getPoint()->m_phi)/2;

                    point = new point_t(index_point, c_grid->m_level_max, phi);
                    getChild(pos)->setPoint(point);
                } else {
                    // copy point for first child from parent (this)
                    point = m_point;
                }

                getChild(pos)->initialize(this, m_level+1, position_t(pos), index_child, point);
            }

            // overwriting phi value for center cell
            getChild(g_childs-1)->getPoint()->m_phi = interpolation();

            // put new child nodes into the next-chain of the points
            point_t *point = m_point->m_next;
            for (short pos = g_childs-1; pos > 0; --pos) {
                getChild(pos)->getPoint()->m_next = point;
                point = getChild(pos)->getPoint();
            }
            this->getPoint()->m_next = point;
        }
        for (node_t &node: *m_childs) {
            node.branch(level-1);
        }
    }
}

/*!
   \brief node_t::debranch
   \note check if m_childs is non-zero before
 */
void node_t::debranch()
{
    point_t *point = (*m_childs)[g_childs-1].getPoint()->m_next;

    delete m_childs;
    m_childs = nullptr;

    m_point->m_next = point;
}

/*!
   \brief node_t::remesh_analyse
   \return if the current node has flag flActive
*/
bool node_t::remesh_analyse()
{
    if (!has(flCached)) {
        assert(!has(flActive));

        // look for active childs
        if (m_childs) {
            for (node_t &node: *m_childs) {
                if (node.remesh_analyse()) {
                    set(flActive);
                }
            }
        }

        // check if the residual of this node
        if (!has(flActive) && (m_position == g_childs-1) && (residual() > c_epsilon)) {
            set(flActive);
        }

        // check neighbours to keep the tree graded
        if (!has(flActive)) {
            /* If there is only one neighbour of my level that has children,
               this node has to stay active.
            */

            for (u_char pos = 0; pos < g_childs; ++pos) {
                const node_t *neighbour = getNeighbour(pos);

                // check if the tree is balanced
                assert(abs(neighbour->getLevel() - m_level) < 2);

                if ((neighbour->getLevel() == m_level) && neighbour->getChilds()) {
                    u_char flags = m_flags; // members are not supported by openmp
                    #pragma omp parallel for reduction(| : flags) if (m_level < g_level_fork)
                    for (auto node = neighbour->getChilds()->begin(); node < neighbour->getChilds()->end(); ++node) {
                        if (node->remesh_analyse()) {
                            flags = flags | flActive;
                            // break; (not compatible with openmp)
                        }
                    }
                    setFlags(flags);
                }
            }
        }

        set(flCached);
    }

    return has(flActive);
}

/*!
   \brief node_t::remesh_savety adds savety zone

   This methods walks recursively through the whole tree to find the nodes
   with highest level in each branch which have a flActive flag set and creates
   child nodes (if neccessary) with flSavetyZone flag set.

   Ok, we do it differently. We just branch every node which has the flActive flag
   itself or has a sibling with it.
*/
void node_t::remesh_savety()
{
    if (m_childs && (m_level+1 < c_grid->m_level_max)) {
        // cumulative  flags of children
        u_char cum_flags = flUnset;
        #pragma omp parallel for reduction(| : cum_flags) if (m_level < g_level_fork)
        for (auto node = m_childs->begin(); node < m_childs->end(); ++node) {
            node->remesh_savety();
            cum_flags = cum_flags | node->getFlags();
        }
        if (cum_flags & flActive) {
            for (node_t &node: *m_childs) {
                node.branch();
                for (node_t &node_child: *node.getChilds()) {
                    node_child.set(flSavetyZone);
                }
            }
        }
    }
}

/*!
   \brief node_t::remesh_clean
   \return if the current node has no children
*/
bool node_t::remesh_clean()
{
    bool veto = false; // veto for removal of this node
    if (m_childs) {
        #pragma omp parallel for reduction(|| : veto) if (m_level < g_level_fork)
        for (auto node = m_childs->begin(); node < m_childs->end(); ++node) {
            if (!node->remesh_clean()) {
                veto = veto || true;
            }
        }
        if (!veto) {
            debranch();
        }
    }
    bool ret = (!veto && (m_flags < flSavetyZone));
    m_flags = flUnset;
    return ret;
}

real node_t::interpolation() const
{
    /*
    real phi = 0;
    for (char pos = 0; pos < g_childs; ++pos) {
        phi += getNeighbour(pos)->getPoint()->m_phi;
    }
    return phi/g_childs;
    */

    // return (m_parent->getPoint()->m_phi + getNeighbour(1)->getNeighbour(2)->getPoint()->m_phi)/2;

    real phi = m_point->m_phi;
    for (size_t pos = 1; pos < g_childs; ++pos) {
        const node_t *node_inter = this;
        if (pos % 2 == 1) {
            node_inter = node_inter->getNeighbour(posRight);
        }
        if (pos > 1) {
            node_inter = node_inter->getNeighbour(posTop);
        }
        // phi-value interpolation
        phi += node_inter->getPoint()->m_phi;
    }

    return phi/g_childs;
}

real node_t::residual() const
{
    assert(m_position == g_childs-1);
    return fabs(m_point->m_phi - m_parent->interpolation());
}

void node_t::updateFlow(const char direction)
{
    if(isLeaf()) {
        const real phi_this = m_point->m_phi;

        std::array<real,  g_childs> phi_neighbour;
        // u_char level_diff_max = 0;
        for (char pos = 0; pos < g_childs; ++pos) {
            const node_t *neighbour = getNeighbour(pos);
            /* as we work with graded trees, we can expect that the level of our
               neighbours is either the same or one level smaller (coarser).
            */
            assert(abs(neighbour->getLevel() - m_level) < 2);
            real phi = neighbour->getPoint()->m_phi;
            if (neighbour->getLevel() < m_level) {
                // if the left neighour cell in coarser, we have to interpolate
                // its value to be comparable with the other values
                phi = (phi+phi_this)/2;

            } else if (neighbour->getChilds()) {
                // the left neighbour is finer!
                assert(g_dimension < 3);
                static const std::array<u_char, 8> faces = {{ /*W(0)*/ 1, 3, /*E(1)*/ 0, 2, /*S(2)*/ 2, 3, /*N(3)*/ 0, 1}};
                for (u_char face_pos = direction; face_pos < pow(2, g_dimension-1); ++face_pos) {
                    phi = neighbour->getChild(faces[face_pos])->getPoint()->m_phi;
                    phi = 2*phi-phi_this; // extrapolating
                }
            }
            phi_neighbour[pos] = phi;
        }

        const real dx = g_span[dimX]/(1 << m_level);

        m_point->m_flow = flowHelper(phi_this, phi_neighbour[direction-1], phi_neighbour[direction], dx, c_grid->dt);
    } else {
        #pragma omp parallel for if (m_level < g_level_fork)
        for (auto node = m_childs->begin(); node < m_childs->end(); ++node) {
            node->updateFlow(direction);
        }
    }
}

void node_t::timeStep(const char direction)
{
    if(isLeaf()) {
        const real flow_this = m_point->m_flow;

        real flow_income = 0;
        // u_char level_diff_max = 0;
        const node_t *neighbour = getNeighbour(direction-1);
        /* as we work with graded trees, we can expect that the level of our
           neighbours is either the same or one level smaller (coarser).
        */
        assert(abs(neighbour->getLevel() - m_level) < 2);
        constexpr u_char dimensionFactor = 1 << (g_dimension - 1);
        if (neighbour->getLevel() == m_level){
            flow_income = neighbour->getPoint()->m_flow;
        } else if (neighbour->getLevel() < m_level) {
            // assert(m_position == 0);
            // if the left neighour cell in coarser, we have to interpolate
            // its value to be comparable with the other values
            flow_income = neighbour->getPoint()->m_flow/dimensionFactor; // *2
        }  else {
            // gather flow from children
            for (u_char pos = 0; pos < g_dimension; ++pos) {
                flow_income += neighbour->getChild(direction+pos*2)->getPoint()->m_flow; // /2
            }
        }

        flow_income = neighbour->getPoint()->m_flow;
        const real dx = g_span[dimX]/(1 << m_level);

        m_point->m_phi += timeStepHelperFlow(flow_this, flow_income, dx, c_grid->dt);
    } else {
        #pragma omp parallel for if (m_level < g_level_fork)
        for (auto node = m_childs->begin(); node < m_childs->end(); ++node) {
            node->timeStep(direction);
        }
    }
}

node_t::~node_t()
{
    if (m_childs) {
        debranch();
    }

    // we delete the position pointers except the one we got from parent
    if(m_position > position_t(0)) {
        delete m_point;
        // m_point = nullptr;
    }
}

multires_grid_t *node_t::c_grid = nullptr;
real node_t::c_epsilon = 0;
