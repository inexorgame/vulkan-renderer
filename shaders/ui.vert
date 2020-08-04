#version 450

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_tex_coords;

// TODO: Use push constants here
layout (binding = 0) uniform UiUniformBuffer {
    vec2 translation;
    vec2 scale;
} ubo;

layout (location = 0) out vec2 tex_coords;

void main() {
    gl_Position = vec4(in_position * ubo.scale + ubo.translation, 0.0, 1.0);
    tex_coords = in_tex_coords;
}
