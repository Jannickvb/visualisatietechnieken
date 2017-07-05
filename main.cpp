#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <map>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma comment(lib, "glew32.lib")


class Shader
{
	std::map<std::string, GLuint> uniforms;
public:
	GLuint programId;
	GLuint getUniform(const std::string &name)
	{
		auto it = uniforms.find(name);
		if (it != uniforms.end())
			return it->second;
		GLuint location = glGetUniformLocation(programId, name.c_str());
		uniforms[name] = location;
		return location;
	}


};


glm::ivec2 screenSize;
float rotation;
float lastTime;

class Vertex
{
public:
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texcoord;
	glm::vec3 normal;
	Vertex(const glm::vec3 &position, const glm::vec3 &color, const glm::vec2 &texcoord, const glm::vec3 &normal) : position(position), color(color), texcoord(texcoord), normal(normal) {}
};


void checkShaderErrors(GLuint shaderId)
{
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);					//kijk of het compileren is gelukt
	if (status == GL_FALSE)
	{
		int length, charsWritten;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);				//haal de lengte van de foutmelding op
		char* infolog = new char[length + 1];
		memset(infolog, 0, length + 1);
		glGetShaderInfoLog(shaderId, length, &charsWritten, infolog);		//en haal de foutmelding zelf op
		std::cout << "Error compiling shader:\n" << infolog << std::endl;
		delete[] infolog;
	}
}


#ifdef WIN32
void GLAPIENTRY onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
#else
void onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
#endif
{
	std::cout << message << std::endl;
}


