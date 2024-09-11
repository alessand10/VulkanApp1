#version 450

layout(binding = 1, rgba8) uniform image2D albedo; 
layout(binding = 2, rgba8) uniform image2D normal; 

layout(binding = 1) uniform sampler2D albedoSampler;
layout(binding = 2) uniform sampler2D normalSampler;


layout(location = 0) in vec3 lightDir;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 outNormal = texture(normalSampler, texCoord).xyz;
    outColor = vec4(dot(outNormal, -lightDir) * texture(albedoSampler, texCoord).xyz, 1.f);
}