#version 460 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec4 in_col;
layout (location = 3) in vec2 in_uv;
layout (location = 4) in vec3 in_tangent;
layout (location = 5) in vec3 in_bitangent;

// Outputs für den Fragment-Shader
out vec2 TexCoords;
out vec2 MotionVectorIn;

// Uniforms
layout (location = 0) uniform mat4x4 model_transform;
layout (location = 4) uniform mat4x4 normal_transform;
layout (location = 8) uniform mat4 camera_transform;
layout (location = 12) uniform mat4 camera_perspective;
uniform mat4 prevViewProjectionMatrix;

void main() {
    gl_Position = model_transform * vec4(in_pos, 1.0);
    gl_Position = camera_transform * gl_Position;
    gl_Position = camera_perspective * gl_Position; 
    TexCoords = in_uv;
    
    // Transformiere Vertex-Position mit aktueller View-Projection-Matrix
    vec4 currentPos = camera_perspective * camera_transform * vec4(in_pos, 1.0);
    
    // Transformiere die gleiche Position mit der vorherigen View-Projection-Matrix
    vec4 previousPos = prevViewProjectionMatrix * vec4(in_pos, 1.0);
    
    // Berechne den Motion-Vector (Bewegung in Screenspace)
    MotionVectorIn = (currentPos.xy / currentPos.w) - (previousPos.xy / previousPos.w);
    if (prevViewProjectionMatrix == (camera_perspective * camera_transform)) {
        MotionVectorIn = vec2(0.0);
    }
}