Shader* loadShader(std::string vs, std::string fs)
{
	std::ifstream vertexShaderFile(vs);
	std::string vertexShaderData((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());
	const char* cvertexShaderData = vertexShaderData.c_str();

	std::ifstream fragShaderFile(fs);
	std::string fragShaderData((std::istreambuf_iterator<char>(fragShaderFile)), std::istreambuf_iterator<char>());
	const char* cfragShaderData = fragShaderData.c_str();


	Shader* ret = new Shader();

	ret->programId = glCreateProgram();							// maak een shaderprogramma aan

	GLuint vertexId = glCreateShader(GL_VERTEX_SHADER);		// maak vertex shader aan
	glShaderSource(vertexId, 1, &cvertexShaderData, NULL);		// laat opengl de shader uit de variabele 'vertexShader' halen
	glCompileShader(vertexId);								// compileer de shader
	checkShaderErrors(vertexId);							// controleer of er fouten zijn opgetreden bij het compileren
	glAttachShader(ret->programId, vertexId);					// hang de shader aan het shaderprogramma


	GLuint fragmentId = glCreateShader(GL_FRAGMENT_SHADER);	// maak fragment shader aan
	glShaderSource(fragmentId, 1, &cfragShaderData, NULL);	// laat opengl de shader uit de variabele 'fragmentShader' halen
	glCompileShader(fragmentId);							// compileer de shader
	checkShaderErrors(fragmentId);							// controleer of er fouten zijn opgetreden bij het compileren
	glAttachShader(ret->programId, fragmentId);					// hang de shader aan het shaderprogramma

	glBindAttribLocation(ret->programId, 0, "a_position");		// zet de positie op vertex attribuut 0
	glBindAttribLocation(ret->programId, 1, "a_color");			// zet de kleur op vertex attribuut 1
	glBindAttribLocation(ret->programId, 2, "a_texcoord");		// zet de texcoord op vertex attribuut 2
	glBindAttribLocation(ret->programId, 3, "a_normal");		// zet de texcoord op vertex attribuut 2
	glLinkProgram(ret->programId);								// link het programma, zorg dat alle attributes en varying gelinked zijn
	glUseProgram(ret->programId);								// Zet dit als actieve programma

	return ret;
}

std::vector<Vertex> vertices;
std::vector<Vertex> cube;
GLuint textureId;
Shader* normalShader;
Shader* postShader;

GLuint fboTextureId;
GLuint fboId;



void init()
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glClearColor(0, 0, 0, 1);

	normalShader = loadShader("simple.vs", "simple.fs");
	postShader = loadShader("postprocess.vs", "postprocess.fs");
	
	glEnableVertexAttribArray(0);							// we gebruiken vertex attribute 0

	if (glDebugMessageCallback)
	{
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}

	rotation = 0;
	lastTime = glutGet(GLUT_ELAPSED_TIME);

	glm::vec3 color(1, 1, 1);

	glm::vec3 n(0, 1, 0);

	vertices.push_back(Vertex(glm::vec3(-1000, 10, -1000), color * 3.0f, glm::vec2(-50, -50), n));
	vertices.push_back(Vertex(glm::vec3(1000, 10, -1000), color * 3.0f, glm::vec2(50, -50), n));
	vertices.push_back(Vertex(glm::vec3(1000, 10, 1000), color * 3.0f, glm::vec2(50, 50), n));

	vertices.push_back(Vertex(glm::vec3(-1000, 10, -1000), color * 3.0f, glm::vec2(-50, -50), n));
	vertices.push_back(Vertex(glm::vec3(-1000, 10, 1000), color * 3.0f, glm::vec2(-50, 50), n));
	vertices.push_back(Vertex(glm::vec3(1000, 10, 1000), color * 3.0f, glm::vec2(50, 50), n));

	vertices.push_back(Vertex(glm::vec3(-1000, -2, -1000), color * 3.0f, glm::vec2(-50, -50), -n));
	vertices.push_back(Vertex(glm::vec3(1000, -2, -1000), color * 3.0f, glm::vec2(50, -50), -n));
	vertices.push_back(Vertex(glm::vec3(1000, -2, 1000), color * 3.0f, glm::vec2(50, 50), -n));

	vertices.push_back(Vertex(glm::vec3(-1000, -2, -1000), color * 3.0f, glm::vec2(-50, -50), -n));
	vertices.push_back(Vertex(glm::vec3(-1000, -2, 1000), color * 3.0f, glm::vec2(-50, 50), -n));
	vertices.push_back(Vertex(glm::vec3(1000, -2, 1000), color * 3.0f, glm::vec2(50, 50), -n));


	double inc = M_PI / 15;

	for (int i = 0; i < 500; i++)
	{
		glm::vec3 pos(45 -30 * (i%4), 0, -15 * (i/4));
		for (double d = 0; d < 2 * M_PI; d += inc)
		{
			double dd = d + inc;
			glm::vec3 n1(cos(d), 0, sin(d));
			glm::vec3 n2(cos(dd), 0, sin(dd));

			vertices.push_back(Vertex(pos + 3.0f * glm::vec3(cos(d), -2, sin(d)), color, glm::vec2(d/(2*M_PI), 0), n1));
			vertices.push_back(Vertex(pos + 3.0f * glm::vec3(cos(dd), -2, sin(dd)), color, glm::vec2(dd / (2 * M_PI), 0), n2));
			vertices.push_back(Vertex(pos + 3.0f * glm::vec3(cos(d), 10, sin(d)), color, glm::vec2(d / (2 * M_PI), 2), n1));

			vertices.push_back(Vertex(pos + 3.0f * glm::vec3(cos(d), 10, sin(d)), color, glm::vec2(d / (2 * M_PI), 2), n1));
			vertices.push_back(Vertex(pos + 3.0f * glm::vec3(cos(dd), -2, sin(dd)), color, glm::vec2(dd / (2 * M_PI), 0), n2));
			vertices.push_back(Vertex(pos + 3.0f * glm::vec3(cos(dd), 10, sin(dd)), color, glm::vec2(dd / (2 * M_PI), 2), n2));
		}
	}

	
	for (int i = -1; i <= 1; i+=2)
	{
		cube.push_back(Vertex(glm::vec3(i, -1, -1), color, glm::vec2(0, 0), glm::vec3(i, 0, 0)));
		cube.push_back(Vertex(glm::vec3(i, 1, -1), color, glm::vec2(1, 0), glm::vec3(i, 0, 0)));
		cube.push_back(Vertex(glm::vec3(i, 1, 1), color, glm::vec2(1, 1), glm::vec3(i, 0, 0)));
		cube.push_back(Vertex(glm::vec3(i, -1, 1), color, glm::vec2(0, 1), glm::vec3(i, 0, 0)));

		cube.push_back(Vertex(glm::vec3(-1, i, -1), color, glm::vec2(0, 0), glm::vec3(0, i, 0)));
		cube.push_back(Vertex(glm::vec3(1, i, -1), color, glm::vec2(1, 0), glm::vec3(0, i, 0)));
		cube.push_back(Vertex(glm::vec3(1, i, 1), color, glm::vec2(1, 1), glm::vec3(0, i, 0)));
		cube.push_back(Vertex(glm::vec3(-1, i, 1), color, glm::vec2(0, 1), glm::vec3(0, i, 0)));

		cube.push_back(Vertex(glm::vec3(-1, -1, i), color, glm::vec2(0, 0), glm::vec3(0, 0, i)));
		cube.push_back(Vertex(glm::vec3(1, -1, i), color, glm::vec2(1, 0), glm::vec3(0, 0, i)));
		cube.push_back(Vertex(glm::vec3(1, 1, i), color, glm::vec2(1, 1), glm::vec3(0, 0, i)));
		cube.push_back(Vertex(glm::vec3(-1, 1,i), color, glm::vec2(0, 1), glm::vec3(0, 0, i)));

	}




	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	int w, h, comp;
	unsigned char* data = stbi_load("test.png", &w, &h, &comp, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	stbi_image_free(data);



	glGenTextures(1, &fboTextureId);
	glBindTexture(GL_TEXTURE_2D, fboTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextureId, 0);

	GLuint rboId;
	glGenRenderbuffers(1, &rboId);
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void display()
{


	Shader* shader = normalShader;



	glm::mat4 mv;
	mv *= glm::lookAt(glm::vec3(0, 3, 20), glm::vec3(0, 5, 0), glm::vec3(0, 1, 0));					//vermenigvuldig met een lookat
	mv = glm::translate(mv, glm::vec3(0, 0, -1));													//of verplaats de camera gewoon naar achter
	//mvp = glm::rotate(mvp, rotation, glm::vec3(0, 1, 0));											//roteer het object een beetje
	
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glViewport(0, 0, 2048, 2048);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader->programId);
	glEnableVertexAttribArray(1);							// en vertex attribute 1
	glEnableVertexAttribArray(2);							// en vertex attribute 2 ook
	glEnableVertexAttribArray(3);							// en vertex attribute 3 ook
	glUniformMatrix4fv(shader->getUniform("projectionMatrix"), 1, 0, glm::value_ptr(glm::perspective(75.0f, (float)screenSize.x / screenSize.y, 0.1f, 5000.0f)));								//en zet de matrix in opengl
	glUniformMatrix4fv(shader->getUniform("modelViewMatrix"), 1, 0, glm::value_ptr(mv));								//en zet de matrix in opengl
	glUniform1f(shader->getUniform("time"), glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
	glUniform1i(shader->getUniform("s_texture"), 0);



	glBindTexture(GL_TEXTURE_2D, textureId);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 11 * 4, &vertices[0]);									//geef aan dat de posities op deze locatie zitten
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 11 * 4, ((float*)&vertices[0]) + 3);					//geef aan dat de kleuren op deze locatie zitten
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 11 * 4, ((float*)&vertices[0]) + 6);					//geef aan dat de texcoords op deze locatie zitten
	glVertexAttribPointer(3, 3, GL_FLOAT, true, 11 * 4, ((float*)&vertices[0]) + 8);					//geef aan dat de texcoords op deze locatie zitten
	
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());																//en tekenen :)

	glUniformMatrix4fv(shader->getUniform("modelViewMatrix"), 1, 0, glm::value_ptr(glm::rotate(glm::translate(mv, glm::vec3(0, 5, 0)), rotation, glm::normalize(glm::vec3(0.5, 1, 0.25)))));								//onleesbare code
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 11 * 4, &cube[0]);									//geef aan dat de posities op deze locatie zitten
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 11 * 4, ((float*)&cube[0]) + 3);					//geef aan dat de kleuren op deze locatie zitten
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 11 * 4, ((float*)&cube[0]) + 6);					//geef aan dat de texcoords op deze locatie zitten
	glVertexAttribPointer(3, 3, GL_FLOAT, true, 11 * 4, ((float*)&cube[0]) + 8);					//geef aan dat de texcoords op deze locatie zitten
	glDrawArrays(GL_QUADS, 0, cube.size());																//en tekenen :)






	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenSize.x, screenSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




	//done drawing, render to screen



	shader = postShader;
	std::vector<glm::vec2> verts;
	verts.push_back(glm::vec2(-1, -1));
	verts.push_back(glm::vec2(1, -1));
	verts.push_back(glm::vec2(1, 1));
	verts.push_back(glm::vec2(-1, 1));

	glUseProgram(shader->programId);
	glEnableVertexAttribArray(1);							// en vertex attribute 1
	glEnableVertexAttribArray(2);							// en vertex attribute 2 ook
	glEnableVertexAttribArray(3);							// en vertex attribute 3 ook
	glUniform1i(shader->getUniform("s_texture"), 0);

	glBindTexture(GL_TEXTURE_2D, fboTextureId);
	glDisableVertexAttribArray(1);							// disable vertex attribute 1
	glDisableVertexAttribArray(2);							// en vertex attribute 2 ook
	glDisableVertexAttribArray(3);							// en vertex attribute 3 ook
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * 4, &verts[0]);									//geef aan dat de posities op deze locatie zitten
	glDrawArrays(GL_QUADS, 0, verts.size());																//en tekenen :)



	glutSwapBuffers();
}

void reshape(int newWidth, int newHeight)
{
	screenSize.x = newWidth;
	screenSize.y = newHeight;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == VK_ESCAPE)
		glutLeaveMainLoop();
}

void update()
{
	float time = glutGet(GLUT_ELAPSED_TIME);
	float elapsed = time - lastTime;
	
	
	rotation += elapsed / 1000.0f;



	glutPostRedisplay();
	lastTime = time;
}




int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(1900, 1000);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Visualisatietechnieken");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(update);

	init();
	
	
	glutMainLoop();

}