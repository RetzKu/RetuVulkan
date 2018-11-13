#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_KHR_vulkan_glsl : enable

struct PointLight {
	vec3 pos;
	float radius;
	vec3 intensity;
	vec3 color;
};

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(std140, set = 1, binding = 0) buffer pointlights
{
	int lightNum;
	PointLight pointlight[200];
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 FragPos;

layout(location = 0) out vec4 outColor;

void main()
{
	float ambientSTR = 0.1;
	vec3 lightCol = vec3(1,1,1);
	vec3 ambient = lightCol*ambientSTR;
	vec3 col = texture(texSampler,fragTexCoord).rgb;
	vec3 norm = normalize(Normal);
	
	vec3 result = vec3(0,0,0);

	outColor = vec4(0,0,0,0);
	
	for(int i = 0; i < lightNum; i++)
	{
		vec3 lightDir = normalize(pointlight[i].pos - FragPos);

		float diff = max(dot(norm,lightDir),0.0);
//		vec3 diffuse = diff * pointlight[i].color;
		vec3 diffuse = diff * lightCol;

		result += (ambient + diffuse)* col;
//		result += ((pointlight[i].color*ambientSTR) + diffuse)* col;
	}
	outColor = vec4(result,1.0);
}