#include <inexor/vulkan-renderer/world/cube.hpp>

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
        this->x_level = x.value();
    }
    if (y != nullopt) {
        this->y_level = y.value();
    }
    if (z != nullopt) {
        this->z_level = z.value();
    }
    this->change();
}

void Indentation::set_x(uint8_t x) {
    this->x_level = x;
    this->change();
}

void Indentation::set_y(uint8_t y) {
    this->y_level = y;
    this->change();
}

void Indentation::set_z(uint8_t z) {
    this->z_level = z;
    this->change();
}

uint8_t Indentation::x() const { return this->x_level; }
uint8_t Indentation::y() const { return this->y_level; }
uint8_t Indentation::z() const { return this->z_level; }

Indentation Indentation::parse(BitStream &stream) {
    // Parse each indentation level one by one.
    return Indentation(Indentation::parse_one(stream), Indentation::parse_one(stream), Indentation::parse_one(stream)
    );
}

Indentation::Indentation() = default;

Indentation::Indentation(uint8_t x, uint8_t y, uint8_t z) {
    this->x_level = x;
    this->y_level = y;
    this->z_level = z;
}

uint8_t Indentation::parse_one(BitStream &stream) {
    bool indented = stream.get(1).value();
    if (indented) {
        return static_cast<uint8_t>(stream.get(3).value());
    }
    return 0;
}

glm::tvec3<uint8_t> Indentation::vec() const {
    return { this->x_level, this->y_level, this->z_level};
}

void Indentation::change() {
    this->on_change(this);
}

Cube::Cube(CubeType type, float size, const glm::vec3 &position) {
    this->cube_type = type;
    this->cube_size = size;
    this->cube_position = position;
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
    return this->cube_type;
}

vector<array<glm::vec3, 3>> Cube::polygons() {
    vector<array<glm::vec3, 3>> polygons;

    uint64_t i = 0;

    polygons.resize(this->leaves() * 12);

    auto polygons_pointer = polygons.data();
    this->all_polygons(polygons_pointer);
    return polygons;
}

void Cube::all_polygons(array<glm::vec3, 3>* &polygons) {
    if (this->cube_type == CubeType::EMPTY) {
        return;
    }
    if (this->cube_type == CubeType::OCTANTS) {
        for (const auto &octant: this->octants.value()) {
            octant->all_polygons(polygons);
        }
        return;
    }

    // _type == (FULL or INDENTED)
    if (!this->valid_cache) {
        if (this->cube_type == CubeType::FULL) {
            this->polygons_cache = this->full_polygons();
        } else {
            this->polygons_cache = this->indented_polygons();
        }
        this->valid_cache = true;
    }

    // TODO: Let polygons return array of pointers to cache instead of copying the value.
    for (const auto polygon: this->polygons_cache) {
        *polygons = polygon;
        polygons++;
    }
}

