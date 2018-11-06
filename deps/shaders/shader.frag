#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

struct PointLight {
	vec3 pos;
	float radius;
	vec3 intensity;
};

layout(std140, set = 1, binding = 0) uniform UniformBufferObject
{
	int lightNum;
	PointLight pointlight[10000];
};


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 2)in vec3 Normal;
layout(location = 3)in vec3 FragPos;

layout(location = 0) out vec4 outColor;

void main()
{
	float ambientSTR = 0.1;
	vec3 lightCol = vec3(1,1,1);
	vec3 ambient = lightCol*ambientSTR;
	vec3 col = texture(texSampler,fragTexCoord).rgb;
	
	
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(vec3(0.1,0,0) - FragPos);
	float diff = max(dot(norm,lightDir),0.0);
	vec3 diffuse = diff * lightCol;
	vec3 result = (ambient + diffuse)* col;
	
	outColor = vec4(result,1.0);
	
	//outColor = texture(texSampler,fragTexCoord);
}