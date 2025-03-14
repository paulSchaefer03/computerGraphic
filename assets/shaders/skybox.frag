#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

vec3 ACESFilmToneMapping(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdrColor = texture(skybox, TexCoords).rgb;
    // ACES-Filmic-Tonemapping anwenden
    vec3 mappedColor = ACESFilmToneMapping(hdrColor);
    // Gamma-Korrektur done in post-processing
    //mappedColor = pow(mappedColor, vec3(1.0 / 2.2));
    FragColor = vec4(mappedColor, 1.0);
}
