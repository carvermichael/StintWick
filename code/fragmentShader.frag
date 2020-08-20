#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec3 normal;
in vec3 fragmentPos;

out vec4 fragColorOut;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform vec3 viewPos;

void main()
{
	vec3 lightDir = normalize(lightPos - fragmentPos);

	// ambient lighting
	vec3 ambientResult = 0.5f * materialAmbient;

	// diffuse lighting
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuseResult = lightDiffuse * diff * materialDiffuse;
	
	// specular lighting
	vec3 viewDir = normalize(viewPos - fragmentPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), materialShininess); 
	vec3 specularResult = lightSpecular * spec * materialSpecular;

	specularResult *= 0.2f; // attenuation stand-in

	vec3 result = (ambientResult + diffuseResult + specularResult);
	fragColorOut = vec4(result, 1.0f);
};
