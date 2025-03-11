#version 460 core
#define LIGHT_COUNT 3
#define HAS_ALBEDO_MAP    (1 << 0)
#define HAS_NORMAL_MAP    (1 << 1)
#define HAS_METALLIC_MAP  (1 << 2)
#define HAS_ROUGHNESS_MAP (1 << 3)
#define HAS_AO_MAP        (1 << 4)
#define NO_PBR_TEXTURES   (1 << 5)
#define CUSTOM_METALLIC   (1 << 6)
#define CUSTOM_ROUGHNESS  (1 << 7)
#define HAS_HEIGHT_MAP    (1 << 8)

// Inputs from vertex shader
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tan;
layout (location = 5) in vec3 in_bit;
layout (location = 6) in vec2 MotionVectorIn;  

in vec3 fragTessPosition;
in vec3 fragTessNormal;
in vec2 fragTessUV;

// Outputs
out vec4 out_color;
layout (location = 1) out vec2 MotionVector; // Bewegungsvektor
// Uniforms

layout (location = 16) uniform vec3 camera_pos;        // Kamera-Position
layout (location = 17) uniform float texture_contribution;
layout (location = 18) uniform float specular;
layout (location = 19) uniform float specular_shininess;
layout (location = 20) uniform vec3 mat_ambient = vec3(1, 1, 1);
layout (location = 21) uniform vec3 mat_diffuse = vec3(1, 1, 1); 
layout (location = 22) uniform vec3 mat_specular = vec3(1, 1, 1);

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;    // Prefiltered Environment Map
uniform sampler2D brdfLUT;           // BRDF-Lookup-Texture
uniform sampler2D tex_diffuse;
uniform sampler2D depthMap;
uniform samplerCube tex_shadows[LIGHT_COUNT];

uniform float heightScale;
uniform uint materialFlags;
uniform float customMetallicValue;
uniform float customRoughnessValue;

struct Light {
    vec3 pos;    // +0
    vec3 col;    // +1
    float range; // +2
};
layout (location = 23) uniform Light lights[LIGHT_COUNT];

const float PI = 3.14159265359; 

bool noPBR = (materialFlags & NO_PBR_TEXTURES) != 0;
bool hasAlbedoMap = (materialFlags & HAS_ALBEDO_MAP) != 0;
bool hasNormalMap = (materialFlags & HAS_NORMAL_MAP) != 0;
bool hasMetallicMap = (materialFlags & HAS_METALLIC_MAP) != 0;
bool hasRoughnessMap = (materialFlags & HAS_ROUGHNESS_MAP) != 0;
bool hasAoMap = (materialFlags & HAS_AO_MAP) != 0;
bool customMetallic = (materialFlags & CUSTOM_METALLIC) != 0;
bool customRoughness = (materialFlags & CUSTOM_ROUGHNESS) != 0;
bool hasHeightMap = (materialFlags & HAS_HEIGHT_MAP) != 0;

