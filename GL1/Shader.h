#pragma once

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class Shader
{
public:
	int id;
	static int fucl;
	// ------------------------------------------------------------------------
	Shader(char const * vertex_shader_name, char const * frag_shader_name)
		:id(-1)
	{
		std::string vertexShaderSrcStr;
		std::ifstream ifstreamV;
		ifstreamV.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			ifstreamV.open(vertex_shader_name);
			std::stringstream sstream;
			sstream << ifstreamV.rdbuf();
			ifstreamV.close();
			vertexShaderSrcStr = sstream.str();
		}
		catch (std::ifstream::failure const & e) {
			std::cout << "Error::Shader::cannot read the shader \"" << vertex_shader_name << "\"." << std::endl;
			vertexShaderSrcStr = "ERROR";
		}

		std::string fragShaderSrcStr;
		std::ifstream ifstreamF;
		ifstreamF.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			ifstreamF.open(frag_shader_name);
			std::stringstream sstream;
			sstream << ifstreamF.rdbuf();
			ifstreamF.close();
			fragShaderSrcStr = sstream.str();
		}
		catch (std::ifstream::failure const & e) {
			std::cout << "Error::Shader::cannot read the shader \"" << frag_shader_name << "\"." << std::endl;
			fragShaderSrcStr = "ERROR";
		}

		char const * vertexShaderSrc = vertexShaderSrcStr.c_str();
		char const * fragShaderSrc = fragShaderSrcStr.c_str();

		/*char const * vertexShaderSrc = read_shader(vertex_shader_name).c_str();
		char const * fragShaderSrc = read_shader(frag_shader_name).c_str();*/
		if (vertexShaderSrc == "ERROR" ||
			fragShaderSrc == "ERROR") {
			id = -1;
			return;
		}

		// Create and compile the vertex shader
		GLuint vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		char const * const p_v(vertexShaderSrc);
		char const * const * const pp_v(&p_v);
		glShaderSource(vertexShader, 1, pp_v, NULL);
		glCompileShader(vertexShader);
		int success;
		std::cout << "\"" << vertex_shader_name << "\" " << std::endl;
		checkCompileErrors(vertexShader, "VERTEX");

		// Create and compile the fragment shader
		GLuint fragShader;
		fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		char const * const p_f(fragShaderSrc);
		char const * const * const pp_f(&p_f);
		glShaderSource(fragShader, 1, pp_f, NULL);
		glCompileShader(fragShader);
		std::cout << "\"" << frag_shader_name << "\" " << std::endl;
		checkCompileErrors(fragShader, "FRAGMENT");

		// Create the shader program
		id = glCreateProgram();
		glAttachShader(id, vertexShader);
		glAttachShader(id, fragShader);
		glLinkProgram(id);
		checkCompileErrors(id, "PROGRAM");
	
		// Delete the shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragShader);
	}
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(id);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setUniform1b(std::string const & name, bool value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setUniform1i(std::string const & name, int value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setUniform1f(std::string const & name, float value) const
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setUniform3f(std::string const & name, float value1, float value2, float value3) const
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), value1, value2, value3);
	}
	void setUniformVec3f(std::string const & name, glm::vec3 const & vec3) 
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), vec3.x, vec3.y, vec3.z);
	}
	void setUniformMat4f(std::string const & name, glm::mat4 const & mat4)
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4));
	}
private:
	std::string const & read_shader(char const *  shader_name)
	{
		static std::string code;
		std::ifstream ifstream;
		ifstream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			ifstream.open(shader_name);
			std::stringstream sstream;
			sstream << ifstream.rdbuf();
			ifstream.close();
			code = sstream.str();
		}
		catch (std::ifstream::failure const & e) {
			std::cout << "Error::Shader::cannot read the shader \"" << shader_name << "\"." << std::endl;
			code = "ERROR";
		}
		return code;	// TODO: code will become empty. Deal with this.
	}
	// ------------------------------------------------------------------------
	void checkCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				char infoLog[512];
				glGetShaderInfoLog(shader, 512, NULL, infoLog);
				std::cout << infoLog << std::endl;
			}
		}
		else {
			glGetProgramiv(id, GL_LINK_STATUS, &success);
			if (!success) {
				char infoLog[512];
				glGetProgramInfoLog(id, 512, NULL, infoLog);
				std::cout << infoLog << std::endl;
			}
		}
	}
};