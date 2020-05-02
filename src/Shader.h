#pragma once

#include "GLAD/glad.h"

#include <string>
#include <vector>

using namespace std;

class Shader{
	GLuint program;
public:

	int load(string vert_path, string frag_path);
	int load(string vert_path, string tesc_path, string tese_path, string frag_path);

	void use();

	GLuint getProgram();

private:
	string readShader(string shader_path);
	GLuint createShader(GLenum shader_type, string shader_src);
	GLuint createProgram(vector<GLuint> shaders);

};

