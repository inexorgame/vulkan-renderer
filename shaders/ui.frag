#version 450

layout (location = 0) in vec2 tex_coords;
layout (binding = 1) uniform sampler2D font_texture;
layout (location = 0) out vec4 out_color;

void main() {
    out_color = texture(font_texture, tex_coords);
}
