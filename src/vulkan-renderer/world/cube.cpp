#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <random>

void swap(inexor::vulkan_renderer::world::Cube &lhs, inexor::vulkan_renderer::world::Cube &rhs) noexcept {
    std::swap(lhs.m_type, rhs.m_type);
    std::swap(lhs.m_size, rhs.m_size);
    std::swap(lhs.m_position, rhs.m_position);
    std::swap(lhs.m_parent, rhs.m_parent);
    std::swap(lhs.m_index_in_parent, rhs.m_index_in_parent);
    std::swap(lhs.m_indentations, rhs.m_indentations);
    std::swap(lhs.m_children, rhs.m_children);
    std::swap(lhs.m_polygon_cache, rhs.m_polygon_cache);
    std::swap(lhs.m_polygon_cache_valid, rhs.m_polygon_cache_valid);
}

namespace inexor::vulkan_renderer::world {
void Cube::remove_children() {
    for (auto &child : m_children) {
        child->remove_children();
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
            std::swap(m_children[order[0]], m_children[order[1]]);
            std::swap(m_children[order[1]], m_children[order[2]]);
            std::swap(m_children[order[2]], m_children[order[3]]);
        }
        for (auto &child : m_children) {
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
            std::swap(m_children[order[0]], m_children[order[2]]);
            std::swap(m_children[order[1]], m_children[order[3]]);
        }
        for (auto &child : m_children) {
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
            std::swap(m_children[order[0]], m_children[order[3]]);
            std::swap(m_children[order[3]], m_children[order[2]]);
            std::swap(m_children[order[2]], m_children[order[1]]);
        }
        for (auto &child : m_children) {
            child->rotate<3>(axis);
        }
    }
}

Cube::Cube(const float size, const glm::vec3 &position) : m_size(size), m_position(position) {}

Cube::Cube(std::weak_ptr<Cube> parent, const std::uint8_t index, const float size, const glm::vec3 &position)
    : Cube(size, position) {
    m_parent = std::move(parent);
    m_index_in_parent = index;
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
    return m_children[idx];
}

std::shared_ptr<const Cube> Cube::operator[](std::size_t idx) const {
    assert(idx <= SUB_CUBES);
    return m_children[idx];
}

