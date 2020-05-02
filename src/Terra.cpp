#include <iostream>
#include <conio.h>
#include <vector>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "SDL/SDL.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Audio.h"

#include "Shader.h"

using namespace glm;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

GLFWwindow* window;

vec2 quad_vertices[] = { { 1.0f, 1.0f}, {1.0f,-1.0f}, {-1.0f,-1.0f}, {-1.0f,1.0f} };
vec2 quad_uv[] = { {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f} };
unsigned int quad_indices[] = { 0, 1, 3, 1, 2, 3 };

vec3 plane_vertices[] = { {-1.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f,-1.0f} };
unsigned int plane_indices[] = { 0, 1, 2, 3 };

Shader terrain_shader;
Shader noise_shader;

unsigned int terrain_length = 256;

unsigned int window_width = 1280;
unsigned int window_height = 720;

int main(int argc, char** argv) {

	if (!glfwInit()) {
		std::cerr << "GLFW failed to initialize!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(window_width, window_height, "Terra", nullptr, nullptr);
	if (!window) {
		std::cerr << "failed to create window!" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);

	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	terrain_shader.load("terrain.vert", "terrain.tesc", "terrain.tese", "terrain.frag");
	
	noise_shader.load("noise.vert", "noise.frag");

	GLuint terrain_VAO;
	glGenVertexArrays(1, &terrain_VAO);
	glBindVertexArray(terrain_VAO);

	GLuint terrain_VBO;
	glGenBuffers(1, &terrain_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	GLuint terrain_EBO;
	glGenBuffers(1, &terrain_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);

	GLuint quad_VAO;
	glGenVertexArrays(1, &quad_VAO);
	glBindVertexArray(quad_VAO);

	GLuint quad_VBO;
	glGenBuffers(1, &quad_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	GLuint quad_UV_VBO;
	glGenBuffers(1, &quad_UV_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, quad_UV_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_uv), quad_uv, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	GLuint quad_EBO;
	glGenBuffers(1, &quad_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	GLuint terrain_texture;
	glGenTextures(1, &terrain_texture);
	glBindTexture(GL_TEXTURE_2D, terrain_texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, terrain_length, terrain_length, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, terrain_texture, 0);

	Audio* music = new Audio();
	music->init();
	music->load("res/music.wav");

	music->play(2);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	while (!glfwWindowShouldClose(window)) {
		double time = glfwGetTime();
		processInput(window);
		if (music->isDone())
			music->restart();

		vec3 pos = vec3(2*cos(glfwGetTime()), 2, 2*sin(glfwGetTime()));
		mat4 model(1.0f);
		mat4 view = lookAt(pos, vec3(0.0f), vec3(0, 1, 0));
		mat4 proj = perspective(radians(90.0f), (float)window_width / window_height, 0.1f, 100.0f);

		mat4 MVP = proj * view * model;

		// GENERATE TERRAIN
		
		glViewport(0, 0, terrain_length, terrain_length);
		
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		noise_shader.use();

		glBindVertexArray(quad_VAO);
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		

		// DRAW TERRAIN

		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, window_width, window_height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

		terrain_shader.use();

		glUniformMatrix4fv(glGetUniformLocation(terrain_shader.getProgram(), "MVP"), 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(terrain_VAO);

		glBindTexture(GL_TEXTURE_2D, terrain_texture);

		glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	music->pause();

	delete music;

	glfwDestroyWindow(window);

	glfwTerminate();

	std::cout << "Press any key to continue. . ." << std::endl;
	_getch();
	return 0;
}