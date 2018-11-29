#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 model;
};
layout(set = 2, binding = 1) uniform CameraMatrix
{
	mat4 view;
	mat4 proj;
};

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 aNormal; 

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 FragPos;

void main()
{
	gl_Position = proj * view * model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
	FragPos = vec3(model * vec4(inPosition,1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
}