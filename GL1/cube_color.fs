#version 330 core
out vec4 fragColor;
in vec2 texCoord;

uniform sampler2D texImg0;
uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
	fragColor = vec4(objectColor, 1.0);
	// fragColor = vec4( objectColor, 1.0) * texture(texImg0, texCoord);
    // fragColor = vec4(0.7);

}