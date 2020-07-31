#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTexCoord;
//layout (location = 2) in vec3 aNormal;

out vec3 fragColor;
//out vec2 texCoords;
//out vec3 normal;

uniform vec3 colorIn;

uniform mat4 transform;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
   fragColor = colorIn;
//   texCoords = aTexCoord;
//   normal = aNormal;
};


/*
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

in vec3 normal;
in vec3 fragmentPos;
in vec2 texCoords;

out vec4 fragColorOut;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light {
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	// point-light attenuation constants
	float constant;
	float linear;
	float quadratic;

	// spotlight constants
	float angleCutoff;
};

uniform Material material;
uniform Light light;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	vec3 lightDir = normalize(light.position - fragmentPos);

	// spotlight calc
	float theta = dot(lightDir, normalize(-light.direction));

	if(theta > light.angleCutoff) {
		// ambient lighting
		vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoords));

		// diffuse lighting
		vec3 norm = normalize(normal);		
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoords));

		// specular lighting
		vec3 viewDir = normalize(viewPos - fragmentPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
		vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoords));

		// point-light calculations
		float distance = length(light.position - fragmentPos);
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

//		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		// putting 'em all together
		vec3 result = (ambient + diffuse + specular);
		fragColorOut = vec4(result, 1.0f);
	}
	else {
		// just do ambient lighting
//		fragColorOut = vec4(0.2f);
		fragColorOut = vec4(light.ambient * vec3(texture(material.diffuse, texCoords)), 1.0);
	}
};

*/