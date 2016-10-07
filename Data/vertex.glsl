
#version 330 core

layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 color;
layout (location = 1) in vec2 texCoord;

//out vec3 vcolor;
out vec2 TexCoord;

uniform mat4 model, view, proj;

void main()
{
	gl_Position = proj * view * model * vec4(position, 1.0f);
	TexCoord = texCoord;
}