std::shared_ptr<Cube> Cube::clone() const {
    std::shared_ptr<Cube> clone = std::make_shared<Cube>(this->m_size, this->m_position);
    clone->m_type = this->m_type;
    clone->m_index_in_parent = this->m_index_in_parent;

    if (clone->m_type == Type::NORMAL) {
        clone->m_indentations = this->m_indentations;
    } else if (clone->m_type == Type::OCTANT) {
        for (std::size_t idx = 0; idx <= this->m_children.size(); idx++) {
            clone->m_children[idx] = this->m_children[idx]->clone();
            clone->m_children[idx]->m_parent = clone;
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
        for (const auto &cube : m_children) {
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
        std::uint8_t index = 0;
        auto create_cube = [&](const glm::vec3 &offset) {
            return std::make_shared<Cube>(weak_from_this(), index++, half_size, m_position + offset);
        };
        // Look into octree documentation to find information about the order of subcubes in space.
        // We can't use initializer list here because clang-tidy complains about it.
        // To the best of our knowledge, this is a false positive.
        m_children[0] = create_cube({0, 0, 0});
        m_children[1] = create_cube({0, 0, half_size});
        m_children[2] = create_cube({0, half_size, 0});
        m_children[3] = create_cube({0, half_size, half_size});
        m_children[4] = create_cube({half_size, 0, 0});
        m_children[5] = create_cube({half_size, 0, half_size});
        m_children[6] = create_cube({half_size, half_size, 0});
        m_children[7] = create_cube({half_size, half_size, half_size});
        break;
    }
    if (m_type == Type::OCTANT && new_type != Type::OCTANT) {
        remove_children();
    }
    m_polygon_cache_valid = false;
    m_type = new_type;
    // TODO: clean up if whole octant is empty, etc.
}

Cube::Type Cube::type() const noexcept {
    return m_type;
}

const std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> &Cube::children() const {
    return m_children;
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

    // post-order traversal
    std::function<void(const Cube &)> collect = [&collect, &polygons, &update_invalid](const Cube &cube) {
        if (cube.type() == world::Cube::Type::OCTANT) {
            for (const auto &child : cube.children()) {
                collect(*child);
            }
            return;
        }
        if (!cube.m_polygon_cache_valid && update_invalid) {
            cube.update_polygon_cache();
        }
        if (cube.m_polygon_cache != nullptr) {
            polygons.push_back(cube.m_polygon_cache);
        }
    };
    collect(*this);
    return polygons;
}

std::shared_ptr<Cube> Cube::neighbor(const NeighborAxis axis, const NeighborDirection direction) {
    if (is_root()) {
        return nullptr;
    }

    // Each axis only requires information and manipulation of one (relevant) bit to find the neighbor.
    const auto relevant_index_bit = static_cast<std::uint8_t>(axis); // bit index of the axis we are working on

    auto get_bit = [&relevant_index_bit](const std::uint8_t cube_index) {
        return ((cube_index >> relevant_index_bit) & 1u) != 0;
    };
    auto toggle_bit = [&relevant_index_bit](const std::uint8_t cube_index) {
        return cube_index ^ (1u << relevant_index_bit);
    };

    auto parent = m_parent.lock();
    const std::uint8_t index = m_index_in_parent;
    const bool home_bit = get_bit(index);

    // The relevant bit denotes whether `m_parent` and `this` share a face on the upper side of the relevant axis.
    // If they share one and also the user wants to go in the positive direction, then the neighbor is not a sibling.
    // (Same for opposite, i.e. share face on lower side and going negative direction.)
    if (home_bit && direction == NeighborDirection::NEGATIVE || !home_bit && direction == NeighborDirection::POSITIVE) {
        // The demanded neighbor is a sibling! Return the neighboring sibling.
        return parent->m_children[toggle_bit(index)];
    }
    if (parent->is_root()) {
        return nullptr;
    }
    // the neighbor is further away than a sibling

    // Keep the history of indices because we just need to mirror indices (i.e. toggle the relevant bit)
    // to find the desired neighboring cube.
    std::vector<std::uint8_t> history;
    history.push_back(index);

    // Find the first cube where the bit (of `get_bit`) is different to `this_bit`.
    // That cubes parent is the first mutual parent of the desired neighbor and `this`.
    std::uint8_t p_index = parent->m_index_in_parent;
    history.push_back(p_index);
    while (get_bit(p_index) == home_bit) {
        parent = parent->m_parent.lock();
        if (parent->is_root()) {
            return nullptr;
        }
        p_index = parent->m_index_in_parent;
        history.push_back(p_index);
    }

    // get the first mutual parent of neighbor and `this`.
    std::shared_ptr<Cube> child = parent->m_parent.lock();

    // Now mirror the path we took by just flipping the relevant bit of each index in the history.
    while (!history.empty()) {
        if (child->m_type != Type::OCTANT) {
            // The neighbor is larger but still a neighbor!
            return std::shared_ptr<Cube>(child);
        }
        child = child->m_children[toggle_bit(history.back())];
        history.pop_back();
    }

    // We found a same-sized neighbor!
    return std::shared_ptr<Cube>(child);
}

std::shared_ptr<Cube> create_random_world(std::uint32_t max_depth, const glm::vec3 &position,
                                          std::optional<std::uint32_t> seed) {
    std::random_device rd;
    std::mt19937 mt(seed ? *seed : rd());
    std::uniform_int_distribution<std::uint8_t> indent(0, 44);
    std::uniform_int_distribution<std::uint32_t> cube_type(0, 100);

    std::shared_ptr<Cube> cube = std::make_shared<Cube>(4.0f, position);
    cube->set_type(Cube::Type::OCTANT);
    std::function<void(const Cube &, std::uint32_t)> populate_cube = [&](const Cube &parent, std::uint32_t depth) {
        for (const auto &child : parent.children()) {
            if (depth != max_depth) {
                child->set_type(Cube::Type::OCTANT);
                populate_cube(*child, depth + 1);
                continue;
            }
            auto ty = cube_type(mt);
            if (ty < 30) {
                child->set_type(Cube::Type::EMPTY);
                continue;
            }
            if (ty < 60) {
                child->set_type(Cube::Type::SOLID);
                continue;
            }
            if (ty < 100) {
                child->set_type(Cube::Type::NORMAL);
                for (int i = 0; i < 12; i++) {
                    child->set_indent(i, Indentation(indent(mt)));
                }
                continue;
            }
        }
    };
    populate_cube(*cube, 0);
    return cube;
}

} // namespace inexor::vulkan_renderer::world
