#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <chrono>    
#include <algorithm>
#include "stb_image.h"

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
	res.window = glfwCreateWindow(1920, 1080, "Clarence's awesome game", NULL, NULL);
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

void processInput(GLFWwindow *window, float * visibility)
{
	int altL = glfwGetKey(window, GLFW_KEY_LEFT_ALT);
	int altR = glfwGetKey(window, GLFW_KEY_RIGHT_ALT);
	int f4 = glfwGetKey(window, GLFW_KEY_F4);
	int upArrow = glfwGetKey(window, GLFW_KEY_UP);
	int dnArrow = glfwGetKey(window, GLFW_KEY_DOWN);

	if ((altL == GLFW_PRESS || altR == GLFW_PRESS) && f4 == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (upArrow == GLFW_PRESS) {
		setVisibility(visibility, *visibility - 0.01f);
	}
	else if (dnArrow == GLFW_PRESS) {
		setVisibility(visibility, *visibility + 0.01f);
	}
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

	// Render stuff
	//GLfloat vertices[]{
	//		// positions	// colors
	//		-.3f, .3f, 0,	1.0f, 0.0f, 0.0f,
	//		-.6f, .9f, 0,	0.0f, 1.0f, 0.0f,
	//		-.6f, -.3f, 0,	0.0f, 0.0f, 1.0f,
	//		0, -.3f, 0,		1.0f, 0.0f, 1.0f,
	//		.6f, -.3f, 0,	1.0f, 1.0f, 0.0f,
	//		.3f, .3f, 0,	0.0f, 1.0f, 1.0f,
	//};
	//GLuint indices[]{
	//	1, 2, 3,
	//	3, 4, 5,
	//	0, 3, 5,
	//};
	GLfloat vertices[]{
		// positions		// colors				// texs
		.3f, .3f, 0,		0.0f, 0.0f, 1.0f,		2.0f, 2.0f,
		.3f, -.3f, 0,		1.0f, 0.0f, 0.0f,		2.0f, 0.0f,
		-.3f, -.3f, 0,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-.3f, .3f, 0,		0.0f, 0.0f, 1.0f,		0.0f, 2.0f,
	};
	GLuint indices[]{
		0, 1, 2,
		0, 2, 3,
	};

	
	// Create texture
	GLuint gorgeousImgs[2];
	glGenTextures(2, gorgeousImgs);
	int width, height, nrChannels;
	unsigned char *img_data(NULL);
	// Read texture
	// Texture #1
	glBindTexture(GL_TEXTURE_2D, gorgeousImgs[0]);
	// Set options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	img_data = stbi_load("ping.png", &width, &height, &nrChannels, 0);
	printf("first imge: width: %d, height: %d, nrChannels: %d", width, height, nrChannels);
	if (img_data == NULL) {
		std::cout << "Fuck. Can't load first img." << std::endl;
		return -1;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(img_data);	// Free the memory of the texture read
	// Texture #2
	glBindTexture(GL_TEXTURE_2D, gorgeousImgs[1]);
	// Set options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	img_data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
	printf("second imge: width: %d, height: %d, nrChannels: %d", width, height, nrChannels);
	if (img_data == NULL) {
		std::cout << "Fuck. Can't load second img." << std::endl;
		return -1;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(img_data);	// Free the memory of the texture read
	
	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

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

	// Polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set fragment shader color
	GLuint colorModF = glGetUniformLocation(shaders_res.shader_program, "ColorModF");
	GLuint offsetF = glGetUniformLocation(shaders_res.shader_program, "offsetF");
	GLuint visibleAmtF = glGetUniformLocation(shaders_res.shader_program, "visibleAmtF");
	// Use the shader program
	glUseProgram(shaders_res.shader_program);

	// Set sampler uniforms (Must set uniforms AFTER using shader programs)
	glUniform1i(glGetUniformLocation(shaders_res.shader_program, "TexImg0"), 0);
	glUniform1i(glGetUniformLocation(shaders_res.shader_program, "TexImg1"), 1);

	auto t_start = std::chrono::high_resolution_clock::now();
	float visibility(.25f);
	// Render loop
	while (!glfwWindowShouldClose(window)) {

		// input
		processInput(window, &visibility);

		// rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gorgeousImgs[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gorgeousImgs[1]);

		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
		float x = sin(time * 3.0f);
		float y = sin(time * 2.0f);
		float z = sin(time * 1.0f);

		//glUniform1f(colorModF, x / 2.0f + 0.5f);
		//glUniform1f(offsetF, y / 4.0f + 0.25f);
		glUniform1f(colorModF, 1.0f);
		glUniform1f(offsetF, 0.0f);
		glUniform1f(visibleAmtF, visibility);
		glBindVertexArray(VAO);
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	// !!! Never forget this
	glfwTerminate();
	return 0;
}


