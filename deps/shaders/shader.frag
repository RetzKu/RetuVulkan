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
	float ambientSTR = 0.4;
	float specularSTR = 0.5;
	vec3 col = texture(texSampler,fragTexCoord).rgb;
	vec3 norm = normalize(Normal);
	
	vec3 result = vec3(0,0,0);

	outColor = vec4(0,0,0,0);

	float lConst = 1.0;
	float lLinear = 0.04;
	float lQuadratic = 0.0003;

	for(int i = 0; i < lightNum ; i++)
	{
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

//		int x = int(pointlight[0].pos.x);
//		int y = int(pointlight[0].pos.y);
//		int z = int(pointlight[0].pos.z);
//

//		if(pointlight[0].pos.y != 0 || pointlight[0].pos.z != 0 || pointlight[0].pos.x != 0)
//		if (x != 0 ) {realOut += vec4(1,0,0,1); }
//		if (y != 0 ) {realOut += vec4(0,1,0,1); }
//		if (z != 0 ) {realOut += vec4(0,0,1,1); }
////
//		if(y != 0 || z != 0 || x != 0)
//		{
//			vec3 specular = specularSTR * spec * vec3(1,0,0);
//			vec3 diffuse = ambientSTR * diff * vec3(1,0,0) * ambientSTR;
//			//outColor =  vec4(1,1,1,1); 
//			//return ;
//		}

//		result += (ambient + diff)* col;

//		result += (ambient + diff)* col * vec3(pointlight[i].color);
		result += (diffuse + specular + ambient)* col;
//		result += ((pointlight[i].color*ambientSTR) + diffuse)* col;
	}
//	result = col * ambientSTR;
//	outColor = vec4(result,1.0) * 0.01 + realOut;
	outColor = vec4(result,1);
}