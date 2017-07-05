uniform sampler2D s_texture;
varying vec2 texCoord;
uniform float time;


void main()
{
	gl_FragColor = texture2D(s_texture, texCoord + vec2(time,0));
}