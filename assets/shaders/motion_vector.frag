#version 460 core
layout (location = 0) out vec2 MotionVector; // Speichert die Bewegungsdaten

in vec2 TexCoords;
in vec2 MotionVectorIn;
layout (location = 16) uniform vec3 camera_pos;        // Kamera-Position
layout (location = 17) uniform float texture_contribution;
layout (location = 18) uniform float specular;
layout (location = 19) uniform float specular_shininess;

void main() {
    MotionVector = MotionVectorIn; // Speichert die Bewegung für Motion Blur
}
