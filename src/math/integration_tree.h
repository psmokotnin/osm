/**
 *  OSM
 *  Copyright (C) 2023  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MATH_INTEGRATION_TREE_H
#define MATH_INTEGRATION_TREE_H

#include <cmath>
#include <vector>
#include "common/atomic.h"

namespace math {

struct integration_tree {

    integration_tree(std::size_t size, std::size_t depth);

    float value() const;
    void add_value(float value);

    std::size_t size() const;
    void setSize(std::size_t size);
    void reset();

private:
    struct node {
        node();
        node(node *left, node *right);

        void setBound(node *left, node *right);

        float value() const;
        void set_value(float value);

        void update();

    private:
        Atomic<float> m_value = 0;
        node *m_left  = nullptr;
        node *m_right = nullptr;
        node *m_top   = nullptr;
    };

    Atomic<std::size_t> m_size = 0;
    Atomic<std::size_t> m_pos = 0;
    std::vector<std::vector<node>> data;
    node m_top;
};

} // namespace math

#endif // MATH_INTEGRATION_TREE_H
