#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <functional>
#include <utility>

void swap(inexor::vulkan_renderer::world::Cube &lhs, inexor::vulkan_renderer::world::Cube &rhs) noexcept {
    std::swap(lhs.m_type, rhs.m_type);
    std::swap(lhs.m_size, rhs.m_size);
    std::swap(lhs.m_position, rhs.m_position);
    std::swap(lhs.m_parent, rhs.m_parent);
    std::swap(lhs.m_indentations, rhs.m_indentations);
    std::swap(lhs.m_childs, rhs.m_childs);
    std::swap(lhs.m_polygon_cache, rhs.m_polygon_cache);
    std::swap(lhs.m_polygon_cache_valid, rhs.m_polygon_cache_valid);
}

namespace inexor::vulkan_renderer::world {
void Cube::remove_childs() {
    for (auto &child : m_childs) {
        child->remove_childs();
        child.reset();
    }
}

std::shared_ptr<Cube> Cube::root() noexcept {
    std::shared_ptr<Cube> new_parent = m_parent.lock();
    if (!new_parent) {
        return shared_from_this();
    }
    std::shared_ptr<Cube> parent;
    while (new_parent) {
        parent = new_parent;
        new_parent = parent->m_parent.lock();
    }
    return parent;
}

std::array<glm::vec3, 8> Cube::vertices() const noexcept {
    assert(m_type == Type::SOLID || m_type == Type::NORMAL);

    const glm::vec3 pos = m_position;
    const glm::vec3 max = {m_position.x + m_size, m_position.y + m_size, m_position.z + m_size};

    if (m_type == Type::SOLID) {
        return {{{pos.x, pos.y, pos.z},
                 {pos.x, pos.y, max.z},
                 {pos.x, max.y, pos.z},
                 {pos.x, max.y, max.z},
                 {max.x, pos.y, pos.z},
                 {max.x, pos.y, max.z},
                 {max.x, max.y, pos.z},
                 {max.x, max.y, max.z}}};
    }
    if (m_type == Type::NORMAL) {
        const float step = m_size / Indentation::MAX;
        const std::array<Indentation, Cube::EDGES> &ind = m_indentations;

        return {{{pos.x + static_cast<float>(ind[0].start()) * step, pos.y + static_cast<float>(ind[1].start()) * step,
                  pos.z + static_cast<float>(ind[2].start()) * step},
                 {pos.x + static_cast<float>(ind[9].start()) * step, pos.y + static_cast<float>(ind[4].start()) * step,
                  max.z - static_cast<float>(ind[2].end()) * step},
                 {pos.x + static_cast<float>(ind[3].start()) * step, max.y - static_cast<float>(ind[1].end()) * step,
                  pos.z + static_cast<float>(ind[11].start()) * step},
                 {pos.x + static_cast<float>(ind[6].start()) * step, max.y - static_cast<float>(ind[4].end()) * step,
                  max.z - static_cast<float>(ind[11].end()) * step},
                 {max.x - static_cast<float>(ind[0].end()) * step, pos.y + static_cast<float>(ind[10].start()) * step,
                  pos.z + static_cast<float>(ind[5].start()) * step},
                 {max.x - static_cast<float>(ind[9].end()) * step, pos.y + static_cast<float>(ind[7].start()) * step,
                  max.z - static_cast<float>(ind[5].end()) * step},
                 {max.x - static_cast<float>(ind[3].end()) * step, max.y - static_cast<float>(ind[10].end()) * step,
                  pos.z + static_cast<float>(ind[8].start()) * step},
                 {max.x - static_cast<float>(ind[6].end()) * step, max.y - static_cast<float>(ind[7].end()) * step,
                  max.z - static_cast<float>(ind[8].end()) * step}}};
    }
    return {};
}

/// 90 degree rotation.
template <>
void Cube::rotate<1>(const RotationAxis::Type &axis) {
    // the reorder function can be replaced by a lambda and used both cases.
    // requires: constexpr vector
    if (m_type == Type::NORMAL) {
        const RotationAxis::EdgeType &edge_rotation = std::get<1>(axis);
        for (const auto &order : edge_rotation) {
            std::swap(m_indentations[order[0]], m_indentations[order[1]]);
            std::swap(m_indentations[order[1]], m_indentations[order[2]]);
            std::swap(m_indentations[order[2]], m_indentations[order[3]]);
        }
        // Some indentations need to be mirrored, as the direction has changed.
        // not the last array, as it contains the edges parallel to the axis around which we rotate
        for (int idx = 0; idx < edge_rotation.size() - 1; idx++) {
            m_indentations[edge_rotation[idx][0]].mirror();
            m_indentations[edge_rotation[idx][2]].mirror();
        }
        return;
    }
    if (m_type == Type::OCTANT) {
        const RotationAxis::ChildType &child_rotation = std::get<0>(axis);
        for (const auto &order : child_rotation) {
            std::swap(m_childs[order[0]], m_childs[order[1]]);
            std::swap(m_childs[order[1]], m_childs[order[2]]);
            std::swap(m_childs[order[2]], m_childs[order[3]]);
        }
        for (auto &child : m_childs) {
            child->rotate<1>(axis);
        }
    }
}

/// 180 degree rotation.
template <>
void Cube::rotate<2>(const RotationAxis::Type &axis) {
    if (m_type == Type::NORMAL) {
        const RotationAxis::EdgeType &edge_rotation = std::get<1>(axis);
        for (const auto &order : edge_rotation) {
            std::swap(m_indentations[order[0]], m_indentations[order[2]]);
            std::swap(m_indentations[order[1]], m_indentations[order[3]]);
        }
        // Some indentations need to be mirrored, as the direction has changed.
        // not the last array, as it contains the edges parallel to the axis around which we rotate
        for (int idx = 0; idx < edge_rotation.size() - 1; idx++) {
            m_indentations[edge_rotation[idx][0]].mirror();
            m_indentations[edge_rotation[idx][1]].mirror();
            m_indentations[edge_rotation[idx][2]].mirror();
            m_indentations[edge_rotation[idx][3]].mirror();
        }
        return;
    }
    if (m_type == Type::OCTANT) {
        const RotationAxis::ChildType &child_rotation = std::get<0>(axis);
        for (const auto &order : child_rotation) {
            std::swap(m_childs[order[0]], m_childs[order[2]]);
            std::swap(m_childs[order[1]], m_childs[order[3]]);
        }
        for (auto &child : m_childs) {
            child->rotate<2>(axis);
        }
    }
}

/// 270 degree rotation.
template <>
void Cube::rotate<3>(const RotationAxis::Type &axis) {
    if (m_type == Type::NORMAL) {
        const RotationAxis::EdgeType &edge_rotation = std::get<1>(axis);
        for (const auto &order : edge_rotation) {
            std::swap(m_indentations[order[0]], m_indentations[order[3]]);
            std::swap(m_indentations[order[3]], m_indentations[order[2]]);
            std::swap(m_indentations[order[2]], m_indentations[order[1]]);
        }
        // Some indentations need to be mirrored, as the direction has changed.
        // not the last array, as it contains the edges parallel to the axis around which we rotate
        m_indentations[edge_rotation[0][1]].mirror();
        m_indentations[edge_rotation[0][3]].mirror();
        m_indentations[edge_rotation[1][1]].mirror();
        m_indentations[edge_rotation[1][3]].mirror();
        return;
    }
    if (m_type == Type::OCTANT) {
        const RotationAxis::ChildType &child_rotation = std::get<0>(axis);
        for (const auto &order : child_rotation) {
            std::swap(m_childs[order[0]], m_childs[order[3]]);
            std::swap(m_childs[order[3]], m_childs[order[2]]);
            std::swap(m_childs[order[2]], m_childs[order[1]]);
        }
        for (auto &child : m_childs) {
            child->rotate<3>(axis);
        }
    }
}

Cube::Cube(const float size, const glm::vec3 &position) : m_size(size), m_position(position) {}

Cube::Cube(std::weak_ptr<Cube> parent, const float size, const glm::vec3 &position) : Cube(size, position) {
    m_parent = std::move(parent);
}

Cube::Cube(Cube &&rhs) noexcept : Cube() {
    swap(*this, rhs);
}

Cube &Cube::operator=(Cube rhs) {
    swap(*this, rhs);
    return *this;
}

std::shared_ptr<Cube> Cube::operator[](std::size_t idx) {
    assert(idx <= SUB_CUBES);
    return m_childs[idx];
}

const std::shared_ptr<const Cube> Cube::operator[](std::size_t idx) const {
    assert(idx <= SUB_CUBES);
    return m_childs[idx];
}

std::shared_ptr<Cube> Cube::clone() const {
    std::shared_ptr<Cube> clone = std::make_shared<Cube>(this->m_size, this->m_position);
    clone->m_type = this->m_type;

    if (clone->m_type == Type::NORMAL) {
        clone->m_indentations = this->m_indentations;
    } else if (clone->m_type == Type::OCTANT) {
        for (std::size_t idx = 0; idx <= this->m_childs.size(); idx++) {
            clone->m_childs[idx] = this->m_childs[idx]->clone();
            clone->m_childs[idx]->m_parent = clone;
        }
    }
    clone->m_polygon_cache_valid = this->m_polygon_cache_valid;
    if (clone->m_type == Type::NORMAL || clone->m_type == Type::SOLID) {
        clone->m_polygon_cache = std::make_shared<std::vector<Polygon>>(*this->m_polygon_cache);
    }
    return clone;
}

bool Cube::is_root() const noexcept {
    return m_parent.lock() == nullptr;
}

std::size_t Cube::grid_level() const noexcept {
    std::size_t level = 0;
    std::shared_ptr<Cube> parent = m_parent.lock();
    while (!parent->is_root()) {
        parent = parent->m_parent.lock();
        level++;
    }
    return level;
}

std::size_t Cube::count_geometry_cubes() const noexcept {
    if (m_type == Type::SOLID || m_type == Type::NORMAL) {
        return 1;
    }
    if (m_type == Type::OCTANT) {
        std::size_t count = 0;
        for (const auto &cube : m_childs) {
            count += cube->count_geometry_cubes();
        }
        return count;
    }
    return 0;
}

void Cube::set_type(const Type new_type) {
    if (m_type == new_type) {
        return;
    }
    switch (new_type) {
    case Type::EMPTY:
    case Type::SOLID:
        break;
    case Type::NORMAL:
        m_indentations = {};
        break;
    case Type::OCTANT:
        const float half_size = m_size / 2;
        auto create_cube = [&](const glm::vec3 &offset) {
            return std::make_shared<Cube>(weak_from_this(), half_size, m_position + offset);
        };
        // about the order look into the octree documentation
        m_childs = {create_cube({0, 0, 0}),
                    create_cube({0, 0, half_size}),
                    create_cube({0, half_size, 0}),
                    create_cube({0, half_size, half_size}),
                    create_cube({half_size, 0, 0}),
                    create_cube({half_size, 0, half_size}),
                    create_cube({half_size, half_size, 0}),
                    create_cube({half_size, half_size, half_size})};
        break;
    }
    if (m_type == Type::OCTANT && new_type != Type::OCTANT) {
        remove_childs();
    }
    m_polygon_cache_valid = false;
    m_type = new_type;
    // TODO: clean up if whole octant is empty, etc.
}

Cube::Type Cube::type() const noexcept {
    return m_type;
}

const std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> &Cube::childs() const {
    return m_childs;
}

std::array<Indentation, Cube::EDGES> Cube::indentations() const noexcept {
    return m_indentations;
}

void Cube::set_indent(const std::uint8_t edge_id, Indentation indentation) {
    if (m_type != Type::NORMAL) {
        return;
    }
    assert(edge_id <= Cube::EDGES);
    m_indentations[edge_id] = indentation;
}

void Cube::indent(const std::uint8_t edge_id, const bool positive_direction, const std::uint8_t steps) {
    if (m_type != Type::NORMAL) {
        return;
    }
    assert(edge_id <= Cube::EDGES);
    if (positive_direction) {
        m_indentations[edge_id].indent_start(steps);
    } else {
        m_indentations[edge_id].indent_end(steps);
    }
    m_polygon_cache_valid = false;
}

void Cube::rotate(const RotationAxis::Type &axis, int rotations) {
    rotations = ((rotations % 4) + 4) % 4;
    if (rotations == 0 || m_type == Type::EMPTY || m_type == Type::SOLID) {
        return;
    }
    switch (rotations) {
    case 1:
        rotate<1>(axis);
        break;
    case 2:
        rotate<2>(axis);
        break;
    case 3:
        rotate<3>(axis);
        break;
    default:
        break;
    }
}

void Cube::update_polygon_cache() const {
    if (m_type == Type::OCTANT || m_type == Type::EMPTY) {
        m_polygon_cache = nullptr;
        m_polygon_cache_valid = true;
        return;
    }
    const std::array<glm::vec3, 8> v = vertices();
    m_polygon_cache = std::make_shared<std::vector<Polygon>>(std::vector<Polygon>{
        {{v[0], v[2], v[1]}}, // x = 0
        {{v[1], v[2], v[3]}}, // x = 0
        {{v[4], v[5], v[6]}}, // x = 1
        {{v[5], v[7], v[6]}}, // x = 1
        {{v[0], v[1], v[4]}}, // y = 0
        {{v[1], v[5], v[4]}}, // y = 0
        {{v[2], v[6], v[3]}}, // y = 1
        {{v[3], v[6], v[7]}}, // y = 1
        {{v[0], v[4], v[2]}}, // z = 0
        {{v[2], v[4], v[6]}}, // z = 0
        {{v[1], v[3], v[5]}}, // z = 1
        {{v[3], v[7], v[5]}}  // z = 1
    });
    if (m_type == Type::SOLID) {
        m_polygon_cache_valid = true;
        return;
    }
    if (m_type == Type::NORMAL) {
        const std::array<Indentation, Cube::EDGES> ind = m_indentations;

        // Check for each side if the side is convex, rotate the hypotenuse (middle diagonal edge) so it becomes convex!
        // x = 0
        if (ind[0].start() + ind[6].start() < ind[9].start() + ind[3].start()) {
            (*m_polygon_cache)[0] = {{v[0], v[2], v[3]}};
            (*m_polygon_cache)[1] = {{v[0], v[3], v[1]}};
        }
        // x = 1
        if (ind[0].end() + ind[6].end() < ind[9].end() + ind[3].end()) {
            (*m_polygon_cache)[2] = {{v[4], v[7], v[6]}};
            (*m_polygon_cache)[3] = {{v[4], v[5], v[7]}};
        }
        // y = 0
        if (ind[1].start() + ind[7].start() < ind[4].start() + ind[10].start()) {
            (*m_polygon_cache)[4] = {{v[0], v[1], v[5]}};
            (*m_polygon_cache)[5] = {{v[0], v[5], v[4]}};
        }
        // y = 1
        if (ind[1].end() + ind[7].end() < ind[4].end() + ind[10].end()) {
            (*m_polygon_cache)[6] = {{v[2], v[7], v[3]}};
            (*m_polygon_cache)[7] = {{v[2], v[6], v[7]}};
        }
        // z = 0
        if (ind[2].start() + ind[8].start() < ind[11].start() + ind[5].start()) {
            (*m_polygon_cache)[8] = {{v[0], v[4], v[6]}};
            (*m_polygon_cache)[9] = {{v[0], v[6], v[2]}};
        }
        // z = 1
        if (ind[2].end() + ind[8].end() < ind[11].end() + ind[5].end()) {
            (*m_polygon_cache)[10] = {{v[1], v[3], v[7]}};
            (*m_polygon_cache)[11] = {{v[1], v[7], v[5]}};
        }
        m_polygon_cache_valid = true;
        return;
    }
    // This point should not be reached.
    assert(false);
}
void Cube::invalidate_polygon_cache() const {
    m_polygon_cache_valid = false;
}
std::vector<PolygonCache> Cube::polygons(const bool update_invalid) const {
    std::vector<PolygonCache> polygons;
    polygons.reserve(count_geometry_cubes());

    std::function<void(std::shared_ptr<const world::Cube>)> collect;
    // post-order traversal
    collect = [&collect, &polygons, &update_invalid](std::shared_ptr<const world::Cube> cube) {
        if (cube->type() == world::Cube::Type::OCTANT) {
            for (const auto &child : cube->childs()) {
                collect(child);
            }
            return;
        }
        if (!cube->m_polygon_cache_valid && update_invalid) {
            cube->update_polygon_cache();
        }
        if (cube->m_polygon_cache != nullptr) {
            polygons.push_back(cube->m_polygon_cache);
        }
    };
    collect(this->shared_from_this());
    return polygons;
}
} // namespace inexor::vulkan_renderer::world
