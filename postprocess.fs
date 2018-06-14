#version 330
uniform sampler2D s_texture;
uniform float time;

in vec2 texCoord;

void main()
{
    float o = time/2048.0;

	vec4 color = texture2D(s_texture, texCoord + 0.01 * vec2(sin(time + texCoord.y * 20), sin(time*0.5 + texCoord.x * 24)));

	gl_FragColor = color;
}