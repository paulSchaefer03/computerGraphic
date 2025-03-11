#version 460 core
// input
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
// output
out vec2 TexCoords;
// uniforms
layout (location = 0) uniform mat4x4 model_transform;
layout (location = 4) uniform mat4x4 normal_transform;
layout (location = 8) uniform mat4x4 camera_transform;
layout (location = 12) uniform mat4x4 camera_perspective;

void main()
{
    TexCoords = in_uv;
    gl_Position = vec4(in_pos.x, in_pos.y, 0.0, 1.0);
}
