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
    
    // Bloom-Effekt: nur helle Pixel anzeigen
/*     float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)); // Luminanz berechnen
    vec3 bloomColor = (brightness > 1.0) ? color : vec3(0.0);
 */

    // sRGB-Korrektur (Gamma 2.2)
    //bloomColor = pow(bloomColor, vec3(1.0/2.2)); 
    FragColor = vec4(color, 1.0);
    //FragColor = vec4(0.9, 0.0, 0.0, 1.0);
 
}
