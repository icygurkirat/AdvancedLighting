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
#include <random>
#include <fstream>

using namespace std;

//GLOBAL variables:-
int Width = 2000, Height = 2000;
GLFWwindow* window;
unsigned int quadVAO, quadVBO;	//quad for deferred shading
unsigned int kernelSBO;			//SBO for storing kernel samples
//SSAO Variables:-
//SSAO first pass: frame buffer and its data
unsigned int gbufferFBO, gPosition, gNormal, gAlbedo, gPositionModel, gNormalModel;	
//SSAO second and third pass:-
unsigned int ssaoFBO, ssaoBlurFBO;
unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
unsigned int noiseTexture;	//random rotation vectors to reduce banding
//SSAO shader params
float ssaoRadius = 0.5f, ssaoEps = 0.025f;
int SSAOKernels = 25;
//Shadow map variables
unsigned int shadowCubeMap, shadowFBO;
unsigned int shadowWidth = Width, shadowHeight = Height;
glm::mat4 shadowProj;
//Particle System Params
int NumParticles = 130, lastUnused = 0;
float ParticleLife = 2.0;	//in seconds
unsigned int smokeTex, particleSBO;

//Probability distribution and sampler
uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
default_random_engine generator;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float mouseX = 0.0f;
float mouseY = 0.0f;
bool firstMouse = true;

glm::vec3 lightPos(2.9f, 2.9f, 0.0f);
int arg = 0, argTex;
bool arg1 = true, arg2 = true, arg3 = true;


void init();
void resize(GLFWwindow* window, int width, int height);
void render();
void CreateVBO();
void initFBO();
void initUniforms();
void generateKernels();
void updateParticles();
void drawScene(Shader shader);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
float lerp(float a, float b, float f);
Shader sceneShader, SSAOShader, blurShader, lightingShader, dispShader, shadowShader;
Shader particleCompute, particleRender;
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
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
		arg = 0;
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		arg = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		arg = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		arg = 3;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		arg1 = true;
		lightingShader.use();
		lightingShader.setBool("arg1", arg1);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		arg1 = false;
		lightingShader.use();
		lightingShader.setBool("arg1", arg1);
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		arg2 = true;
		lightingShader.use();
		lightingShader.setBool("arg2", arg2);
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		arg2 = false;
		lightingShader.use();
		lightingShader.setBool("arg2", arg2);
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
		arg3 = true;
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
		arg3 = false;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		cout << "# of kernels: " << SSAOKernels << "\t Kernel radius: " << ssaoRadius;
		cout << "\t Float comparison epsilon: " << ssaoEps << endl;
		cout << "Enter 1, 2 or 3 in terminal to change corresponding param" << endl;
		int x;
		cin >> x;
		switch (x) {
		case 1:
			cout << "Enter new value of # of samples" << endl;
			cin >> x;
			SSAOKernels = x;
			SSAOShader.use();
			generateKernels();
			SSAOShader.setInt("kernelSize", x);
			break;
		case 2:
			SSAOShader.use();
			float y;
			cout << "Enter new value of radius" << endl;
			cin >> y;
			SSAOShader.setFloat("radius", y);
			ssaoRadius = y;
			break;
		case 3:
			SSAOShader.use();
			cout << "Enter new value of eps" << endl;
			cin >> y;
			SSAOShader.setFloat("eps", y);
			ssaoEps = y;
			break;
		default:
			cout << "Invalid input....Exiting edit window" << endl;
		}
	}
}

