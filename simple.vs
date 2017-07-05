#version 150
attribute vec3 a_position;
attribute vec3 a_color;
attribute vec2 a_texcoord;
attribute vec3 a_normal;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

varying vec3 normal;
varying vec3 color;
varying vec2 texCoord;

void main()
{
	color = a_color;
	texCoord = a_texcoord;

	mat3 normalMatrix = mat3(modelViewMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	normal = normalMatrix * a_normal;
	
	gl_Position = projectionMatrix * modelViewMatrix * vec4(a_position,1);
}