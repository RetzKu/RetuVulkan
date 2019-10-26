#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform samplerCube cubemap;
//layout(binding = 1) uniform sampler2D cubemap;

layout(location = 0) in vec3 FragPos;

layout(location = 0) out vec4 outFragColor;

void main()
{
	//vec3 col = texture(cubemap,vec2(FragPos.x,FragPos.y));
	vec3 col = texture(cubemap,FragPos).rgb;
	//vec3 col = vec3(1,1,1);
	outFragColor = vec4(col,1);
}