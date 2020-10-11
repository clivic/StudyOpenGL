#version 330 core
out vec4 fragColor;
in vec2 texCoord;

uniform sampler2D texImg0;
uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
	fragColor = vec4(lightColor * objectColor, 1.0) * texture(texImg0, texCoord);
}