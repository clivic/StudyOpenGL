#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTex;

//uniform float offsetF;
uniform mat4 transform;
out vec4 VertexColor;
out vec2 TexCoord;

void main()
{
   //gl_Position = vec4(aPos.x + offsetF, aPos.y, aPos.z, 1.0);
   gl_Position = transform * vec4(aPos, 1.0f);

   // Color
   VertexColor = vec4(aCol, 1.0);

   // Tex Coordinates
   TexCoord = aTex;
}