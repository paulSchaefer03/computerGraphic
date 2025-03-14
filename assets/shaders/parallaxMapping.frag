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

// Inputs from vertex shader
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tan;
layout (location = 5) in vec3 in_bit;
layout (location = 6) in vec3 in_tangent_viewPos;

// Outputs
out vec4 out_color;

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
  
uniform float height_scale;
uniform uint materialFlags;
uniform float customMetallicValue;
uniform float customRoughnessValue;

uniform samplerCube tex_shadows[LIGHT_COUNT];
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


 

void main()
{           
    // offset texture coordinates with Parallax Mapping
    vec3 V = normalize(camera_pos - in_pos);
    vec2 texCoords = ParallaxMapping(in_uv,  V);

}