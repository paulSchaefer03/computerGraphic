#version 460 core

// input
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tangent;
layout (location = 5) in vec3 in_bitangent;

// output
layout (location = 0) out vec3 out_pos;
layout (location = 1) out vec3 out_norm;
layout (location = 2) out vec4 out_color;
layout (location = 3) out vec2 out_uv;
layout (location = 4) out vec3 out_tangent;
layout (location = 5) out vec3 out_bitangent;

// uniforms
// mat4x4 takes up for locations, each location being 16 bytes
layout (location = 0) uniform mat4x4 model_transform;
layout (location = 4) uniform mat4x4 normal_transform;
layout (location = 8) uniform mat4x4 camera_transform;
layout (location = 12) uniform mat4x4 camera_perspective;

void main() {
    vec4 worldPosition = model_transform * vec4(in_pos, 1.0);
    //gl_Position = camera_perspective * camera_transform * worldPosition;
    out_pos = worldPosition.xyz;
    //out_pos = in_pos;
    gl_Position = vec4(in_pos, 1.0);
    
    // only rotate the normal
    //out_norm = normalize(mat3x3(normal_transform) * in_norm);
    out_norm = normalize(mat3x3(model_transform) * in_norm);
    vec3 transformedTangent = normalize(mat3(normal_transform) * in_tangent);
    out_tangent = normalize(transformedTangent - dot(transformedTangent, out_norm) * out_norm);
    out_bitangent = cross(out_norm, out_tangent);

    out_color = in_col;
    out_uv = in_uv;
}