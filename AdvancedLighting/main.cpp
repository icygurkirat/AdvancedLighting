#define TITLE "Advanced Lighting"
#define STB_IMAGE_IMPLEMENTATION
#define SSAOKernels 50

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
#include <random>

using namespace std;

//GLOBAL variables:-
int Width = 2000, Height = 2000;
GLFWwindow* window;
unsigned int quadVAO, quadVBO;	//quad for deferred shading
//SSAO Variables:-
//SSAO first pass: frame buffer and its data
unsigned int gbufferFBO, gPosition, gNormal, gAlbedo;	
//SSAO second and third pass:-
unsigned int ssaoFBO, ssaoBlurFBO;
unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
unsigned int noiseTexture;	//random rotation vectors to reduce banding

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float mouseX = 0.0f;
float mouseY = 0.0f;
bool firstMouse = true;

glm::vec3 lightPos(1.2f, 10.0f, 2.0f);


void init();
void resize(GLFWwindow* window, int width, int height);
void render();
void CreateVBO();
void initFBO();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
float lerp(float a, float b, float f);
Shader sceneShader, SSAOShader, blurShader, lightingShader, dispShader;
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
	}
};

ModMat models[4];


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
	initFBO();

	//Initializing the transformation matrices:-
	sceneShader.use();
	glm::mat4 model = glm::mat4();
	sceneShader.setMatf4("model", glm::value_ptr(model));

	glm::mat4 view = camera.GetViewMatrix();
	sceneShader.setMatf4("view", glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), ((float)Width) / Height, 0.1f, 100.0f);
	sceneShader.setMatf4("projection", glm::value_ptr(projection));

	SSAOShader.use();
	SSAOShader.setMatf4("projection", glm::value_ptr(projection));
	SSAOShader.setInt("gPosition", 0);
	SSAOShader.setInt("gNormal", 1);
	SSAOShader.setInt("texNoise", 2);

	//rendering loop
	while (!glfwWindowShouldClose(window)) {
		render();
	}

	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);

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

	CreateVBO();

	sceneShader = Shader("resources\\shaders\\scene.vs", "resources\\shaders\\scene.fs");
	SSAOShader = Shader("resources\\shaders\\ssao.vs", "resources\\shaders\\ssao.fs");
	//blurShader = Shader("ssao.vs", "blur.fs");
	//lightingShader = Shader("ssao.vs", "lighting.fs");
	dispShader = Shader("resources\\shaders\\ssao.vs", "resources\\shaders\\drawQuad.fs");

	//Loading all the models:-
	//stbi_set_flip_vertically_on_load(true);
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
	models[3].thetax = -45.0f;
	models[3].thetaz = 90.0f;
	models[3].x = -1.0f;
	models[3].y = -1.55f;
	models[3].z = -2.7f;
	models[3].update();
}

void resize(GLFWwindow* window, int width, int height) {
	Width = width;
	Height = height;
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), ((float)Width) / Height, 0.1f, 100.0f);
	sceneShader.setMatf4("projection", glm::value_ptr(projection));
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
	float timeValue = glfwGetTime();

	deltaTime = timeValue - lastFrame;
	lastFrame = timeValue;

	lightPos.x = 5.0f*cos(timeValue);
	lightPos.z = 5.0f*sin(timeValue);



	//PHASE 1: Render the scene geometry to GBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	sceneShader.use();
	sceneShader.setMatf4("view", glm::value_ptr(view));
	// nanosuit and dresser models 
	glEnable(GL_CULL_FACE);
	sceneShader.setMatf4("model", glm::value_ptr(models[1].matrix));
	models[1].model.Draw(sceneShader);
	sceneShader.setMatf4("model", glm::value_ptr(models[2].matrix));
	models[2].model.Draw(sceneShader);
	// room cube and deagle
	glDisable(GL_CULL_FACE);
	sceneShader.setMatf4("model", glm::value_ptr(models[0].matrix));
	models[0].model.Draw(sceneShader);
	sceneShader.setMatf4("model", glm::value_ptr(models[3].matrix));
	models[3].model.Draw(sceneShader);
	

	

	//PHASE 2: SSAO stage:-
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glBindVertexArray(quadVAO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	SSAOShader.use();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	//PHASE LAST: Displaying on quad:-
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(quadVAO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	dispShader.use();
	dispShader.setInt("albedo", 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void CreateVBO() {
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void initFBO() {
	//PHASE 1: G buffer:-
	glGenFramebuffers(1, &gbufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFBO);
	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	// tell OpenGL which color attachments we'll use for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width, Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "G Buffer Framebuffer not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//PHASE 2: Aplying SSAO:-
	//generating sample kernels:-
	uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
	default_random_engine generator;
	SSAOShader.use();
	for (unsigned int i = 0; i < 50; ++i) {
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / 50.0;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		SSAOShader.setVec3("samples[" + to_string(i) + "]", sample.x, sample.y, sample.z);
	}

	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++) {
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	// SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "SSAO Framebuffer not complete!" << endl;



	//PHASE 3: blur stage
	/*glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	cout << "SSAO Blur Framebuffer not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
}

float lerp(float a, float b, float f) {
	//linear interpolation
	return a + f * (b - a);
}