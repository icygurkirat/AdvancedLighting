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
//Model model;
Camera camera;


struct ModMat {
	float scale, thetax, thetay, thetaz;
	float x, y, z;
	glm::mat4 matrix;
	Model model;
	ModMat(string path) {
		scale = 1.0; thetax = 0; thetay = 0; thetaz = 0;
		x = 0; y = 0; z = 0;
		matrix = glm::mat4();
		model = Model(&path[0]);
	}
	ModMat(){}
	
	void update() {
		matrix = glm::mat4();
		matrix = glm::translate(matrix, glm::vec3(x, y, z));
		matrix = glm::rotate(matrix, glm::radians(thetaz), glm::vec3(0.0, 0.0, 1.0));
		matrix = glm::rotate(matrix, glm::radians(thetay), glm::vec3(0.0, 1.0, 0.0));
		matrix = glm::rotate(matrix, glm::radians(thetax), glm::vec3(1.0, 0.0, 0.0));
		matrix = glm::scale(matrix, glm::vec3(scale));
		cout << "-------------------" << endl;
		cout << "Scale = " << scale << endl;
		cout << "theta = (" << thetax << ", " << thetay << ", " << thetaz << ")\n";
		cout << "Pos = (" << x << ", " << y << ", " << z << ")\n";
	}
};

int curr = 3;

ModMat models[5];


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
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		models[curr].scale += 0.01;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		models[curr].scale -= 0.1;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		models[curr].thetax += 15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		models[curr].thetax -= 15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		models[curr].thetay += 15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		models[curr].thetay -= 15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		models[curr].thetaz += 15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
		models[curr].thetaz -= 15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		models[curr].x += 0.15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		models[curr].x -= 0.15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		models[curr].y += 0.15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		models[curr].y -= 0.15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
		models[curr].z += 0.15;
		models[curr].update();
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		models[curr].z -= 0.15;
		models[curr].update();
	}
}

int main() {
	init();

	//Initializing the transformation matrices:-


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

	//Loading all the models:-
	models[0] = ModMat("resources\\3D_Models\\Box\\box.obj");
	models[0].scale = 7.0f;
	models[0].update();

	models[1] = ModMat("resources\\3D_Models\\nanoSuit\\nanosuit.obj");
	models[1].scale = 0.3f;
	models[1].thetay = -90.0f;
	models[1].x = 1.8f;
	models[1].y = -3.45f;
	models[1].z = 1.95f;
	models[1].update();

	models[2] = ModMat("resources\\3D_Models\\eb_dresser_01\\eb_dresser_01.obj");
	models[2].scale = 0.0200007f;
	models[2].x = -1.2f;
	models[2].y = -3.45f;
	models[2].z = -3.3f;
	models[2].update();

	models[3] = ModMat("resources\\3D_Models\\Deagle\\deagle.obj");
	models[3].scale = -0.0900013f;
	models[3].thetax = 45.0f;
	models[3].thetaz = -90.0f;
	models[3].x = -1.2f;
	models[3].y = -1.35f;
	models[3].z = -2.7f;
	models[3].update();

	//models[4] = ModMat("resources\\3D_Models\\Chair\\chair.obj");


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
	currShader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
	currShader.setMatf4("view", glm::value_ptr(view));
	currShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);

	currShader.setMatf4("model", glm::value_ptr(models[0].matrix));
	models[0].model.Draw(currShader);

	currShader.setMatf4("model", glm::value_ptr(models[1].matrix));
	models[1].model.Draw(currShader);

	currShader.setMatf4("model", glm::value_ptr(models[2].matrix));
	models[2].model.Draw(currShader);

	currShader.setMatf4("model", glm::value_ptr(models[3].matrix));
	models[3].model.Draw(currShader);

	
	glfwPollEvents();
	glfwSwapBuffers(window);
}