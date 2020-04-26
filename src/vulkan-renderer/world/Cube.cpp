//
// Created by moritz on 21.04.20.
//

#include "Cube.h"
//
// Created by moritz on 16.04.20.
//

#include "Cube.h"

using namespace inexor::world;

/**
 * How often a cube can be indented, results in MAX_INDENTATION+1 steps.
 */
const static uint8_t MAX_INDENTATION = 8;
/**
 * The default size of a cube / the octree size boundaries.
 */
const static float DEFAULT_CUBE_SIZE = 1;
/**
 * The default position of the cube in the coordinate system.
 */
const static glm::vec3 DEFAULT_CUBE_POSITION = {0., 0., 0.};

void Indentation::set(optional<uint8_t> x, optional<uint8_t> y, optional<uint8_t> z) {
    if (x != nullopt) {
        this->_x = x.value();
    }
    if (y != nullopt) {
        this->_y = y.value();
    }
    if (z != nullopt) {
        this->_z = z.value();
    }
    this->_change();
}

void Indentation::set_x(uint8_t x) {
    this->_x = x;
    this->_change();
}

void Indentation::set_y(uint8_t y) {
    this->_y = y;
    this->_change();
}

void Indentation::set_z(uint8_t z) {
    this->_z = z;
    this->_change();
}

uint8_t Indentation::x() const { return this->_x; }
uint8_t Indentation::y() const { return this->_y; }
uint8_t Indentation::z() const { return this->_z; }

Indentation Indentation::parse(BitStream &stream) {
    // Parse each indentation level one by one.
    return Indentation(
        Indentation::_parse_one(stream),
        Indentation::_parse_one(stream),
        Indentation::_parse_one(stream)
    );
}

Indentation::Indentation() = default;

Indentation::Indentation(uint8_t x, uint8_t y, uint8_t z) {
    this->_x = x;
    this->_y = y;
    this->_z = z;
}

uint8_t Indentation::_parse_one(BitStream &stream) {
    bool indented = stream.get(1).value();
    if (indented) {
        return static_cast<uint8_t>(stream.get(3).value());
    }
    return 0;
}

glm::tvec3<uint8_t> Indentation::vec() const {
    return { this->_x, this->_y, this->_z };
}

void Indentation::_change() {
    this->on_change(this);
}

Cube::Cube(CubeType type, float size, const glm::vec3 &position) {
    this->_type = type;
    this->_size = size;
    this->_position = position;
}

Cube::Cube(array<Indentation, 8> &indentations, float size, const glm::vec3 &position):
    Cube(CubeType::INDENTED, size, position) {
    // Parse indentations to optional<...>
    this->indentations = {static_cast<array<Indentation, 8> &&>(indentations)};
    for (auto &indentation: this->indentations.value()) {
        // indentation.on_change.connect([this] { this->_change(); });
    }
}

Cube::Cube(array<shared_ptr<Cube>, 8> &octants, float size, const glm::vec3 &position):
    Cube(CubeType::OCTANTS, size, position) {
    // Parse octants to optional<...>
    this->octants = {static_cast<array<shared_ptr<Cube>, 8> &&>(octants)};
}


Cube Cube::parse(vector<unsigned char> &data) {
    BitStream stream = BitStream(data.data(), data.size());
    return Cube::parse(stream);
}

Cube Cube::parse(BitStream &stream) {
    return Cube::parse(stream, DEFAULT_CUBE_SIZE, DEFAULT_CUBE_POSITION);
}

Cube Cube::parse(BitStream &stream, float size, const glm::vec3 &position) {
    CubeType type = static_cast<CubeType>(stream.get(2).value());
    if(type == CubeType::EMPTY || type == CubeType::FULL) {
        return Cube(type, size, position);
    }
    if(type == CubeType::INDENTED) {
        // Parse indentations
        array<Indentation, 8> indentations;
        for (uint8_t i = 0; i < 8; i++) {
            indentations[i] = Indentation::parse(stream);
        }
        return Cube(indentations, size, position);
    }

    assert(type == CubeType::OCTANTS);
    // Parse the octants.
    const float half = size/2;
    const float x = position.x;
    const float y = position.y;
    const float z = position.z;
    const float xh = x+half;
    const float yh = y+half;
    const float zh = z+half;
    array<shared_ptr<Cube>, 8> octants = {
        make_shared<Cube>(Cube::parse(stream, half, { x , y , z  })),
        make_shared<Cube>(Cube::parse(stream, half, { x , y , zh })),
        make_shared<Cube>(Cube::parse(stream, half, { x , yh, z  })),
        make_shared<Cube>(Cube::parse(stream, half, { x , yh, zh })),
        make_shared<Cube>(Cube::parse(stream, half, { xh, y , z  })),
        make_shared<Cube>(Cube::parse(stream, half, { xh, y , zh })),
        make_shared<Cube>(Cube::parse(stream, half, { xh, yh, z  })),
        make_shared<Cube>(Cube::parse(stream, half, { xh, yh, zh }))
    };
    return Cube(octants, size, position);
}

