#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct PointLight {
	vec4 pos;                    
	vec4 radius;
	vec4 intensity;	
	vec4 color; //16     
};

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(std140, set = 1, binding = 0) buffer pointlights
{
	int lightNum;
	PointLight pointlight[200];
};

layout(std140, set = 2, binding = 0) buffer cameraPos
{
	vec3 cameraPosition;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 FragPos;

layout(location = 0) out vec4 outColor;

void main()
{
	float ambientSTR = 1;
	float specularSTR = 0.5;


	vec3 col = texture(texSampler[0],fragTexCoord).rgb;
	//vec4 blendMap = texture(texSampler[1],fragTexCoord);

	//col = col * blendMap.r;

	vec3 norm = normalize(Normal);
	vec3 result = vec3(0,0,0);
	outColor = vec4(0,0,0,0);

	for(int i = 0; i < lightNum ; i++)
	{
		float lConst = 1.0;
		float lLinear = 0.04 * (pointlight[i].intensity[0]*pointlight[i].intensity[0]);
		float lQuadratic = 0.0003 * (pointlight[i].radius[0] * pointlight[i].radius[0]);
		//vec3 lightCol = vec3(pointlight[i].color) * ambientSTR;
		vec3 lightDir = normalize(vec3(pointlight[i].pos) - FragPos);
		vec3 viewDir = normalize(cameraPosition - FragPos);
		vec3 reflectDir = reflect(-lightDir,norm);

		float spec = pow(max(dot(viewDir,reflectDir),0.0),128);
		float diff = max(dot(norm,lightDir),0.0);

		float lightDistance = length(pointlight[i].pos.rgb - FragPos);
		float attentuation = 1.0/ (lConst + lLinear * lightDistance + lQuadratic*(lightDistance*lightDistance));

		vec3 ambient = ambientSTR * pointlight[i].color.rgb * attentuation;
		vec3 diffuse = ambientSTR * diff * pointlight[i].color.rgb * attentuation;
		vec3 specular = specularSTR * spec * pointlight[i].color.rgb * attentuation;

		result += (diffuse + specular + ambient)* col;
		//result *= blendResult;
	}
	outColor = vec4(result,1);
}