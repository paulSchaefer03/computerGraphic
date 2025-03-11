#version 460 core

// interpolated input from vertex shader
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;

// final pixel/fragment color
layout (location = 0) out vec4 out_color;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() { // blinn-phong shading
    vec2 uv = SampleSphericalMap(normalize(in_pos)); // Normalisiert lokale Position
    vec3 color = texture(equirectangularMap, uv).rgb;
    out_color = vec4(color, 1.0);
}