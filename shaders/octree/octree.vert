#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (binding = 0) uniform UniformBufferObject {
	mat4 projection;
	mat4 model;
	mat4 view;
} ubo;

layout (location = 0) out vec3 frag_color;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
    frag_color = in_color;
}
