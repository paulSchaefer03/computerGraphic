#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (location = 40) uniform mat4x4 view;
layout (location = 44) uniform mat4x4 projection;

void main()
{
    TexCoords = aPos; // Übergibt die Würfel-Koordinaten
    mat4 rotView = mat4(mat3(view)); // Entfernt die Translation
    vec4 clipPos = projection * rotView * vec4(aPos, 1.0);
    gl_Position = clipPos.xyww; // Projektion auf die Bildschirmfläche
}
