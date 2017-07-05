#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "ObjModel.h"
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

std::vector<Shader*> shaders;
std::vector<ObjModel*> models;
std::vector<float> distances;
int activeModel = 0;

int currentShader;

glm::ivec2 screenSize;
float rotation;
float lastTime;


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
	glLinkProgram(ret->programId);								// link het programma, zorg dat alle attributes en varying gelinked zijn
	glUseProgram(ret->programId);								// Zet dit als actieve programma

	return ret;
}

void init()
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glClearColor(1, 0.7f, 0.3f, 1.0f);


	currentShader = 0;
	shaders.push_back(loadShader("simple.vs", "simple.fs"));
	shaders.push_back(loadShader("multitex.vs", "multitex.fs"));
	shaders.push_back(loadShader("textureanim.vs", "textureanim.fs"));
	shaders.push_back(loadShader("texture.vs", "texture.fs"));
	shaders.push_back(loadShader("vertexanim.vs", "vertexanim.fs"));

	models.push_back(new ObjModel("models/ship/shipA_OBJ.obj"));
	distances.push_back(50);
	models.push_back(new ObjModel("models/car/honda_jazz.obj"));
	distances.push_back(150);
	//	model = new ObjModel("models/bloemetje/PrimroseP.obj");
	models.push_back(new ObjModel("models/normalstuff/normaltest.obj"));
	distances.push_back(2);
	models.push_back(new ObjModel("models/normalstuff/normaltest2.obj"));
	distances.push_back(2);


	glEnableVertexAttribArray(0);							// positie
	glEnableVertexAttribArray(1);							// texcoord
	glEnableVertexAttribArray(2);							// normal
	glEnableVertexAttribArray(3);							// tangent

	if (glDebugMessageCallback)
	{
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}

	rotation = 0;
	lastTime = glutGet(GLUT_ELAPSED_TIME);


}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	Shader* shader = shaders[currentShader];

	glm::mat4 projection = glm::perspective(70.0f, screenSize.x / (float)screenSize.y, 0.01f, 200.0f);		//begin met een perspective matrix
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, distances[activeModel]), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));					//vermenigvuldig met een lookat
	glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(0, 0, -1));													//of verplaats de camera gewoon naar achter
	model = glm::rotate(model, rotation, glm::vec3(0, 1, 0));											//roteer het object een beetje
	
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));

	glUseProgram(shader->programId);

	glUniformMatrix4fv(shader->getUniform("modelMatrix"), 1, 0, glm::value_ptr(model));								//en zet de matrix in opengl
	glUniformMatrix4fv(shader->getUniform("viewMatrix"), 1, 0, glm::value_ptr(view));								//en zet de matrix in opengl
	glUniformMatrix4fv(shader->getUniform("projectionMatrix"), 1, 0, glm::value_ptr(projection));								//en zet de matrix in opengl
	glUniformMatrix3fv(shader->getUniform("normalMatrix"), 1, 0, glm::value_ptr(normalMatrix));								//en zet de matrix in opengl
	glUniform1f(shader->getUniform("time"), glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
	glUniform1i(shader->getUniform("s_texture"), 0);

	models[activeModel]->draw();

													//en tekenen :)

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
	if (key == 'a')
		currentShader = (currentShader + shaders.size() - 1) % shaders.size();
	if (key == 'd')
		currentShader = (currentShader + 1) % shaders.size();
	if (key == 'w' || key == 's')
		activeModel = (activeModel + 1) % models.size();
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