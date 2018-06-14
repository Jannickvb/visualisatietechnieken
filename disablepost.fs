#version 330
uniform sampler2D s_texture;

in vec2 texCoord;

void main()
{
	vec4 color = texture2D(s_texture, texCoord);
	gl_FragColor = color;
}