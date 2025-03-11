#version 460 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
// output
out vec2 TexCoords;
// uniforms

void main() {
    TexCoords = in_uv;
    gl_Position = vec4(in_pos.x, in_pos.y, 0.0, 1.0);
}