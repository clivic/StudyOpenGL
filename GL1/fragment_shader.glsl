#version 330 core
out vec4 FragColor;
in vec4 VertexColor;
in vec2 TexCoord;

uniform float ColorModF;

uniform sampler2D TexImg0;
uniform sampler2D TexImg1;

uniform float visibleAmtF;

void main()
{
	// FragColor = VertexColor * ColorModF;
	FragColor = mix(texture(TexImg0, TexCoord), texture(TexImg1, TexCoord), visibleAmtF) * ColorModF;
	// FragColor = mix(texture(TexImg0, TexCoord), texture(TexImg1, TexCoord.y), 0.25) * VertexColor * ColorModF;
}