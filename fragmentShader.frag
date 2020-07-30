#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec4 fragColor;
in vec2 texCoords;
in vec3 normal;

out vec4 fragColorOut;

void main()
{
   fragColorOut = fragColor;
};
