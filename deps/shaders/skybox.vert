#version 450
#extension GL_ARB_shading_language_420pack : enable


layout(binding = 0) uniform UBO
{
	mat4 view;
	mat4 projection;
	mat4 model;
}ubo;

layout (location = 0) in vec3 inPosition;
layout(location = 0) out vec3 FragPos;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = ubo.projection * ubo.view * vec4(inPosition, 1.0);
	FragPos = inPosition;
}