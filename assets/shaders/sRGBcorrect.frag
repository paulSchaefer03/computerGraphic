#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float aberrationStrength;

void main()
{
    //Chrome-Aberration-Effekt
    float shift = aberrationStrength * length(TexCoords - 0.5);
    vec3 color;
    color.r = texture(screenTexture, TexCoords + vec2( shift, 0.0)).r;
    color.g = texture(screenTexture, TexCoords).g;
    color.b = texture(screenTexture, TexCoords - vec2( shift, 0.0)).b;
    
    // sRGB-Korrektur (Gamma 2.2)
    color = pow(color, vec3(1.0/2.2)); 
    FragColor = vec4(color, 1.0);
    
}
