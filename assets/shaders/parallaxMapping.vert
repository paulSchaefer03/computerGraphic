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
layout (location = 2) out vec4 out_col;
layout (location = 3) out vec2 out_uv;
layout (location = 4) out vec3 out_tangent;
layout (location = 5) out vec3 out_bitangent;
layout (location = 6) out vec3 out_tangent_viewPos; 
// uniforms
// mat4x4 takes up for locations, each location being 16 bytes
layout (location = 0) uniform mat4x4 model_transform;
layout (location = 4) uniform mat4x4 normal_transform;
layout (location = 8) uniform mat4x4 camera_transform;
layout (location = 12) uniform mat4x4 camera_perspective;

layout (location = 16) uniform vec3 camera_pos;

void main()
{
    gl_Position = model_transform * vec4(in_pos, 1.0);
    out_pos = gl_Position.xyz;
    gl_Position = camera_transform * gl_Position;
    gl_Position = camera_perspective * gl_Position;
    out_norm = normalize(mat3x3(normal_transform) * in_norm);
    vec3 transformedTangent = normalize(mat3(normal_transform) * in_tangent);
    out_tangent = normalize(transformedTangent - dot(transformedTangent, out_norm) * out_norm);
    out_bitangent = cross(out_norm, out_tangent);

    out_norm = normalize(mat3(normal_transform) * in_norm); 

    out_uv = in_uv;   
    out_col = in_col;

    vec3 T   = normalize(mat3(model_transform) * in_tangent);
    vec3 B   = normalize(mat3(model_transform) * in_bitangent);
    vec3 N   = normalize(mat3(model_transform) * in_norm);
    mat3 TBN = transpose(mat3(T, B, N));

    out_tangent_viewPos = TBN * camera_pos;
    out_pos  = TBN * out_pos;
}   