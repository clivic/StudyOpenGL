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

// Constants
float const WINDOW_WIDTH(1920);
float const WINDOW_HEIGHT(1080);

// Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(100, 100, width - 200, height - 200);
}

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

	// Enable depth buffer
	glEnable(GL_DEPTH_TEST);

	res.err_str = "";
	res.return_code = 0;
	return res;
}

void read_shader(char const *  shader_name, char * shader_text, size_t shader_text_max_len)
{
	// shader_text must be length of 0!
	if (*shader_text != '\0') {
		std::cout << "shader_text too long or doesn't have a terminator char. " << std::endl;
		return;
	}

	if (shader_text_max_len < 1) {
		std::cout << "shader_text_max_len must be >= 1! " << std::endl;
		return;
	}

	std::ifstream ifstream;
	ifstream.open(shader_name);

	if (ifstream.is_open()) {
		char line[256];
		while (ifstream.getline(line, 256)) {
			strcat_s(line, "\n");
			if (strlen(line) + strlen(shader_text) <= shader_text_max_len) {
				strcat_s(shader_text, shader_text_max_len, line);
			}
			else {
				std::cout << "fuck, " << shader_text_max_len << " is too small!" << std::endl;
				break;
			}
		}
	}
	else {
		std::cout << "cannot open the shader" << std::endl;
	}
	//ifstream.close();// You don't have to cuz ifstream is RAII
	//ifstream.clear();
}

struct init_shaders_res
{
	char const * err_str;
	GLuint shader_program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	int return_code;
	init_shaders_res() : err_str("not modified after initialization"), shader_program(0), vertex_shader(0), fragment_shader(0), return_code(-1) {};
};

init_shaders_res init_shaders(void)
{
	init_shaders_res res;
	char vertexShaderSrc[2048]{ '\0' };
	char fragShaderSrc[2048]{ '\0' };
	read_shader("vertex_shader.glsl", vertexShaderSrc, 2048);
	read_shader("fragment_shader.glsl", fragShaderSrc, 2048);
	if (!strlen(vertexShaderSrc) || !strlen(fragShaderSrc)) {
		res.err_str = "Fails in reading shaders";
		return res;
	}
	else {
		// Debug
		for (int i(0); i < strlen(vertexShaderSrc); ++i) {
			std::cout << vertexShaderSrc[i];
		}
		std::cout << std::endl << std::endl;
		for (int i(0); i < strlen(fragShaderSrc); ++i) {
			std::cout << fragShaderSrc[i];
		}
		std::cout << std::endl << std::endl;

		// Create and compile the vertex shader
		GLuint vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		char const * const p_v(vertexShaderSrc);
		char const * const * const pp_v(&p_v);
		glShaderSource(vertexShader, 1, pp_v, NULL);
		glCompileShader(vertexShader);
		int success;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << infoLog << std::endl;
		}

		// Create and compile the fragment shader
		GLuint fragShader;
		fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		char const * const p_f(fragShaderSrc);
		char const * const * const pp_f(&p_f);
		glShaderSource(fragShader, 1, pp_f, NULL);
		glCompileShader(fragShader);
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
			std::cout << infoLog << std::endl;
		}

		// Create the shader program
		res.shader_program = glCreateProgram();
		glAttachShader(res.shader_program, vertexShader);
		glAttachShader(res.shader_program, fragShader);
		glLinkProgram(res.shader_program);
		glGetProgramiv(res.shader_program, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(res.shader_program, 512, NULL, infoLog);
			std::cout << infoLog << std::endl;
		}

		res.vertex_shader = vertexShader;
		res.fragment_shader = fragShader;
		res.err_str = "";
		res.return_code = 0;
		return res;
	}
}

void setVisibility(float* visibility, float newAmt)
{
	if (visibility == NULL)return;
	float lo(0), hi(1);
	*visibility = (newAmt < lo) ? lo : (hi < newAmt) ? hi : newAmt;
	std::cout << "visibility: " << *visibility << std::endl;
}