int main() {
	ifstream file;
	file.open("resources\\args.txt");
	string tmp;
	file >> tmp; file >> Width;
	file >> tmp; file >> Height;
	file >> tmp; file >> shadowWidth;
	file >> tmp; file >> NumParticles;
	file >> tmp; file >> ParticleLife;
	shadowHeight = shadowWidth;
	cout << "Screen Size: " << Width << "x" << Height << endl;
	cout << "Shadow Map resolution: " << shadowWidth << "x" << shadowHeight << endl;
	cout << "Number of particles: " << NumParticles << endl;
	cout << "Life of each particle: " << ParticleLife << " seconds" << endl;
	cout << "---------------------------------------------------" << endl;
	file.close();

	init();
	cout << "Creating FrameBuffers for multiple shader passes...";
	initFBO();
	cout << "DONE" << endl<<"Initializing shader uniforms...";
	initUniforms();
	cout << "DONE" << endl << endl;

	cout << "---------------------------------------------------" << endl;
	cout << "Press these keys when rendering window is active, for the corresponding changes" << endl;
	cout << "[1]: Gbuffer albedo pass. Simply displays the 3D models" << endl;
	cout << "[2]: SSAO shader pass. Dark regions represent occlusion" << endl;
	cout << "[3]: SSAO Blur pass. Blurring of SSAO Frame buffer" << endl;
	cout << "[0]: All shaders (Albedo + SSAO + Illumination + Particle System). Press the following keys to modify this framebuffer" << endl;
	cout << "[4]: Enable SSAO" << endl;
	cout << "[5]: Disbale SSAO" << endl;
	cout << "[6]: Enable Shadows" << endl;
	cout << "[7]: Disable Shadows" << endl;
	cout << "[8]: Enable Particle System" << endl;
	cout << "[9]: Disable Particle System" << endl;
	cout << "[P]: Change SSAO parameters" << endl;
	cout << "Illumination params can be changed directly in lighting.fs shader file" << endl;
	cout << "Modify resources/args.txt to change rendering params" << endl;
	cout << "---------------------------------------------------" << endl;

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
	cout << "Creating OpenGL context...";
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
	cout << "DONE" << endl;


	//GLAD Initialization:-
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << endl;
		exit(EXIT_FAILURE);
	}
	cout << "GLAD Initialized" << endl;


	glViewport(0, 0, Width, Height);
	glfwSetFramebufferSizeCallback(window, resize);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Wireframe mode:-
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	camera = Camera(0.0f, 0.0f, 3.0f, 0.0f, 1.0f, 0.0f, -90.0f, 0.0f);
	camera.MovementSpeed = 5.0f;

	CreateVBO();

	cout << "Loading Shaders...";
	sceneShader = Shader("resources\\shaders\\scene.vs", "resources\\shaders\\scene.fs");
	SSAOShader = Shader("resources\\shaders\\ssao.vs", "resources\\shaders\\ssao.fs");
	blurShader = Shader("resources\\shaders\\ssao.vs", "resources\\shaders\\blur.fs");
	lightingShader = Shader("resources\\shaders\\ssao.vs", "resources\\shaders\\lighting.fs");
	dispShader = Shader("resources\\shaders\\ssao.vs", "resources\\shaders\\drawQuad.fs");
	shadowShader = Shader("resources\\shaders\\shadow.vs", "resources\\shaders\\shadow.fs", "resources\\shaders\\shadow.gs");
	particleCompute.computeShader("resources\\shaders\\particle.compute");
	particleRender = Shader("resources\\shaders\\particle.vs", "resources\\shaders\\particle.fs");
	cout << "DONE\n";

	//Loading all the models:-
	//stbi_set_flip_vertically_on_load(true);
	cout << "Loading 3D Models...";
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
	cout << "DONE" << endl;
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

	lightPos.x = 2.9f*cos(timeValue);
	lightPos.z = 2.9f*sin(timeValue);
	glm::mat4 view = camera.GetViewMatrix();
	glm::vec3 viewPos = camera.Position;
	
	vector<glm::mat4> shadowTransforms;


	//PHASE 1: Render the scene geometry to GBUFFER
	glViewport(0, 0, Width, Height);
	glBindFramebuffer(GL_FRAMEBUFFER, gbufferFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	sceneShader.use();
	sceneShader.setMatf4("view", glm::value_ptr(view));
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMap);
	drawScene(sceneShader);
	if (arg == 1) {
		argTex = gAlbedo;
		dispShader.use();
		dispShader.setBool("gray", false);
		glBindVertexArray(quadVAO);
		goto PHASE_LAST;
	}
	if (!arg1)
		goto PHASE_SHADOW;
	

	

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
	if (arg == 2) {
		argTex = ssaoColorBuffer;
		dispShader.use();
		dispShader.setBool("gray", true);
		goto PHASE_LAST;
	}


	//PHASE 3: SSAO Blur stage:-
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	blurShader.use();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	if (arg == 3) {
		argTex = ssaoColorBufferBlur;
		dispShader.use();
		dispShader.setBool("gray", true);
		goto PHASE_LAST;
	}


