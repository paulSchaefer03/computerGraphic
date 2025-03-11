#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform sampler2D motionVectorTexture;
uniform float motionBlurStrength;
uniform float motionBlurSamples;

void main() {
    vec3 color = vec3(0.0);
    float totalWeight = 0.0;
    float samples = motionBlurSamples; 

    // Lese Motion-Vektor
    vec2 motionVector = texture(motionVectorTexture, TexCoords).rg;
    
    // Falls der Motion Vector fast null ist, rendere das Bild normal
    if (length(motionVector) < 0.002) {
        FragColor = texture(screenTexture, TexCoords);
        return;
    }

    // Begrenze Motion Vectors weiter
    motionVector = clamp(motionVector, -motionBlurStrength, motionBlurStrength);

    // Bessere Gewichtung: Zentrum stärker, Ränder schwächer
    for (float i = -samples / 2; i <= samples / 2; i++) {
        float weight = 1.0 - abs(i) / (samples / 2.0); // Mittelpunkt stärker gewichten
        vec2 sampleUV = TexCoords - (motionVector * (i / samples));
        color += texture(screenTexture, sampleUV).rgb * weight;
        totalWeight += weight;
    }

    FragColor = vec4(color / totalWeight, 1.0);
}
