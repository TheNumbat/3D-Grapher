
#version 330 core

in vec3 vcolor;
out vec4 color;

//uniform vec4 pcolor;

void main()
{
	color = vec4(vcolor, 1.0f);
}