PHASE_SHADOW:
	if (!arg2)
		goto PHASE_PHONG;
	//PHASE 4: Rendering to shadow map
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	glViewport(0, 0, shadowWidth, shadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	shadowShader.use();
	for (unsigned int i = 0; i < 6; ++i)
		shadowShader.setMatf4("shadowMatrices[" + std::to_string(i) + "]", glm::value_ptr(shadowTransforms[i]));
	shadowShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
	drawScene(shadowShader);


PHASE_PHONG:
	//Blinn-Phong illumination:- 
	glViewport(0, 0, Width, Height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(quadVAO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPositionModel);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalModel);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMap);
	lightingShader.use();
	lightingShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
	lightingShader.setVec3("viewPos", viewPos.x, viewPos.y, viewPos.z);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//PHASE 0: Render particles
	if (arg3) {
		updateParticles();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, smokeTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		particleRender.use();
		particleRender.setMatf4("view", glm::value_ptr(view));
		particleRender.setVec3("camPos", viewPos.x, viewPos.y, viewPos.z);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, NumParticles);
	}
	goto RENDER_END;



	//PHASE LAST: Displaying on quad:-
PHASE_LAST:
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, argTex);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

RENDER_END:
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void CreateVBO() {
	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.5f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.5f, 0.0f, 0.0f,
		1.0f,  1.0f, 0.5f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.5f, 1.0f, 0.0f,
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


	//Loading Smoke Texture
	glGenTextures(1, &smokeTex);
	glBindTexture(GL_TEXTURE_2D, smokeTex);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int a, b, c;
	unsigned char *image = stbi_load("resources\\ParticleAtlas.png", &a, &b, &c, 0);
	if (image) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, a, b, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		cout << "Could not load the Particle Atlas texture image" << endl;
	stbi_image_free(image);

	//Buffer object for particle data:-
	glGenBuffers(1, &particleSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSBO);
	//initial data: all dead particles
	float *temp = new float[NumParticles * 8];
	for (int i = 0; i < NumParticles * 8; i++)
		temp[i] = ParticleLife + 1.0;
	glBufferData(GL_SHADER_STORAGE_BUFFER, NumParticles * 8 * sizeof(float), temp, GL_STREAM_DRAW);
	free(temp);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleSBO);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, particleSBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(4);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
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
	// Model view position color buffer
	glGenTextures(1, &gPositionModel);
	glBindTexture(GL_TEXTURE_2D, gPositionModel);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gPositionModel, 0);
	// normal Model color buffer
	glGenTextures(1, &gNormalModel);
	glBindTexture(GL_TEXTURE_2D, gNormalModel);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gNormalModel, 0);

	// tell OpenGL which color attachments we'll use for rendering 
	unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);
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
	//UBO for storing kernel samples
	glGenBuffers(1, &kernelSBO);
	generateKernels();
	vector<glm::vec3> ssaoNoise;
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
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Width, Height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	cout << "SSAO Blur Framebuffer not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//Shadow map frame buffer:-
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glGenTextures(1, &shadowCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMap);
	for (int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCubeMap, 0);
	//no need to render to a color buffer:-
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "Shadow Map Framebuffer not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float lerp(float a, float b, float f) {
	//linear interpolation
	return a + f * (b - a);
}

