#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 0) out vec4 outLightDir;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out mat4 outTNBMatrix;
layout(location = 6) out float outVertexLightValue;


void main() {
    vec3 lightDir = {-2.f, -3.f, 1.f};
    lightDir = normalize(lightDir);

    float vertexLightValue = dot(-lightDir, inNormal);
    outVertexLightValue = vertexLightValue;

    vec3 bitangent = normalize(cross(inTangent, inNormal));

    mat4 tnbMatrix = {
        vec4(inTangent, 0.f),
        vec4(inNormal, 0.f),
        vec4(bitangent, 0.f),
        vec4(0.f, 0.f, 0.f, 0.f)
    };

    outTNBMatrix = transpose(tnbMatrix);

    // Move the light direction into the local space of the 

    vec4 inPositionModified = vec4(inPosition, 1.0);
    inPositionModified = ubo.proj * ubo.view * inPositionModified;
    gl_Position = inPositionModified;
    outLightDir = vec4(lightDir, 0.f);
    outTexCoord = inTexCoord;
}