void processInput(GLFWwindow *window, float * visibility, glm::vec3 * cam_pos)
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

	if ((altL == GLFW_PRESS || altR == GLFW_PRESS) && f4 == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (upArrow == GLFW_PRESS) {
		setVisibility(visibility, *visibility - 0.01f);
	}
	else if (dnArrow == GLFW_PRESS) {
		setVisibility(visibility, *visibility + 0.01f);
	}
	if (w == GLFW_PRESS) {
		cam_pos->y += 0.1f;
	}
	else if (s == GLFW_PRESS) {
		cam_pos->y -= 0.1f;
	}
	else if (a == GLFW_PRESS) {
		cam_pos->x -= 0.1f;
	}
	else if (d == GLFW_PRESS) {
		cam_pos->x += 0.1f;
	}
}

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
	init_shaders_res shaders_res = init_shaders();
	if (shaders_res.return_code) {
		std::cout << shaders_res.err_str << std::endl;
		return -1;
	}
	glDeleteShader(shaders_res.vertex_shader);
	glDeleteShader(shaders_res.fragment_shader);

	GLfloat vertices[]{
		// positions			// colors				// texs
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
	GLuint gorgeousImgs[2];
	glGenTextures(2, gorgeousImgs);
	// Read texture
	createTexture("ping.png", gorgeousImgs[0]);
	createTexture("awesomeface.png", gorgeousImgs[1]);

	// Set Virtual Array Object
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);	// Any subsequent VBO calls will be stored in the current VAO bound
	GLuint VBO;
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

	// Transform using matrix
	glm::mat4 trans = glm::mat4(1.0f);

	// Uniforms
	GLuint colorModF = glGetUniformLocation(shaders_res.shader_program, "ColorModF");
	GLuint transformLoc = glGetUniformLocation(shaders_res.shader_program, "transform");
	GLuint visibleAmtF = glGetUniformLocation(shaders_res.shader_program, "visibleAmtF");
	// Use the shader program (Have to use the program before setting the uniforms)
	glUseProgram(shaders_res.shader_program);

	// Set sampler uniforms (Must set uniforms AFTER using shader programs)
	glUniform1i(glGetUniformLocation(shaders_res.shader_program, "TexImg0"), 0);
	glUniform1i(glGetUniformLocation(shaders_res.shader_program, "TexImg1"), 1);

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

	auto t_start = std::chrono::high_resolution_clock::now();
	float visibility(.25f);
	glm::vec3 cam_pos(0,0,3.0f);
	// Render loop
	while (!glfwWindowShouldClose(window)) {

		// input
		processInput(window, &visibility, &cam_pos);

		// rendering
		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
		float x = sin(time * 3.0f);
		float y = sin(time * 2.0f);
		float z = sin(time * 1.0f);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gorgeousImgs[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gorgeousImgs[1]);

		glUniform1f(colorModF, 1.0f);
		glUniform1f(visibleAmtF, visibility);
		glBindVertexArray(VAO);

		//1
		/*trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0));
		trans = glm::rotate(trans, time, glm::vec3(0.0, 0.0, 1.0));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/

		//2
		//trans = glm::mat4(1.0f); // reset it to identity matrix
		//trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));
		//float scaleAmount = std::abs(cos(time));
		//trans = glm::scale(trans, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
		//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, -1.0f * cam_pos);
		int viewLoc = glGetUniformLocation(shaders_res.shader_program, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection;
		float r_angle = 90.0f * std::abs(std::sin(time));
		projection = glm::perspective(glm::radians(45.0f), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		int projectionLoc = glGetUniformLocation(shaders_res.shader_program, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model;
		int modelLoc;
		int len = sizeof(cube_positions) / sizeof(cube_positions[0]);
		for (int i(0); i < len; ++i) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, cube_positions[i]);
			model = glm::rotate(model, time * glm::radians(-55.0f*(i+1)), glm::vec3(1.0f * i, 0.5f*(i+1), 0.25f*(i+2)));
			modelLoc = glGetUniformLocation(shaders_res.shader_program, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	// !!! Never forget this
	glfwTerminate();
	return 0;
}


