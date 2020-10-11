#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <chrono>    
#include <algorithm>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include "Shader.h"

// Constants
float const WINDOW_WIDTH(1920);
float const WINDOW_HEIGHT(1080);

// Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(100, 100, width - 200, height - 200);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

struct init_res
{
	char const * err_str;
	GLFWwindow * window;
	int return_code;
	init_res() : err_str("not modified after initialization"), window(NULL), return_code(-1) {};
};

init_res init(void)
{
	init_res res;

	if (!glfwInit()) {
		res.err_str = "glfwInit() fails";
		return res;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window
	res.window = glfwCreateWindow(static_cast<int>(WINDOW_WIDTH), static_cast<int>(WINDOW_HEIGHT), "Clarence's awesome game", NULL, NULL);
	if (res.window == NULL) {
		res.err_str = "window is null";
		glfwTerminate();
		return res;
	}
	glfwMakeContextCurrent(res.window);

	// Before calling any GL functions, define them using GLAD
	//gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!gladLoadGL()) {
		res.err_str = "shit happened for gladLoadGL()";
		glfwTerminate();
		return res;
	}

	// Viewport
	glViewport(100, 100, 1720, 880);

	// Register callbacks
	glfwSetFramebufferSizeCallback(res.window, framebufferSizeCallback);
	glfwSetCursorPosCallback(res.window, mouse_callback);
	glfwSetScrollCallback(res.window, scroll_callback);

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// No mouse cursor
	glfwSetInputMode(res.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	res.err_str = "";
	res.return_code = 0;
	return res;
}

void setVisibility(float* visibility, float newAmt)
{
	if (visibility == NULL)return;
	float lo(0), hi(1);
	*visibility = (newAmt < lo) ? lo : (hi < newAmt) ? hi : newAmt;
	std::cout << "visibility: " << *visibility << std::endl;
}

void processInput(GLFWwindow *, float *);

bool createTexture(char const * img_name, GLuint texobj_id)
{
	glBindTexture(GL_TEXTURE_2D, texobj_id);
	// Set options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char *img_data(NULL);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	img_data = stbi_load(img_name, &width, &height, &nrChannels, 0);
	printf("image \"%s\": width: %d, height: %d, nrChannels: %d", img_name, width, height, nrChannels);
	if (img_data == NULL) {
		std::cout << "Fuck. Can't load image \"" << img_name << "\"." << std::endl;
		return GL_FALSE;
	}

	// which format?
	GLint format;
	char const * r_png = strstr(img_name, ".png");
	if (r_png != nullptr) {
		format = GL_RGBA;
	}
	else {
		format = GL_RGB;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, img_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(img_data);	// Free the memory of the texture read
	glBindTexture(GL_TEXTURE_2D, 0);
	return GL_TRUE;
}

float radius(10.0f);
float mouseSensitivity(1.0f);

// dangerous global variables
float deltaTime(0.0f);
float currMousePosX(WINDOW_WIDTH / 2.0f),	prevMousePosX(WINDOW_WIDTH / 2.0f);
float currMousePosY(WINDOW_HEIGHT / 2.0f),	prevMousePosY(WINDOW_HEIGHT / 2.0f);
bool firstEntered = true;

// Camera
Camera cam(glm::vec3(0.0f, 0.0f, 3.0f));

int main()
{
	// Init
	init_res res = init();
	if (res.return_code) {
		std::cout << res.err_str << std::endl;
		return -1;
	}
	GLFWwindow * window = res.window;

	// Read shaders
	Shader cubeShader("cube_color.vs", "cube_color.fs");
	Shader lightSrcShader("light_src.vs", "light_src.fs");
	if (cubeShader.id == -1 || lightSrcShader.id == -1) {
		return -1;
	}
	//init_shaders_res shaders_res = init_shaders();
	/*if (shaders_res.return_code) {
		std::cout << shaders_res.err_str << std::endl;
		return -1;
	}
	glDeleteShader(shaders_res.vertex_shader);
	glDeleteShader(shaders_res.fragment_shader);*/

	// A box
	GLfloat vertices[]{
		// positions			// colors (not using)	// texs
		.3f, .3f,	.3f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		.3f, -.3f,	.3f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-.3f, -.3f, .3f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-.3f, .3f,	.3f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,

		.3f, .3f,  -.3f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		.3f, -.3f, -.3f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		.3f, -.3f,  .3f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		.3f,  .3f,  .3f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,

		.3f, .3f,  -.3f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		.3f, .3f,   .3f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-.3f, .3f,  .3f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-.3f, .3f, -.3f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,

		-.3f, .3f,  .3f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		-.3f, -.3f, .3f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-.3f, -.3f,-.3f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-.3f, .3f, -.3f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,

		-.3f, .3f, -.3f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		-.3f, -.3f,-.3f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		.3f, -.3f, -.3f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		.3f, .3f,  -.3f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,

		.3f, -.3f,  .3f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
		.3f, -.3f, -.3f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-.3f, -.3f,-.3f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-.3f, -.3f, .3f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
	};
	GLuint indices[]{
		0, 1, 2,
		0, 2, 3,
		4,5,6,
		4,6,7,
		8,9,10,
		8,10,11,
		12,13,14,
		12,14,15,
		16,17,18,
		16,18,19,
		20,21,22,
		20,22,23,
	};

	// Create texture
	GLuint gorgeousImg;
	glGenTextures(1, &gorgeousImg);
	//GLuint gorgeousImgs[2];
	//glGenTextures(2, gorgeousImgs);
	// Read texture
	createTexture("ping.png", gorgeousImg);
	//createTexture("ping.png", gorgeousImgs[0]);
	//createTexture("awesomeface.png", gorgeousImgs[1]);

	// Set Vertex Array Object for the cube
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);	// Any subsequent VBO calls will be stored in the current VAO bound
	GLuint VBO;	// Vertex Buffer Object
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Specify what the data in this VBO means. (Set vertex attributes)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);	// positions #0
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));	// colors #1
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));	// texs #2
	glEnableVertexAttribArray(2);
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// VAO for the light source
	unsigned int lightSrcVAO;
	glGenVertexArrays(1, &lightSrcVAO);
	glBindVertexArray(lightSrcVAO);
	// Use the same VBO as the cube
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// set the vertex attribute 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Use the same EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Transform using matrix
	glm::mat4 trans = glm::mat4(1.0f);

	// Uniforms
	//GLuint colorModF = glGetUniformLocation(shaders_res.shader_program, "ColorModF");
	//GLuint transformLoc = glGetUniformLocation(shaders_res.shader_program, "transform");
	//GLuint visibleAmtF = glGetUniformLocation(shaders_res.shader_program, "visibleAmtF");
	// Use the shader program (Have to use the program before setting the uniforms)
	//glUseProgram(shaders_res.shader_program);
	cubeShader.use();
	// Set sampler uniforms (Must set uniforms AFTER using shader programs)
	cubeShader.setUniform1i("texImg0", 0);
	cubeShader.setUniform3f("objectColor", 1.0f, 0.5f, 0.31f);
	cubeShader.setUniform3f("lightColor", 1.0f, 1.0f, 1.0f);
	lightSrcShader.use();

	//glUniform1i(glGetUniformLocation(shaders_res.shader_program, "texImg0"), 0);
	//glUniform3f(glGetUniformLocation(shaders_res.shader_program, "objectColor"), 1.0f, 0.5f, 0.31f);
	//glUniform3f(glGetUniformLocation(shaders_res.shader_program, "lightColor"), 1.0f, 1.0f, 1.0f);
	//glUniform1i(glGetUniformLocation(shaders_res.shader_program, "TexImg1"), 1);

	// Polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// A bunch of cube positions
	glm::vec3 cube_positions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f),
	};

	// Light source position
	glm::vec3 lightSrcPos(1.2f, 1.0f, -2.0f);

	auto t_start = std::chrono::high_resolution_clock::now();
	float lastTime = 0.0f;
	float visibility(.25f);

	// Render loop
	while (!glfwWindowShouldClose(window)) {

		// time and delta time
		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
		deltaTime = time - lastTime;

		// input
		processInput(window, &visibility);

		//// Camera angle
		//updateCameraAngle(&cam, lastTime == 0.0f);

		/*	float x = sin(time * 3.0f);
			float y = sin(time * 2.0f);
			float z = sin(time * 1.0f);*/

		// rendering
		//glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Teal color
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Black color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Cube
		cubeShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gorgeousImg);
		//glBindTexture(GL_TEXTURE_2D, gorgeousImgs[0]);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, gorgeousImgs[1]);

		//glUniform1f(colorModF, 1.0f);
		//glUniform1f(visibleAmtF, visibility);
		glBindVertexArray(VAO);

		glm::mat4 view = glm::mat4(1.0f);
		//view = glm::translate(view, -1.0f * glm::vec3(0.0f, 0.0f, 3.0f));
		//float camX = sin(time) * radius;
		//float camZ = cos(time) * radius;
		view = cam.GetViewMatrix();
		cubeShader.setUniformMat4f("view", view);
		//int viewLoc = glGetUniformLocation(shaders_res.shader_program, "view");
		//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection;
		float r_angle = 90.0f * std::abs(std::sin(time));
		projection = glm::perspective(glm::radians(cam.Zoom), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		cubeShader.setUniformMat4f("projection", projection);
		/*int projectionLoc = glGetUniformLocation(shaders_res.shader_program, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));*/

		glm::mat4 model;
		int modelLoc;
		int len = sizeof(cube_positions) / sizeof(cube_positions[0]);
		for (int i(0); i < len; ++i) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, cube_positions[i]);
			model = glm::rotate(model, time * glm::radians(-55.0f*(i + 1)), glm::vec3(1.0f * i, 0.5f*(i + 1), 0.25f*(i + 2)));
			cubeShader.setUniformMat4f("model", model);
			/*modelLoc = glGetUniformLocation(shaders_res.shader_program, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));*/
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// Draw light src
		lightSrcShader.use();
		lightSrcShader.setUniformMat4f("projection", projection);
		lightSrcShader.setUniformMat4f("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightSrcPos);
		model = glm::scale(model, glm::vec3(0.25f)); // a smaller cube
		lightSrcShader.setUniformMat4f("model", model);

		glBindVertexArray(lightSrcVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// last time
		lastTime = time;
	}


	// !!! Never forget this
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window, float * visibility)
{
	int altL = glfwGetKey(window, GLFW_KEY_LEFT_ALT);
	int altR = glfwGetKey(window, GLFW_KEY_RIGHT_ALT);
	int f4 = glfwGetKey(window, GLFW_KEY_F4);
	int upArrow = glfwGetKey(window, GLFW_KEY_UP);
	int dnArrow = glfwGetKey(window, GLFW_KEY_DOWN);
	int w = glfwGetKey(window, GLFW_KEY_W);
	int a = glfwGetKey(window, GLFW_KEY_A);
	int s = glfwGetKey(window, GLFW_KEY_S);
	int d = glfwGetKey(window, GLFW_KEY_D);
	int q = glfwGetKey(window, GLFW_KEY_Q);
	int e = glfwGetKey(window, GLFW_KEY_E);

	float camSpeed(2.5f);
	camSpeed *= deltaTime;
	if ((altL == GLFW_PRESS || altR == GLFW_PRESS) && f4 == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (upArrow == GLFW_PRESS) {
		//setVisibility(visibility, *visibility - 0.01f);
		++radius;
	}
	else if (dnArrow == GLFW_PRESS) {
		//setVisibility(visibility, *visibility + 0.01f);
		--radius;
	}
	if (w == GLFW_PRESS) {
		//cam.Translate(glm::vec3(0.0f, 0.0f, 1.0f), camSpeed);
		cam.Translate(cam.Forward * 1.0f, camSpeed);
		//cam.Translate(glm::vec3(cam.Forward.x, 0, cam.Forward.z) * 1.0f, camSpeed);	// Only on the ground
	}
	else if (s == GLFW_PRESS) {
		//cam.Translate(glm::vec3(0.0f, 0.0f, -1.0f), camSpeed);
		cam.Translate(cam.Forward * -1.0f, camSpeed);
	}
	if (a == GLFW_PRESS) {
		//cam.Translate(glm::vec3(-1.0f, 0.0f, 0.0f), camSpeed);
		cam.Translate(cam.Right * -1.0f, camSpeed);
	}
	else if (d == GLFW_PRESS) {
		//cam.Translate(glm::vec3(1.0f, 0.0f, 0.0f), camSpeed);
		cam.Translate(cam.Right * 1.0f, camSpeed);
	}
	if (q == GLFW_PRESS) {
		//cam.Translate(glm::vec3(0.0f, -1.0f, 0.0f), camSpeed);
		cam.Translate(cam.Up * -1.0f, camSpeed);
	}
	else if (e == GLFW_PRESS) {
		//cam.Translate(glm::vec3(0.0f, 1.0f, 0.0f), camSpeed);
		cam.Translate(cam.Up * 1.0f, camSpeed);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	currMousePosX = static_cast<float>(xpos);
	currMousePosY = static_cast<float>(ypos);

	if (firstEntered) {
		prevMousePosX = currMousePosX;
		prevMousePosY = currMousePosY;
		firstEntered = false;
	}

	float xoffset = currMousePosX - prevMousePosX;
	float yoffset = prevMousePosY - currMousePosY; 

	//std::printf("currMousePosY: %f, prevMousePosY: %f\n", currMousePosY, prevMousePosY);

	prevMousePosX = currMousePosX;
	prevMousePosY = currMousePosY;

	cam.UpdateAngle(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	cam.UpdateZoom(static_cast<float>(yoffset));
}

//void read_shader(char const *  shader_name, char * shader_text, size_t shader_text_max_len)
//{
//	// shader_text must be length of 0!
//	if (*shader_text != '\0') {
//		std::cout << "shader_text too long or doesn't have a terminator char. " << std::endl;
//		return;
//	}
//
//	if (shader_text_max_len < 1) {
//		std::cout << "shader_text_max_len must be >= 1! " << std::endl;
//		return;
//	}
//
//	std::ifstream ifstream;
//	ifstream.open(shader_name);
//
//	if (ifstream.is_open()) {
//		char line[256];
//		while (ifstream.getline(line, 256)) {
//			strcat_s(line, "\n");
//			if (strlen(line) + strlen(shader_text) <= shader_text_max_len) {
//				strcat_s(shader_text, shader_text_max_len, line);
//			}
//			else {
//				std::cout << "fuck, " << shader_text_max_len << " is too small!" << std::endl;
//				break;
//			}
//		}
//	}
//	else {
//		std::cout << "cannot open the shader" << std::endl;
//	}
//	//ifstream.close();// You don't have to cuz ifstream is RAII
//	//ifstream.clear();
//}
//
//struct init_shaders_res
//{
//	char const * err_str;
//	GLuint shader_program;
//	GLuint vertex_shader;
//	GLuint fragment_shader;
//	int return_code;
//	init_shaders_res() : err_str("not modified after initialization"), shader_program(0), vertex_shader(0), fragment_shader(0), return_code(-1) {};
//};
//
//init_shaders_res init_shaders(void)
//{
//	init_shaders_res res;
//	char vertexShaderSrc[2048]{ '\0' };
//	char fragShaderSrc[2048]{ '\0' };
//	read_shader("vts_lighting.glsl", vertexShaderSrc, 2048);
//	read_shader("frs_lighting.glsl", fragShaderSrc, 2048);
//	if (!strlen(vertexShaderSrc) || !strlen(fragShaderSrc)) {
//		res.err_str = "Fails in reading shaders";
//		return res;
//	}
//	else {
//		// Debug
//		for (int i(0); i < strlen(vertexShaderSrc); ++i) {
//			std::cout << vertexShaderSrc[i];
//		}
//		std::cout << std::endl << std::endl;
//		for (int i(0); i < strlen(fragShaderSrc); ++i) {
//			std::cout << fragShaderSrc[i];
//		}
//		std::cout << std::endl << std::endl;
//
//		// Create and compile the vertex shader
//		GLuint vertexShader;
//		vertexShader = glCreateShader(GL_VERTEX_SHADER);
//		char const * const p_v(vertexShaderSrc);
//		char const * const * const pp_v(&p_v);
//		glShaderSource(vertexShader, 1, pp_v, NULL);
//		glCompileShader(vertexShader);
//		int success;
//		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//		if (!success) {
//			char infoLog[512];
//			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//			std::cout << infoLog << std::endl;
//		}
//
//		// Create and compile the fragment shader
//		GLuint fragShader;
//		fragShader = glCreateShader(GL_FRAGMENT_SHADER);
//		char const * const p_f(fragShaderSrc);
//		char const * const * const pp_f(&p_f);
//		glShaderSource(fragShader, 1, pp_f, NULL);
//		glCompileShader(fragShader);
//		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
//		if (!success) {
//			char infoLog[512];
//			glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
//			std::cout << infoLog << std::endl;
//		}
//
//		// Create the shader program
//		res.shader_program = glCreateProgram();
//		glAttachShader(res.shader_program, vertexShader);
//		glAttachShader(res.shader_program, fragShader);
//		glLinkProgram(res.shader_program);
//		glGetProgramiv(res.shader_program, GL_LINK_STATUS, &success);
//		if (!success) {
//			char infoLog[512];
//			glGetProgramInfoLog(res.shader_program, 512, NULL, infoLog);
//			std::cout << infoLog << std::endl;
//		}
//
//		res.vertex_shader = vertexShader;
//		res.fragment_shader = fragShader;
//		res.err_str = "";
//		res.return_code = 0;
//		return res;
//	}
//}