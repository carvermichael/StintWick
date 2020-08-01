#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec3 normal;
in vec3 fragmentPos;

out vec4 fragColorOut;

uniform vec3 objectAmbient;
uniform vec3 objectDiffuse;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;

void main()
{

	vec3 lightDir = normalize(lightPos - fragmentPos);

	// ambient lighting
	vec3 ambientResult = 0.5f * objectAmbient;

	// diffuse lighting
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuseResult = lightDiffuse * diff * objectDiffuse;

	// putting 'em all together
	vec3 result = (ambientResult + diffuseResult);
	fragColorOut = vec4(result, 1.0f);
};
