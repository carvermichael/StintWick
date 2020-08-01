#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec4 fragColor;

out vec4 fragColorOut;

uniform vec3 objectAmbient;
uniform vec3 objectDiffuse;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;

void main()
{
   fragColorOut = vec4(objectDiffuse, 1.0f);
};
