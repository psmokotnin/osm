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

#include "integration_tree.h"

namespace math {

integration_tree::integration_tree(std::size_t size, std::size_t depth) : m_size(size)
{
    data.resize(depth);
    for (std::size_t i = 0; i < depth; ++i) {
        data[i].reserve(std::pow(2, depth - i));

        for (std::size_t j = 0; j < data[i].capacity(); ++j) {
            if (i == 0) {
                data[i].emplace_back();
            } else {
                data[i].emplace_back(&data[i - 1][j * 2], &data[i - 1][j * 2 + 1]);
            }
        }
    }

    m_top.setBound(&data[depth - 1][0], &data[depth - 1][1]);
}

float integration_tree::value() const
{
    return m_top.value();
}

void integration_tree::add_value(float value)
{
    ++m_pos;
    if (m_pos == m_size) {
        m_pos = 0;
    }

    data[0][m_pos].set_value(value);
}

std::size_t integration_tree::size() const
{
    return m_size;
}

void integration_tree::setSize(std::size_t size)
{
    m_size = size;
    reset();
}

void integration_tree::reset()
{
    m_pos = 0;
    for (std::size_t i = 0; i < data[0].size(); ++i) {
        data[0][i].set_value(0);
    }
}

integration_tree::node::node() {}
integration_tree::node::node(node *left, node *right) : m_left(left), m_right(right)
{
    m_left->m_top = this;
    m_right->m_top = this;
}

void integration_tree::node::setBound(node *left, node *right)
{
    m_left         = left;
    m_right        = right;
    m_left->m_top  = this;
    m_right->m_top = this;
}

float integration_tree::node::value() const
{
    return m_value;
}

void integration_tree::node::set_value(float value)
{
    m_value = value;
    if (m_top) {
        m_top->update();
    }
}

void integration_tree::node::update()
{
    set_value(m_left->m_value + m_right->m_value);
}

} // namespace math
