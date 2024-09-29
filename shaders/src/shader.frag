#version 450

layout(binding = 1) uniform sampler2DArray albedoSampler;
layout(binding = 2) uniform sampler2DArray normalSampler;

layout(push_constant) uniform PushConstants {
    int textureIndex;
} pc;


layout(location = 0) in vec3 lightDir;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 outNormal = texture(normalSampler, vec3(texCoord, pc.textureIndex)).xyz;
    outColor = vec4(dot(outNormal, -lightDir) * texture(albedoSampler, vec3(texCoord, pc.textureIndex)).xyz, 1.f);
}