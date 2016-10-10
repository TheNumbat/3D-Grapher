
#version 330 core

in vec2 TexCoord;
//in vec3 vcolor;
out vec4 color;

//uniform vec4 pcolor;
uniform sampler2D tex1;
//uniform sampler2D tex2;

void main()
{	
	color = texture(tex1, TexCoord);
	//color = mix(texture(tex1, TexCoord), texture(tex2, TexCoord), 0.2f) * vec4(vcolor, 0.5f);
}
