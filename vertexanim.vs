#version 330
layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_color;
layout(location=2) in vec2 a_texcoord;
layout(location=3) in vec3 a_normal;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform float time;

out vec3 normal;
out vec3 color;
out vec2 texCoord;

void main()
{
	mat4 modelViewMatrix = viewMatrix * modelMatrix;
	color = a_color;
	texCoord = a_texcoord;

	mat3 normalMatrix = mat3(modelViewMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	normal = normalMatrix * a_normal;
	
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position + vec3(sin(time + a_position.y), 0.25 * sin(a_position.y*1.24) * cos(time*1.432 + a_position.x), 0),1);
}