void initUniforms() {
	//Initializing the transformation matrices:-
	sceneShader.use();
	glm::mat4 model = glm::mat4();
	sceneShader.setMatf4("model", glm::value_ptr(model));

	glm::mat4 view = camera.GetViewMatrix();
	sceneShader.setMatf4("view", glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), ((float)Width) / Height, 0.1f, 100.0f);
	sceneShader.setMatf4("projection", glm::value_ptr(projection));

	shadowProj = glm::perspective(glm::radians(90.0f),
		((float)shadowWidth) / shadowHeight, 0.5f, 25.0f);

	shadowShader.use();
	shadowShader.setFloat("farPlane", 25.0f);


	SSAOShader.use();
	SSAOShader.setMatf4("projection", glm::value_ptr(projection));
	SSAOShader.setInt("gPosition", 0);
	SSAOShader.setInt("gNormal", 1);
	SSAOShader.setInt("texNoise", 2);
	SSAOShader.setInt("kernelSize", SSAOKernels);
	// tile noise texture over screen based on screen dimensions divided by noise size
	SSAOShader.setVec2("noiseScale", Width / 4.0, Height / 4.0);
	SSAOShader.setFloat("radius", ssaoRadius);		//Kernel radius
	SSAOShader.setFloat("eps", ssaoEps);		//float comparison epsilon

	blurShader.use();
	blurShader.setInt("blurKernal", 4);
	blurShader.setInt("ssao", 0);

	dispShader.use();
	dispShader.setInt("albedo", 0);

	lightingShader.use();
	lightingShader.setInt("gPosition", 0);
	lightingShader.setInt("gNormal", 1);
	lightingShader.setInt("gAlbedo", 2);
	lightingShader.setInt("ssao", 3);
	lightingShader.setInt("shadowMap", 4);
	lightingShader.setFloat("farPlane", 25.0f);
	lightingShader.setBool("arg1", arg1);
	lightingShader.setBool("arg2", arg2);

	particleRender.use();
	particleRender.setInt("smoke", 0);
	particleRender.setInt("gPosition", 1);
	particleRender.setFloat("maxLife", ParticleLife);
	particleRender.setMatf4("proj", glm::value_ptr(projection));
	particleRender.setInt("screenWidth", Width);
	particleRender.setInt("screenHeight", Height);

	particleCompute.use();
	particleCompute.setFloat("maxLife", ParticleLife);


	glBindVertexArray(quadVAO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, kernelSBO);
}

void generateKernels() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, kernelSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, SSAOKernels * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
	//generating sample kernels:-
	for (unsigned int i = 0; i < SSAOKernels; ++i) {
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = ((float)i) / SSAOKernels;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, i * 4 * sizeof(float), 3 * sizeof(float), glm::value_ptr(sample));
	}
}

void drawScene(Shader shader) {
	shader.use();
	// nanosuit and dresser models 
	glEnable(GL_CULL_FACE);
	shader.setMatf4("model", glm::value_ptr(models[1].matrix));
	models[1].model.Draw(shader);
	shader.setMatf4("model", glm::value_ptr(models[2].matrix));
	models[2].model.Draw(shader);
	// room cube and deagle
	glDisable(GL_CULL_FACE);
	shader.setMatf4("model", glm::value_ptr(models[0].matrix));
	models[0].model.Draw(shader);
	shader.setMatf4("model", glm::value_ptr(models[3].matrix));
	models[3].model.Draw(shader);
}

void updateParticles() {
	//generate new particles
	int deltaN = (int)(((float)NumParticles / ParticleLife)*deltaTime);
	deltaN = max(1, deltaN);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSBO);
	for (int i = 1; i <= deltaN; i++) {
		float data[8];
		//time since birth
		data[0] = 0.0;	
		//initial velocity
		data[1] = (randomFloats(generator) * 2.0 - 1.0) / 30.0;
		data[2] = 0.0;// (randomFloats(generator) * 2.0 - 1.0) / 10.0;
		data[3] = 0.0;
		//current position
		data[4] = -2.16f;
		data[5] = -1.45f;
		data[6] = -2.0f;
		//size
		data[7] = (randomFloats(generator)) / 10.0;

		//updating Storage Buffer
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, lastUnused * 8 * sizeof(float), 8 * sizeof(float), &data);

		lastUnused++;
		if (lastUnused == NumParticles)
			lastUnused = 0;
	}

	//updating the positions of all particles
	particleCompute.use();
	particleCompute.setFloat("deltaTime", deltaTime);
	glDispatchCompute(NumParticles, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}