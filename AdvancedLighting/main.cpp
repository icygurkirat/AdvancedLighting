#define TITLE "Advanced Lighting"

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "Helper\Shader.h"
#include <iostream>

using namespace std;

//GLOBAL variables:-
int Width = 1920, Height = 1080;
GLFWwindow* window;
unsigned int VBO;


void init();
void resize(GLFWwindow* window, int width, int height);
void render();
void CreateVBO();
Shader currShader;


void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main() {
	init();

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

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	CreateVBO();
	currShader = Shader("resources\\shaders\\VertexShader.glsl", "resources\\shaders\\FragmentShader.glsl");
	currShader.use();
}

void resize(GLFWwindow* window, int width, int height) {
	Width = width;
	Height = height;
	glViewport(0, 0, width, height);
}

void render() {
	processInput(window);

	glClear(GL_COLOR_BUFFER_BIT);

	float timeValue = glfwGetTime();
	float greenValue = (sin(timeValue) / 2.0f);
	currShader.setFloat("offset", greenValue);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glfwPollEvents();
	glfwSwapBuffers(window);
}

void CreateVBO() {
	float vertices[] = {
		// positions         // colors
		0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
		0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
	};

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}