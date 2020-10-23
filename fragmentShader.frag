#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec3 normal;
in vec3 fragmentPos;

out vec4 fragColorOut;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

struct Light {
	bool current;
	
	vec3 pos;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define NUM_LIGHTS 50
uniform Light lights[NUM_LIGHTS];

uniform vec3 viewPos;

uniform vec3 playerPos;

void main()
{
	vec3 fogColor = vec3(0.5, 0.5, 0.5);

	float distFromPlayer = length(playerPos - fragmentPos);
	
	float fogMax = 15.0;
	float fogMin = 0.0;
	float fogFactor = (distFromPlayer - fogMax) / 
						(fogMax - fogMin);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	
	// ambient lighting	
	vec3 ambientResult = 1.0f * materialAmbient;

	vec3 diffuseResult = vec3(0.0f);
	vec3 specularResult = vec3(0.0f);

	for(int i = 0; i < NUM_LIGHTS; i++) {
		if(!lights[i].current) continue;
	
		vec3 lightDir = normalize(lights[i].pos - fragmentPos);

		// diffuse lighting
		vec3 norm = normalize(normal);
		float diff = max(dot(norm, lightDir), 0.0);
		diffuseResult += lights[i].diffuse * diff * materialDiffuse;

		// specular lighting
		vec3 viewDir = normalize(viewPos - fragmentPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0f), materialShininess); 
		vec3 specularResult = lights[i].specular * spec * materialSpecular;
		//vec3 specularResult = vec3(0.5f, 0.5f, 0.5f);		
	}

	// TODO: re-add attenuation for shrapnel lighting
	specularResult *= 0.2f; // attenuation stand-in

	vec3 result = (ambientResult + diffuseResult + specularResult);

	// uncomment next line for fog
	// result = mix(result, fogColor, fogFactor);

	fragColorOut = vec4(result, 1.0f);
};
