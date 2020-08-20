#version 330 core
#extension GL_ARB_separate_shader_objects : enable

out vec4 fragColorOut;

uniform vec4 color; // RBGA

void main()
{
   fragColorOut = vec4(color);
};