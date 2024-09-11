#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 0) out vec3 outLightDir;
layout(location = 1) out vec2 outTexCoord;


void main() {
    vec3 lightDir = {-2.f, -3.f, -1.f};
    lightDir = normalize(lightDir);

    vec4 inPositionModified = vec4(inPosition, 1.0);
    inPositionModified = ubo.proj * ubo.view * inPositionModified;
    gl_Position = inPositionModified;
    outLightDir = lightDir;
    outTexCoord = inTexCoord;
}