//TEST normal
vec3 getNormalFromMap(vec2 texCoords)
{
    vec3 tangentNormal = texture(normalMap, texCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(in_pos);
    vec3 Q2  = dFdy(in_pos);
    vec2 st1 = dFdx(texCoords);
    vec2 st2 = dFdy(texCoords);

    vec3 N   = normalize(in_norm);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// Cook-Torrance BRDF components
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calculateNormal(vec3 norm, vec2 uv) {
    // Berechne TBN-Matrix
    vec3 T = normalize(in_tan);
    vec3 B = normalize(in_bit);
    vec3 N = normalize(norm);
    mat3 TBN = mat3(T, B, N);

    vec3 tangentNormal = texture(normalMap, uv).rgb * 2.0 - 1.0;

    return normalize(TBN * tangentNormal);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 blinnPhongShading(vec4 textureColor) {
    vec3 norm = normalize(in_norm);

    // ambient color (low scattered indirect light)
    float ambient_strength = 0.1;
    vec3 ambient_col = ambient_strength * mat_ambient;
    vec3 diffuse_col = vec3(0, 0, 0);
    vec3 specular_col = vec3(0, 0, 0);

    // calculate diffuse and specular colors with each light
    for (int light_i = 0; light_i < LIGHT_COUNT; light_i++) {
        Light light = lights[light_i];
        vec3 light_dir = normalize(light.pos - in_pos); // unit vector from light to fragment/pixel

        // calculate shadow depth via distance from pixel/fragment to light
        vec3 fragment_to_light = in_pos - light.pos;
        float depth_current = length(fragment_to_light);
        // sample cube texture via vector, not UV coordinates
        //float depth_real = texture(tex_shadow, fragment_to_light).r;
        // depth_real = depth_real * light.range;
        // // use a bias to remedy sampling inaccuracies
        float bias_max = 1.0;
        float bias_min = 0.005;
        float bias = max((1.0 - dot(norm, light_dir) * bias_max), bias_min); 
        // // pixel is in shadow, if light did not see it
        float light_contribution = 1.0;
        //if (depth_current - bias > depth_real) light_contribution = 0.0;
/*
        // percentage-closer-filter
        float samples = 4;
        float offset = 0.01;
        float shadow = 0.0;
        for(float x = -offset; x < offset; x += offset / (samples * 0.5)) {
            for(float y = -offset; y < offset; y += offset / (samples * 0.5)) {
                for(float z = -offset; z < offset; z += offset / (samples * 0.5)) {
                    float depth_real = texture(tex_shadows[light_i], fragment_to_light + vec3(x, y, z)).r; 
                    depth_real *= light.range;
                    if(depth_current - bias > depth_real) shadow += 1.0;
                }
            }
        }
        // normalize shadow from 0 to 1
        shadow = shadow / (samples * samples * samples);
        // invert shadow to get light contribution
        float light_contribution = 1.0 - shadow;*/

        // calculate attenuation (point light)
        // see https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation for constant linear/quad factors
        float light_distance = length(light.pos - in_pos);
        float linear = light_distance * 0.14;
        float quadratic = light_distance * light_distance * 0.07;
        float attenuation = 1.0 / (1.0 + linear + quadratic);

        // direct light color
        float diffuse_strength = dot(norm, light_dir);
        diffuse_strength = max(diffuse_strength, 0.0); // filter out "negative" strength
        diffuse_col += light.col * diffuse_strength * attenuation * mat_diffuse * light_contribution;

        // specular color (reflected directly to camera)
        vec3 camera_dir = normalize(in_pos - camera_pos); // unit vector from camera to fragment/pixel
        vec3 reflected_dir = reflect(light_dir, norm);
        float specular_strength = dot(camera_dir, reflected_dir);
        specular_strength = max(specular_strength, 0.0); // filter out "negative" strength
        specular_strength = pow(specular_strength, specular_shininess);
        specular_strength = specular_strength * min(specular, 1.0);
        specular_col += light.col * specular_strength * attenuation * mat_specular * light_contribution;
    }

    // final color
    out_color = mix(in_col, textureColor, texture_contribution);
    return out_color.rgb * (ambient_col + diffuse_col + specular_col); // combine light-affected colors
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 

    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * heightScale;
    vec2 deltaTexCoords = P / numLayers; 

    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
    
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords; 
}

float calculateShadow(vec3 frag_pos, vec3 normal, int light_i) {
    Light light = lights[light_i];
    vec3 fragment_to_light = frag_pos - light.pos;
    
    // Berechnung der Tiefe
    float depth_current = length(fragment_to_light);
    float bias_max = 1.0;
    float bias_min = 0.005;
    float bias = max((1.0 - dot(normal, normalize(light.pos - frag_pos)) * bias_max), bias_min);

    // Percentage-Closer Filtering (PCF)
    float samples = 4;
    float offset = 0.01;
    float shadow = 0.0;
    
    for(float x = -offset; x < offset; x += offset / (samples * 0.5)) {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5)) {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5)) {
                float depth_real = texture(tex_shadows[light_i], fragment_to_light + vec3(x, y, z)).r;
                depth_real *= light.range;
                if (depth_current - bias > depth_real) {
                    shadow += 1.0;
                }
            }
        }
    }

    // Normalisierung des Schattenwertes
    shadow = shadow / (samples * samples * samples);
    return 1.0 - shadow;  // Schattenfaktor (0 = vollständig im Schatten, 1 = beleuchtet)
}


