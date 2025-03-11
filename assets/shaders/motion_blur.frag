#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform sampler2D motionVectorTexture;
uniform sampler2D bloomBlur;
uniform float motionBlurStrength;
uniform float motionBlurSamples;
uniform float exposure;
uniform float gamma;

void main() {
    vec3 color = vec3(0.0);
    float samples = motionBlurSamples;
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;

    // Lese Motion-Vektor
    vec2 motionVector = texture(motionVectorTexture, TexCoords).rg;
    
    // Additiver Bloom (Helligkeit hinzufügen)
    hdrColor += bloomColor;

    // Falls der Motion Vector fast null ist, rendere das Bild normal
    if (length(motionVector) < 0.002) {
        motionVector = vec2(0.0);
    }

    // Begrenze Motion Vectors weiter
    motionVector = clamp(motionVector, -motionBlurStrength, motionBlurStrength);

    vec3 blurredColor = vec3(0.0);
    // Bessere Gewichtung: Zentrum stärker, Ränder schwächer
    for (float i = -samples / 2; i <= samples / 2; i++) {
        vec2 sampleUV = TexCoords - (motionVector * (i / samples));
        blurredColor += texture(screenTexture, sampleUV).rgb;
    }
    hdrColor = mix(hdrColor, blurredColor / float(samples), 0.5);

    // Tone Mapping & Gamma Korrektur
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}
