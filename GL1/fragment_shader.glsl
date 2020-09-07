#version 330 core
uniform vec3 TColor;
out vec4 FragColor;

void main()
{
   //FragColor = vec4(0.5f, 0.2f, 0.0f, 1.0f);
   FragColor = vec4(TColor, 1.0f);
}