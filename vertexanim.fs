uniform sampler2D s_texture;
varying vec2 texCoord;
varying vec3 color;
uniform float time;


void main()
{
	gl_FragColor = mix(texture2D(s_texture, texCoord), texture2D(s_texture, texCoord + vec2(0.5, 0.5)), 0.5 + 0.5 * sin(time));
}