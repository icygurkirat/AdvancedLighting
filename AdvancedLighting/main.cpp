#define TITLE "Advanced Lighting"
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Helper/Shader.h"
#include "Helper/camera.h"
#include "Helper/LoadModel/model.h"
#include <iostream>
#include <string>

using namespace std;

//GLOBAL variables:-
int Width = 2000, Height = 2000;
GLFWwindow* window;
unsigned int VBO;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float mouseX = 0.0f;
float mouseY = 0.0f;
bool firstMouse = true;

glm::vec3 lightPos(1.2f, 10.0f, 2.0f);


void init();
void resize(GLFWwindow* window, int width, int height);
void render();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
Shader currShader;
Model model;
Camera camera;


void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

int main() {
	init();

	//Initializing the transformation matrices:-
	glm::mat4 model1 = glm::mat4();
	currShader.setMatf4("model", glm::value_ptr(model1));


	glm::mat4 view = camera.GetViewMatrix();
	currShader.setMatf4("view", glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), ((float)Width) / Height, 0.1f, 100.0f);
	currShader.setMatf4("projection", glm::value_ptr(projection));

	//rendering loop
	while (!glfwWindowShouldClose(window)) {
		render();
	}

	glfwTerminate();
	return 0;
}

void init() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(Width, Height, TITLE, NULL, NULL);
	if (window == NULL) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);


	//GLAD Initialization:-
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << endl;
		exit(EXIT_FAILURE);
	}


	glViewport(0, 0, Width, Height);
	glfwSetFramebufferSizeCallback(window, resize);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	//Wireframe mode:-
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	camera = Camera(0.0f, 0.0f, 3.0f, 0.0f, 1.0f, 0.0f, -90.0f, 0.0f);
	camera.MovementSpeed = 5.0f;

	string location = "resources\\3D_Models\\nanoSuit\\nanosuit.obj";
	model = Model(&location[0]);
	currShader = Shader("resources\\shaders\\VertexShader.glsl", "resources\\shaders\\FragmentShader.glsl");
	currShader.use();

	currShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
	currShader.setFloat("material.shininess", 32.0f);
	currShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	currShader.setVec3("light.diffuse", 0.75f, 0.75f, 0.75f); 
	currShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
}

void resize(GLFWwindow* window, int width, int height) {
	Width = width;
	Height = height;
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), ((float)Width) / Height, 0.1f, 100.0f);
	currShader.setMatf4("projection", glm::value_ptr(projection));
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse) {
		mouseX = xpos;
		mouseY = ypos;
		firstMouse = false;
		return;
	}

	float xoffset = xpos - mouseX;
	float yoffset = mouseY - ypos;
	mouseX = xpos;
	mouseY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset, true);
}

void render() {
	processInput(window);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float timeValue = glfwGetTime();

	deltaTime = timeValue - lastFrame;
	lastFrame = timeValue;

	lightPos.x = 5.0f*cos(timeValue);
	lightPos.z = 5.0f*sin(timeValue);
	
	glm::mat4 view = camera.GetViewMatrix();
	currShader.setMatf4("view", glm::value_ptr(view));
	currShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
	model.Draw(currShader);
	currShader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glfwPollEvents();
	glfwSwapBuffers(window);
}