uint64_t Cube::leaves() {
    switch(this->cube_type) {
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
    assert(false); // This point should never be reached, as we handled all types already.
    return 0;
}

array<array<glm::vec3, 3>, 12> Cube::full_polygons(array<glm::vec3, 8> &v) {
    return {{
        {{ v[0], v[2], v[1] }}, // x = 0
        {{ v[1], v[2], v[3] }}, // x = 0

        {{ v[4], v[5], v[6] }}, // x = 1
        {{ v[5], v[7], v[6] }}, // x = 1

        {{ v[0], v[1], v[4] }}, // y = 0
        {{ v[1], v[5], v[4] }}, // y = 0

        {{ v[2], v[6], v[3] }}, // y = 1
        {{ v[3], v[6], v[7] }}, // y = 1

        {{ v[0], v[4], v[2] }}, // z = 0
        {{ v[2], v[4], v[6] }}, // z = 0

        {{ v[1], v[3], v[5] }}, // z = 1
        {{ v[3], v[7], v[5] }}, // z = 1
    }};
}

array<array<glm::vec3, 3>, 12> Cube::full_polygons() {
    assert(this->cube_type == CubeType::FULL);

    array<glm::vec3, 8> v = this->vertices();
    return this->full_polygons(v);
}

array<array<glm::vec3, 3>, 12> Cube::indented_polygons() {
    assert(this->cube_type == CubeType::INDENTED);

    array<glm::vec3, 8> v = this->vertices();

    array<array<glm::vec3, 3>, 12> vertices = this->full_polygons(v);
    array<glm::tvec3<uint8_t>, 8> in = this->indentation_levels();

    // Check for each side if the side is convex, rotate the hypotenuse so it becomes convex!
    // x = 0
    if (in[0].x + in[3].x < in[1].x + in[2].x) {
        vertices[0] = {{ v[0], v[2], v[3] }};
        vertices[1] = {{ v[0], v[3], v[1] }};
    }

    // x = 1
    if (in[4].x + in[7].x < in[5].x + in[6].x) {
        vertices[2] = {{ v[4], v[7], v[6] }};
        vertices[3] = {{ v[4], v[5], v[7] }};
    }

    // y = 0
    if (in[0].y + in[5].y < in[1].y + in[4].y) {
        vertices[4] = {{ v[0], v[1], v[5] }};
        vertices[5] = {{ v[0], v[5], v[4] }};
    }

    // y = 1
    if (in[2].y + in[7].y < in[3].y + in[6].y) {
        vertices[6] = {{ v[2], v[7], v[3] }};
        vertices[7] = {{ v[2], v[6], v[7] }};
    }

    // z = 0
    if (in[0].z + in[6].z < in[2].z + in[4].z) {
        vertices[8] = {{ v[0], v[4], v[6] }};
        vertices[9] = {{ v[0], v[6], v[2] }};
    }

    // z = 1
    if (in[1].z + in[7].z < in[3].z + in[6].z) {
        vertices[10] = {{ v[1], v[3], v[7] }};
        vertices[11] = {{ v[1], v[7], v[5] }};
    }
    return vertices;
}

array<glm::vec3, 8> Cube::vertices() {
    assert(this->cube_type == CubeType::FULL || this->cube_type == CubeType::INDENTED);
    glm::vec3 &p = this->cube_position;

    // Most distant corner of a "full" cube (from p)
    glm::vec3 f = {p.x + this->cube_size, p.y + this->cube_size, p.z + this->cube_size};

    if (this->cube_type == CubeType::FULL) {
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
    assert(this->cube_type == CubeType::INDENTED);
    const float step = this->cube_size / MAX_INDENTATION;

    array<glm::tvec3<uint8_t>, 8> in = this->indentation_levels();

    // Calculate the vertex-positions with respect to the indentation level.
    return array<glm::vec3, 8>{{
        {p.x + step * in[0].x, p.y + step * in[0].y, p.z + step * in[0].z},
        {p.x + step * in[1].x, p.y + step * in[1].y, f.z - step * in[1].z},
        {p.x + step * in[2].x, f.y - step * in[2].y, p.z + step * in[2].z},
        {p.x + step * in[3].x, f.y - step * in[3].y, f.z - step * in[3].z},
        {f.x - step * in[4].x, p.y + step * in[4].y, p.z + step * in[4].z},
        {f.x - step * in[5].x, p.y + step * in[5].y, f.z - step * in[5].z},
        {f.x - step * in[6].x, f.y - step * in[6].y, p.z + step * in[6].z},
        {f.x - step * in[7].x, f.y - step * in[7].y, f.z - step * in[7].z}
    }};
}

void Cube::invalidate_cache() {
    this->valid_cache = false;
}

void Cube::change() {
    this->invalidate_cache();
    this->on_change(this);
}

void Cube::change(Indentation *indentation) {
    this->change();
}
array<glm::tvec3<uint8_t>, 8> Cube::indentation_levels() {
    array<glm::tvec3<uint8_t>, 8> in;
    auto &indents = this->indentations.value();
    for (size_t i = 0; i < in.size(); i++) {
        in[i] = indents[i].vec();
    }
    return in;
}
