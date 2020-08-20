
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 normal;
out vec3 fragmentPos;

//uniform mat4 transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragmentPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * vec4(fragmentPos, 1.0f);
//   fragmentPos = vec3(model * transform * vec4(aPos, 1.0));
   
   normal = aNormal;
};