CubeType Cube::type() {
    return this->_type;
}

vector<array<glm::vec3, 3>> Cube::polygons() {
    vector<array<glm::vec3, 3>> polygons;

    uint64_t i = 0;

    polygons.resize(this->leaves() * 12);

    auto polygons_pointer = polygons.data();
    this->_all_polygons(polygons_pointer);
    return polygons;
}

void Cube::_all_polygons(array<glm::vec3, 3>* &polygons) {
    if (this->_type == CubeType::EMPTY) {
        return;
    }
    if (this->_type == CubeType::OCTANTS) {
        for (const auto &octant: this->octants.value()) {
            octant->_all_polygons(polygons);
        }
        return;
    }

    // _type == (FULL or INDENTED)
    if (!this->_valid_cache) {
        if (this->_type == CubeType::FULL) {
            this->_polygons_cache = this->_full_polygons();
        } else {
            this->_polygons_cache = this->_indented_polygons();
        }
        this->_valid_cache = true;
    }

    // TODO: Let polygons return array of pointers to cache instead of copying the value.
    for (const auto polygon: this->_polygons_cache) {
        *polygons = polygon;
        polygons++;
    }
}

uint64_t Cube::leaves() {
    switch(this->_type) {
        case CubeType::EMPTY:
            return 0;
        case CubeType::FULL:
        case CubeType::INDENTED:
            return 1;
        case CubeType::OCTANTS:
            uint64_t i = 0;
            for (const auto& octant: this->octants.value()) {
                i += octant->leaves();
            }
            return i;
    }
    assert(false);
    return 0;
}

array<array<glm::vec3, 3>, 12> Cube::_full_polygons() {
    array<glm::vec3, 8> v = this->_vertices();
    return {{
        {{ v[0], v[1], v[2] }}, // x = 0
        {{ v[1], v[2], v[3] }}, // x = 0

        {{ v[4], v[5], v[6] }}, // x = 1
        {{ v[5], v[6], v[7] }}, // x = 1

        {{ v[0], v[1], v[4] }}, // y = 0
        {{ v[1], v[4], v[5] }}, // y = 0

        {{ v[2], v[3], v[6] }}, // y = 1
        {{ v[3], v[6], v[7] }}, // y = 1

        {{ v[0], v[2], v[4] }}, // z = 0
        {{ v[2], v[6], v[4] }}, // z = 0

        {{ v[1], v[3], v[5] }}, // z = 1
        {{ v[3], v[7], v[5] }}, // z = 1
    }};
}

