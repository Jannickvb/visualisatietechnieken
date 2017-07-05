attribute vec3 a_position;
attribute vec3 a_normal;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform float time;


void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position + vec3(sin(time + a_position.y), 0.25 * sin(a_position.y*1.24) * cos(time*1.432 + a_position.x), 0),1);
}