void main() {

    vec2 texCoords = in_uv;

    if(hasHeightMap) {

        mat3 TBN = transpose(mat3(in_tan, in_bit, in_norm));
        texCoords = ParallaxMapping(in_uv, normalize(TBN * (camera_pos - in_pos)));

    } 

    if ((!hasAlbedoMap && !hasAoMap && !hasMetallicMap && !hasNormalMap && !hasRoughnessMap)) {// Für Objekte wie Lichter/Lichtquellen ohne Textur
        vec4 textureColor = texture(tex_diffuse, texCoords).rgba;
        vec3 color = blinnPhongShading(textureColor);
        out_color.rgb = color;
        return;
    } 
    if(noPBR) {//Anpassung für Objekte ohne PBR-Texturierung
        vec4 textureColor = vec4(1.0, 1.0, 1.0, 1.0);
        vec3 norm = in_norm;
        if(hasAlbedoMap) {
            textureColor = texture(albedoMap, texCoords).rgba;//Gamma Korrektur?
        }
        if(hasNormalMap) {
            vec3 norm = calculateNormal(in_norm, texCoords);
        }

        vec3 color = blinnPhongShading(textureColor);
        out_color.rgb = color; 
        return;
    }


    //For Models with no tangent and bitangent getNormal Funktion needed
    //vec3 norm = calculateNormal(in_norm, texCoords);
    vec3 norm = getNormalFromMap(texCoords);
    
    //vec3 albedo = pow(texture(albedoMap, texCoords).rgb, vec3(2.2));
    vec3 albedo = texture(albedoMap, texCoords).rgb;
    //albedo = albedo * 1.5; // Skalierung des Albedo-Werts
    float metallic = texture(metallicMap, texCoords).r;
    //metallic = 1 - metallic;
    float roughness = texture(roughnessMap, texCoords).r;
    //roughness = 1 - roughness; //* roughness; // adjust roughness to be more accurate
    float ao = 1.0;
    if(hasAoMap) {
        ao = texture(aoMap, texCoords).r;
    }
    //IMGui Sliders für Roughness und Metallic
    if(customRoughness) { 
        roughness = customRoughnessValue;
    }
    if(customMetallic) {
        metallic = customMetallicValue;
    }

    //Default werte zum Testen
/*     float metallic = 0.0;
    float roughness = 1.0;
    float ao = 1.0; 
    vec3 norm = vec3(0.0, 0.0, 0.0); */

    vec3 N = normalize(norm);
    vec3 V = normalize(camera_pos - in_pos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 R = reflect(-V, N);

    vec3 Lo = vec3(0.0);
    float shadow_factor = 1.0;
    for (int i = 0; i < LIGHT_COUNT; ++i) {
        Light light = lights[i];
        vec3 L = normalize(light.pos - in_pos);
        vec3 H = normalize(V + L);

        float light_distance = length(light.pos - in_pos);
        float linear = light_distance * 0.14;
        float quadratic = light_distance * light_distance * 0.07;
        float attenuation = 1.0 / (1.0 + linear + quadratic);
        vec3 radiance = light.col * attenuation;

        // Complicated shadow calculation
        //shadow_factor *= calculateShadow(in_pos, N, i);
        
        //Simple Shadow Calculation
/*         vec3 fragToLight = in_pos - lights[i].pos;
        float closestDepth = texture(tex_shadows[i], fragToLight).r; // Depth-Wert aus Shadow-Map
        float currentDepth = length(fragToLight) / lights[i].range;
        
        if (currentDepth > closestDepth + 0.005) {
            shadow_factor *= 0.5; // Schatten abdunkeln
        }  */
         

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL ;//* shadow_factor;
    }

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS; // Fresnel Schlick with roughness
    kD *= 1.0 - metallic; // Only non-metallic surfaces diffuse

    // Indirect diffuse lighting (IBL diffuse irradiance)
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;

    // Indirect specular lighting (IBL specular prefiltered environment map)
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;  
    vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    // Combine results
    vec3 ambient = ((kD * diffuseIBL + specularIBL) * shadow_factor) * ao;

    vec3 color = ambient + Lo;

    // HDR Tone Mapping und Gamma-Korrektur
    color = color / (color + vec3(1.0));
    //color = pow(color, vec3(1.0 / 2.2));

    out_color = vec4(color, 1.0);

    MotionVector = MotionVectorIn; 

/*     vec3 fragToLight = in_pos - lights[0].pos;
    float closestDepth = texture(tex_shadows[0], fragToLight).r;

    out_color = vec4(vec3(closestDepth), 1.0); 
    */
    
/*     shadow_factor = 1.0; 
    

    for (int i = 0; i < LIGHT_COUNT; i++) {
        vec3 fragToLight = in_pos - lights[i].pos;
        float closestDepth = texture(tex_shadows[i], fragToLight).r; // Depth-Wert aus Shadow-Map
        float currentDepth = length(fragToLight) / lights[i].range;
        
        if (currentDepth > closestDepth + 0.005) {
            shadow_factor *= 0.5; // Schatten abdunkeln
        }
    } 
*/
    //out_color = vec4(vec3(shadow_factor), 1.0);  

}