array<array<glm::vec3, 3>, 12> Cube::_indented_polygons() {
    assert(this->_type == CubeType::INDENTED);

    array<glm::vec3, 8> v = this->_vertices();
    array<array<glm::vec3, 3>, 12> vertices = {{
        {{ v[0], v[2], v[3] }}, // x = 0
        {{ v[0], v[1], v[3] }}, // x = 0

        {{ v[4], v[6], v[7] }}, // x = 1
        {{ v[4], v[5], v[7] }}, // x = 1

        {{ v[0], v[1], v[5] }}, // y = 0
        {{ v[1], v[5], v[4] }}, // y = 0

        {{ v[2], v[3], v[7] }}, // y = 1
        {{ v[2], v[7], v[6] }}, // y = 1

        {{ v[0], v[6], v[4] }}, // z = 0
        {{ v[0], v[2], v[6] }}, // z = 0

        {{ v[1], v[3], v[7] }}, // z = 1
        {{ v[1], v[7], v[5] }}  // z = 1
    }};
    auto v0 = this->indentations.value()[0].vec();
    auto v1 = this->indentations.value()[1].vec();
    auto v2 = this->indentations.value()[2].vec();
    auto v3 = this->indentations.value()[3].vec();
    auto v4 = this->indentations.value()[4].vec();
    auto v5 = this->indentations.value()[5].vec();
    auto v6 = this->indentations.value()[6].vec();
    auto v7 = this->indentations.value()[7].vec();

    // Check for each side if the side is convex, rotate the hypotenuse so it becomes convex!
    // x = 0
    if (v0.x + v3.x >= v1.x + v2.x) {
        vertices[0] = {{ v[0], v[1], v[2] }};
        vertices[1] = {{ v[1], v[2], v[3] }};
    }

    // x = 1
    if (v4.x + v7.x >= v5.x + v6.x) {
        vertices[2] = {{ v[4], v[5], v[6] }};
        vertices[3] = {{ v[5], v[6], v[7] }};
    }

    // y = 0
    if (v0.y + v5.y >= v1.y + v4.y) {
        vertices[4] = {{ v[0], v[1], v[4] }};
        vertices[5] = {{ v[0], v[1], v[4] }};
    }

    // y = 1
    if (v2.y + v7.y >= v3.y + v6.y) {
        vertices[6] = {{ v[2], v[3], v[6] }};
        vertices[7] = {{ v[3], v[6], v[7] }};
    }

    // z = 0
    if (v0.z + v6.z >= v2.z + v4.z) {
        vertices[8] = {{ v[0], v[2], v[4] }};
        vertices[9] = {{ v[2], v[6], v[4] }};
    }

    // z = 1
    if (v1.z + v7.z >= v3.z + v6.z) {
        vertices[10] = {{ v[1], v[3], v[5] }};
        vertices[11] = {{ v[3], v[7], v[5] }};
    }
    return vertices;
}

array<glm::vec3, 8> Cube::_vertices() {
    assert(this->_type == CubeType::FULL || this->_type == CubeType::INDENTED);
    glm::vec3 &p = this->_position;

    // Most distant corner of a "full" cube (from p)
    glm::vec3 f = {p.x + this->_size, p.y + this->_size, p.z + this->_size };

    if (this->_type == CubeType::FULL) {
        return array<glm::vec3, 8>{{
            {p.x, p.y, p.z},
            {p.x, p.y, f.z},
            {p.x, f.y, p.z},
            {p.x, f.y, f.z},
            {f.x, p.y, p.z},
            {f.x, p.y, f.z},
            {f.x, f.y, p.z},
            {f.x, f.y, f.z}
        }};
    }
    assert(this->_type == CubeType::INDENTED);
    const float step = this->_size / MAX_INDENTATION;
    // TODO: Implement Indentation copy-constructor without the signal or as shared_ptr to the signal?
    auto &i = this->indentations.value();
    const glm::tvec3<float> i0 = i[0].vec();
    const glm::tvec3<float> i1 = i[1].vec();
    const glm::tvec3<float> i2 = i[2].vec();
    const glm::tvec3<float> i3 = i[3].vec();
    const glm::tvec3<float> i4 = i[4].vec();
    const glm::tvec3<float> i5 = i[5].vec();
    const glm::tvec3<float> i6 = i[6].vec();
    const glm::tvec3<float> i7 = i[7].vec();

    // Calculate the vertex-positions with respect to the indentation level.
    return array<glm::vec3, 8>{{
        {p.x + step * i0.x, p.y + step * i0.y, p.z + step * i0.z},
        {p.x + step * i1.x, p.y + step * i1.y, f.z - step * i1.z},
        {p.x + step * i2.x, f.y - step * i2.y, p.z + step * i2.z},
        {p.x + step * i3.x, f.y - step * i3.y, f.z - step * i3.z},
        {f.x - step * i4.x, p.y + step * i4.y, p.z + step * i4.z},
        {f.x - step * i5.x, p.y + step * i5.y, f.z - step * i5.z},
        {f.x - step * i6.x, f.y - step * i6.y, p.z + step * i6.z},
        {f.x - step * i7.x, f.y - step * i7.y, f.z - step * i7.z}
    }};
}

void Cube::invalidate_cache() {
    this->_valid_cache = false;
}

void Cube::_change() {
    this->invalidate_cache();
    this->on_change(this);
}

void Cube::_change(Indentation *indentation) {
    this->_change();
}
