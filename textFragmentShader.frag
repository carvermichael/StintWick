#version 330 core
in vec2 texCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {
	vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(text, texCoords).r); // setting the alpha here in order to make nonCharacter bits transparent
	color = vec4(textColor, 1.0f) * sampled;
};