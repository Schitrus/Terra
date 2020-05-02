#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

int Shader::load(string vert_path, string frag_path) {
	string vert_src, frag_src;
	GLuint vertex = -1;
	GLuint fragment = -1;
	try {
		vert_src = readShader(vert_path);
		vertex = createShader(GL_VERTEX_SHADER, vert_src);

		frag_src = readShader(frag_path);
		fragment = createShader(GL_FRAGMENT_SHADER, frag_src);

		program = createProgram({ vertex, fragment });
	}
	catch (string error) {
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		cerr << "Error in creating shader program " << vert_path << ", " << frag_path << endl;
		cerr << error;
		return -1;
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return 0;
}

int Shader::load(string vert_path, string tesc_path, string tese_path, string frag_path) {
	string vert_src, tesc_src, tese_src, frag_src;
	GLuint vertex   = -1;
	GLuint tessctrl = -1;
	GLuint tesseval = -1;
	GLuint fragment = -1;
	try {
		vert_src = readShader(vert_path);
		vertex = createShader(GL_VERTEX_SHADER, vert_src);

		tesc_src = readShader(tesc_path);
		tessctrl = createShader(GL_TESS_CONTROL_SHADER, tesc_src);

		tese_src = readShader(tese_path);
		tesseval = createShader(GL_TESS_EVALUATION_SHADER, tese_src);

		frag_src = readShader(frag_path);
		fragment = createShader(GL_FRAGMENT_SHADER, frag_src);

		program = createProgram({ vertex, tessctrl, tesseval, fragment });
	}
	catch (string error) {
		glDeleteShader(vertex);
		glDeleteShader(tessctrl);
		glDeleteShader(tesseval);
		glDeleteShader(fragment);
		cerr << "Error in creating shader program " << vert_path << ", " << frag_path << endl;
		cerr << error;
		return -1;
	}
	glDeleteShader(vertex);
	glDeleteShader(tessctrl);
	glDeleteShader(tesseval);
	glDeleteShader(fragment);

	return 0;
}

void Shader::use() {
	glUseProgram(program);
}

GLuint Shader::getProgram() {
	return program;
}

string Shader::readShader(string shader_path) {
	
	ifstream file("shaders/" + shader_path);

	if (!file.is_open()) {
		cerr << "Failed to open " << shader_path << endl;
	}

	stringstream shader_stream;

	string shader_line;
	
	while (getline(file, shader_line)) {
		shader_stream << shader_line << endl;
	}

	file.close();

	return shader_stream.str();
}

GLuint Shader::createShader(GLenum shader_type, string shader_src) {
	GLuint shader;
	shader = glCreateShader(shader_type);
	if (!shader)
		throw "Failed to create shader";
	const char* src = shader_src.c_str();
	glShaderSource(shader, 1, &src, nullptr);

	glCompileShader(shader);

	GLint compiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLogLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
		GLchar* infoLog = new GLchar[infoLogLen];
		glGetShaderInfoLog(shader, infoLogLen, nullptr, infoLog);
		throw string(infoLog);
	}
	return shader;
}

GLuint Shader::createProgram(vector<GLuint> shaders) {
	GLuint program;
	program = glCreateProgram();
	if (!program)
		throw "Failed to create program";

	for(GLuint shader : shaders)
		glAttachShader(program, shader);

	glLinkProgram(program);

	GLint linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint infoLogLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
		GLchar* infoLog = new GLchar[infoLogLen];
		glGetProgramInfoLog(program, infoLogLen, nullptr, infoLog);
		throw string(infoLog);
	}
	return program;
}