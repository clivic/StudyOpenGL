#version 330 core
out vec4 FragColor;
in vec4 VertexColor;
uniform float ColorModF;
void main()
{
	FragColor = VertexColor * ColorModF;
}