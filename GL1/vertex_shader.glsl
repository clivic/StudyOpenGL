#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

uniform float offsetF;
out vec4 VertexColor;

void main()
{
   gl_Position = vec4(aPos.x + offsetF, aPos.y, aPos.z, 1.0);

   // Color
   VertexColor = vec4(aCol, 1.0);
}