#version 450

layout(binding = 1) uniform sampler2DArray albedoSampler;
layout(binding = 2) uniform sampler2DArray normalSampler;

layout(push_constant) uniform PushConstants {
    int textureIndex;
} pc;


layout(location = 0) in vec4 lightDir;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in mat4 tnbMatrix;
layout(location = 6) in float outVertexLightValue;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 sampledNormal = vec4(texture(normalSampler, vec3(texCoord, pc.textureIndex)).xyz, 0.f);
    vec4 objectSpaceNormal = tnbMatrix * sampledNormal;
    float vertexNormalInfluence = 0.2f;
    float lightStrength = dot(-lightDir, objectSpaceNormal) * (1.f - vertexNormalInfluence) + (outVertexLightValue * vertexNormalInfluence);
    outColor = vec4(lightStrength * texture(albedoSampler, vec3(texCoord, pc.textureIndex)).xyz, 1.f);
}