#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct PointLight {
	vec4 pos;                    
	float radius;
	vec4 intensity;	
	vec4 color;      
};


layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(std430, set = 1, binding = 0) buffer pointlights
{
	vec4 lightNum;
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
	
	for(int i = 0; i < lightNum.x; i++)
	{
		vec3 lightDir = normalize(vec3(pointlight[i].pos) - FragPos);
		float diff = max(dot(norm,lightDir),0.0);

//		vec3 diffuse = diff * pointlight[i].color;
//		vec3 diffuse = diff * lightCol;

//		result += (ambient + diff)* col * vec3(pointlight[i].color.x, 0, 0.0);
//		result += (ambient + diff)* col * vec3(pointlight[i].color);
//		result += (ambient + diff)* col * vec3(0.37,0.37,0.93);
//		result += ((pointlight[i].color*ambientSTR) + diffuse)* col;
	}
	result = col * ambientSTR;
	outColor = vec4(result,1.0);
}