#include <iostream>
#include <conio.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <utility>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "SDL/SDL.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Audio.h"

#include "Shader.h"

using namespace glm;

bool inc_freq = false;
bool dec_freq = false;

bool w_key = false;
bool s_key = false;
bool a_key = false;
bool d_key = false;

void press(int key_type, int key, int action, bool &isPressed) {
	if (key == key_type && action == GLFW_PRESS)
		isPressed = true;
	if (key == key_type && action == GLFW_RELEASE)
		isPressed = false;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	press(GLFW_KEY_UP, key, action, inc_freq);
	press(GLFW_KEY_DOWN, key, action, dec_freq);

	press(GLFW_KEY_W, key, action, w_key);
	press(GLFW_KEY_S, key, action, s_key);
	press(GLFW_KEY_A, key, action, a_key);
	press(GLFW_KEY_D, key, action, d_key);

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

vec3 plane_vertices[] = { {-1.0f, 0.0f, -1.0f}, {1.0f, 0.0f,-1.0f}, {-1.0f, 0.0f, 1.0f}, { 1.0f, 0.0f,1.0f} };
unsigned int plane_indices[] = { 0, 1, 2, 3 };

Shader terrain_shader;
Shader noise_shader;

unsigned int terrain_length = 4096;

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

	GLuint terrain_texture[2];
	glGenTextures(2, terrain_texture);
	glBindTexture(GL_TEXTURE_2D, terrain_texture[0]);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, terrain_length, terrain_length, 0, GL_RED, GL_FLOAT, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, terrain_texture[0], 0);

	glBindTexture(GL_TEXTURE_2D, terrain_texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, terrain_length, terrain_length, 0, GL_RED, GL_FLOAT, nullptr);

	GLuint gradient_texture;
	glGenTextures(1, &gradient_texture);
	glBindTexture(GL_TEXTURE_2D, gradient_texture);
	
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLuint gradient_length = 1024;
	vec2* gradients = new vec2[gradient_length];

	srand(time(nullptr));

	ivec2 seed = ivec2(rand(), rand());

	srand(seed.x);

#define PI 3.14159265359

	for (int i = 0; i < gradient_length; i++) {
		float angle = radians(float(rand()%36000)/100.0);
		gradients[i] = vec2(sin(angle), cos(angle));
	}

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, gradient_length, 0, GL_RG, GL_FLOAT, gradients);


	Audio* music = new Audio();
	music->init();
	music->load("res/music.wav");

	//music->play(2);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	GLfloat freq = 1.5;

	double dt = glfwGetTime();
	double time = glfwGetTime();

	vec2 position(0.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {
		dt = glfwGetTime() - time;
		time = glfwGetTime();
		processInput(window);
		if (music->isDone())
			music->restart();

		vec3 light_position = vec3(2 * sin(glfwGetTime()), 2, 2 * cos(glfwGetTime()));

		vec3 pos = vec3(0, 1.1, 0.1);
		mat4 model(1.0f);
		mat4 view = lookAt(pos, vec3(0.0f), vec3(0, 1, 0));
		mat4 proj = perspective(radians(90.0f), (float)window_width / window_height, 0.1f, 100.0f);

		mat4 MVP = proj * view * model;

		// GENERATE TERRAIN
		
		glViewport(0, 0, terrain_length, terrain_length);
		
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		
		
		noise_shader.use();

		if (inc_freq)
			freq *= 1.0 + dt;
		if (dec_freq)
			freq *= 1.0 - dt;
		if (w_key)
			position.y -= dt;
		if (s_key)
			position.y += dt;
		if (a_key)
			position.x -= dt;
		if (d_key)
			position.x += dt;

		glUniform2iv(glGetUniformLocation(noise_shader.getProgram(), "seed"), 0, &seed[0]);

		glBindVertexArray(quad_VAO);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < 6; i++) {

			swap(terrain_texture[0], terrain_texture[1]);

			glBindTexture(GL_TEXTURE_2D, terrain_texture[0]);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, terrain_texture[0], 0);

			glUniform1f(glGetUniformLocation(noise_shader.getProgram(), "freq"), pow(2, i));
			glUniform2fv(glGetUniformLocation(noise_shader.getProgram(), "offset"), 1, &position[0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, terrain_texture[1]);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D, gradient_texture);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			
			
		}

		// DRAW TERRAIN

		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, window_width, window_height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

		terrain_shader.use();

		glUniformMatrix4fv(glGetUniformLocation(terrain_shader.getProgram(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(glGetUniformLocation(terrain_shader.getProgram(), "light_position"), 1, &light_position[0]);
		glUniform3fv(glGetUniformLocation(terrain_shader.getProgram(), "view_position"), 1, &pos[0]);
		glUniform1f(glGetUniformLocation(terrain_shader.getProgram(), "res"), 128);

		glBindVertexArray(terrain_VAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrain_texture[0]);

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