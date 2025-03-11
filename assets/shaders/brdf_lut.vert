#version 460 core

// input
layout (location = 0) in vec3 in_pos;
layout (location = 2) in vec2 in_uvA;
// output
out vec2 in_uv;

void main()
{
    in_uv = in_uvA;
    gl_Position = vec4(in_pos , 1.0);
}