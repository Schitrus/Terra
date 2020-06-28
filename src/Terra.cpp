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

#include "Camera.h"

#define PI 3.14159265359

using namespace glm;

bool inc_freq = false;
bool dec_freq = false;

bool w_key = false;
bool s_key = false;
bool a_key = false;
bool d_key = false;

unsigned int window_width = 1280;
unsigned int window_height = 720;

void press(int key_type, int key, int action, bool &isPressed) {
	if (key == key_type && action == GLFW_PRESS)
		isPressed = true;
	if (key == key_type && action == GLFW_RELEASE)
		isPressed = false;
}

double cursorPositionY = 0.0f;
double cursorPositionX = 0.0f;

double dposy = 0.0f;
double dposx = 0.0f;

vec3 environment_color = vec3(0.5f, 0.7f, 1.0f);

static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	dposx += 0.0025*(xpos - cursorPositionX);
	dposy += 0.0025*(ypos - cursorPositionY);
	cursorPositionX = xpos;
	cursorPositionY = ypos;
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

vec3 plane_vertices[] = { {-1.0f, 0.0f,-1.0f}, {1.0f, 0.0f,-1.0f}, {-1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f} };
unsigned int plane_indices[] = { 0, 1, 2, 3 };

vec3 cube_vertices[] = { {-1.0f,-1.0f,-1.0f}, { 1.0f,-1.0f,-1.0f}, {-1.0f, 1.0f,-1.0f}, { 1.0f, 1.0f,-1.0f},
						 {-1.0f,-1.0f, 1.0f}, { 1.0f,-1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f, 1.0f} };
unsigned int cube_indices[] = { 1, 2, 3, 2, 1, 0,  4, 5, 6, 7, 6, 5,
                                1, 4, 5, 4, 1, 0,  2, 3, 6, 7, 6, 3,
                                2, 4, 6, 4, 2, 0,  1, 3, 5, 7, 5, 3};

Shader terrain_shader;
Shader noise_shader;
Shader shadow_shader;
Shader light_shader;

unsigned int terrain_length = 64;

unsigned int shadow_size = 1024*8;

vec2 grad(vec2* grads, ivec2 pos) {
	int p = (pos.x * pos.x + pos.y * pos.y) % 1024;
	return grads[p];
}

float fade(float x) {
	return x * x * x * (x * (x * 6 - 15) + 10);
}

float perlin(vec2* grads, vec2 UV, vec2 offset, ivec2 seed, float freq) {
	vec2 pos = freq * (UV + vec2(-0.5) + offset);
	vec2 in_pos = fract(abs(pos));
	in_pos.x = pos.x >= 0 ? in_pos.x : 1 - in_pos.x;
	in_pos.y = pos.y >= 0 ? in_pos.y : 1 - in_pos.y;
	ivec2 ex_pos = ivec2(floor(pos)) + seed;

	vec2 g0, g1, g2, g3;

	ivec2 dx = ivec2(1, 0);
	ivec2 dy = ivec2(0, 1);

	g0 = grad(grads, ex_pos);
	g1 = grad(grads, ex_pos + dx);
	g2 = grad(grads, ex_pos + dy);
	g3 = grad(grads, ex_pos + dx + dy);

	float d0, d1, d2, d3;

	d0 = dot(g0, in_pos);
	d1 = dot(g1, in_pos - vec2(dx));
	d2 = dot(g2, in_pos - vec2(dy));
	d3 = dot(g3, in_pos - vec2(dx) - vec2(dy));

	float x0, x1, y0;

	x0 = mix(d0, d1, fade(in_pos.x));
	x1 = mix(d2, d3, fade(in_pos.x));
	y0 = mix(x0, x1, fade(in_pos.y));

	return y0 / freq;
}

void generateTerrain(GLuint FBO, GLuint VAO, GLuint* terrain_texture, GLuint gradient_texture, vec2 offset, ivec2 seed) {
	// GENERATE TERRAIN

	glViewport(0, 0, terrain_length + 2, terrain_length + 2);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	noise_shader.use();

	glUniform2iv(glGetUniformLocation(noise_shader.getProgram(), "seed"), 1, &seed[0]);
	glUniform1f(glGetUniformLocation(noise_shader.getProgram(), "size"), terrain_length);

	glBindVertexArray(VAO);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < 6; i++) {

		swap(terrain_texture[0], terrain_texture[1]);

		glBindTexture(GL_TEXTURE_2D, terrain_texture[0]);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, terrain_texture[0], 0);

		glUniform1f(glGetUniformLocation(noise_shader.getProgram(), "freq"), pow(2, i));
		glUniform2fv(glGetUniformLocation(noise_shader.getProgram(), "offset"), 1, &offset[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrain_texture[1]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, gradient_texture);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	}
}

void drawShadow(GLuint FBO, GLuint shadow_FBO, GLuint quad_VAO, GLuint terrain_VAO, GLuint* terrain_texture, GLuint gradient_texture, GLuint shadow_texture, vec2 pospos, ivec2 seed, mat4 MVP, float res) {
	
	generateTerrain(FBO, quad_VAO, terrain_texture, gradient_texture, pospos, seed);

	//DRAW SHADOW

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, shadow_size, shadow_size);

	glBindFramebuffer(GL_FRAMEBUFFER, shadow_FBO);

	shadow_shader.use();

	glUniformMatrix4fv(glGetUniformLocation(shadow_shader.getProgram(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
	glUniform1f(glGetUniformLocation(shadow_shader.getProgram(), "res"), res);

	glBindVertexArray(terrain_VAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrain_texture[0]);

	glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void drawTerrain(GLuint FBO, GLuint quad_VAO, GLuint terrain_VAO, GLuint* terrain_texture, GLuint gradient_texture, GLuint shadow_texture, vec2 pospos, ivec2 seed, mat4 MVP, mat4 modelViewMatrix, mat4 lightMatrix, vec3 light_position, vec3 view_position, float res) {

	generateTerrain(FBO, quad_VAO, terrain_texture, gradient_texture, pospos, seed);

	// DRAW TERRAIN

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, window_width, window_height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	terrain_shader.use();

	glUniformMatrix4fv(glGetUniformLocation(terrain_shader.getProgram(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(terrain_shader.getProgram(), "modelViewMatrix"), 1, GL_FALSE, &modelViewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(terrain_shader.getProgram(), "lightMatrix"), 1, GL_FALSE, &lightMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(terrain_shader.getProgram(), "light_position"), 1, &light_position[0]);
	glUniform3fv(glGetUniformLocation(terrain_shader.getProgram(), "view_position"), 1, &view_position[0]);
	glUniform3fv(glGetUniformLocation(terrain_shader.getProgram(), "irradience_color"), 1, &environment_color[0]);
	glUniform1f(glGetUniformLocation(terrain_shader.getProgram(), "res"), res);

	glBindVertexArray(terrain_VAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrain_texture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadow_texture);

	glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT, 0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_POLYGON);

	glBindVertexArray(0);
}

void drawLight(GLuint VAO, mat4 MVP, vec3 light_color) {

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_EQUAL);

	glViewport(0, 0, window_width, window_height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	light_shader.use();

	glUniformMatrix4fv(glGetUniformLocation(light_shader.getProgram(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(glGetUniformLocation(light_shader.getProgram(), "light_color"), 1, &light_color[0]);

	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
}

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

	glfwSetCursorPosCallback(window, cursor_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwGetCursorPos(window, &cursorPositionX, &cursorPositionY);

	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	terrain_shader.load("terrain.vert", "terrain.tesc", "terrain.tese", "terrain.frag");
	
	noise_shader.load("noise.vert", "noise.frag");

	shadow_shader.load("terrain.vert", "terrain.tesc", "terrain.tese", "shadow.frag");

	light_shader.load("light.vert", "light.frag");

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

	GLuint cube_VAO;
	glGenVertexArrays(1, &cube_VAO);
	glBindVertexArray(cube_VAO);

	GLuint cube_VBO;
	glGenBuffers(1, &cube_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	GLuint cube_EBO;
	glGenBuffers(1, &cube_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

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
 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, terrain_length + 2, terrain_length + 2, 0, GL_RED, GL_FLOAT, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, terrain_texture[0], 0);

	glBindTexture(GL_TEXTURE_2D, terrain_texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, terrain_length + 2, terrain_length + 2, 0, GL_RED, GL_FLOAT, nullptr);

	// Shadow FrameBuffer

	GLuint depth_FBO;
	glGenFramebuffers(1, &depth_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_FBO);

	GLuint shadow_texture;
	glGenTextures(1, &shadow_texture);
	glBindTexture(GL_TEXTURE_2D, shadow_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadow_size, shadow_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////////////////////////

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

	ivec2 seed = ivec2(rand()%1000, rand()%1000);

	srand(seed.x);

	for (int i = 0; i < gradient_length; i++) {
		float angle = radians(float(rand()%36000)/100.0);
		gradients[i] = vec2(sin(angle), cos(angle));
	}

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RG32F, gradient_length, 0, GL_RG, GL_FLOAT, gradients);


	Audio* music = new Audio();
	music->init();
	music->load("res/music.wav");

	//music->play(2);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	GLfloat freq = 1.5;

	double dt = glfwGetTime();
	double time = glfwGetTime();

	vec3 position(0.0f, 0.0f, 0.0f);

	Camera camera;
	camera.setRatio(window_width, window_height);
	camera.move(vec3(0.0f, 1.0f, 0.0f));

	float tesselationResolution = terrain_length - 1;

	while (!glfwWindowShouldClose(window)) {
		dt = glfwGetTime() - time;
		time = glfwGetTime();
		processInput(window);
		if (music->isDone())
			music->restart();

		// Day cycle time in seconds
		float day_cycle_time = 10.0f;
		float season_cycle_time = day_cycle_time * 6 * 4;


		float world_longitude = 0.0f;
		float world_latitude = 0.0f;
		float world_tilt = 0.0f;

		mat4 light_dayRotation = rotate((float)radians(360.0f * glfwGetTime() / day_cycle_time + world_longitude), vec3(0.0f, 1.0f, 0.0f));

		mat4 light_seasonRotation = rotate((float)radians(90.0f + world_tilt * sin((float)radians(360.0f * glfwGetTime() / season_cycle_time))), vec3(1.0f, 0.0f, 0.0f));

		mat4 light_distance = translate(vec3(10000, 0, 0));

		mat4 light_transform = light_seasonRotation * light_dayRotation * light_distance;

		vec3 light_position = light_transform * vec4(0.0f, 0.0f, 0.0f, 1.0f);

		float cycle_time_inH = 24 * fract(glfwGetTime() / day_cycle_time + world_longitude + 0.25);

		std::cout << "Time: " << cycle_time_inH << endl;


		if (cycle_time_inH < 1)
			environment_color = vec3(0.1f, 0.1f, 0.25f);
		else if (cycle_time_inH < 4)
			environment_color = mix(vec3(0.1f, 0.1f, 0.25f), vec3(0.25f, 0.5f, 1.0f), (cycle_time_inH - 1) / 3);
		else if (cycle_time_inH < 6)
			environment_color = mix(vec3(0.25f, 0.5f, 1.0f), vec3(1.0f, 0.5f, 0.25f), (cycle_time_inH - 4) / 2);
		else if (cycle_time_inH < 8)
			environment_color = mix(vec3(1.0f, 0.5f, 0.25f), vec3(0.5f, 0.7f, 1.0f), (cycle_time_inH - 6) / 2);
		else if (cycle_time_inH < 11)
			environment_color = mix(vec3(0.5f, 0.7f, 1.0f), vec3(0.7f, 0.8f, 1.0f), (cycle_time_inH - 8) / 3);
		else if (cycle_time_inH < 13)
			environment_color = vec3(0.7f, 0.8f, 1.0f);
		else if (cycle_time_inH < 16)
			environment_color = mix(vec3(0.7f, 0.8f, 1.0f), vec3(0.5f, 0.7f, 1.0f), (cycle_time_inH - 13) / 3);
		else if (cycle_time_inH < 18)
			environment_color = mix(vec3(0.5f, 0.7f, 1.0f), vec3(1.0f, 0.5f, 0.25f), (cycle_time_inH - 16) / 2);
		else if (cycle_time_inH < 20)
			environment_color = mix(vec3(1.0f, 0.5f, 0.25f), vec3(0.25f, 0.5f, 1.0f), (cycle_time_inH - 18) / 2);
		else if (cycle_time_inH < 23)
			environment_color = mix(vec3(0.25f, 0.5f, 1.0f), vec3(0.1f, 0.1f, 0.25f), (cycle_time_inH - 20) / 3);
		else
			environment_color = vec3(0.1f, 0.1f, 0.25f);

		mat4 roty = rotate(mat4(1.0f), (float)-cursorPositionX, vec3(0.0f, 1.0f, 0.0f));

		mat4 rotx = rotate(mat4(1.0f), (float)-cursorPositionY, vec3(1.0f, 0.0f, 0.0f));

		camera.steer(-dposx);
		camera.tilt(-dposy);

		dposx = 0.0f;
		dposy = 0.0f;

		if (inc_freq)
			freq *= 1.0 + dt;
		if (dec_freq)
			freq *= 1.0 - dt;
		if (w_key)
			camera.move((float)dt * vec3(0.0f, 0.0f, -1.0f));
			//position += vec3(roty * (float)dt * 0.05f * vec4(0.0f, 0.0f,-1.0f, 0.0f));
		if (s_key)
			camera.move((float)dt * vec3(0.0f, 0.0f, 1.0f));
			//position += vec3(roty * (float)dt * 0.05f * vec4(0.0f, 0.0f, 1.0f, 0.0f));
		if (a_key)
			camera.move((float)dt * vec3(-1.0f, 0.0f, 0.0f));
			//position += vec3(roty * (float)dt * 0.05f * vec4(-1.0f, 0.0f, 0.0f, 0.0f));
		if (d_key)
			camera.move((float)dt * vec3(1.0f, 0.0f, 0.0f));
			//position += vec3(roty * (float)dt * 0.05f * vec4(1.0f, 0.0f, 0.0f, 0.0f));

		position = camera.getPosition();

		float height = 0.0;

		for (int i = 0; i < 6; i++) {
			height += perlin(gradients, vec2(0.5, 0.5), vec2(position.x, position.z), seed, pow(2, i));
		}

		vec2 pospos = floor(vec2(position.x, position.z) * ((float)tesselationResolution + 3));
		pospos /= (tesselationResolution + 3);

		vec3 pos = vec3(2 * (position.x - pospos.x), 0.1 + height, 2 * (position.z - pospos.y));

		vec3 view_dir = vec3(0.0f, 0.0f, -1.0f);

		view_dir = vec3(roty * rotx * vec4(view_dir, 0.0f));

		mat4 model(1.0f);
		mat4 view = camera.getView();//lookAt(pos, pos + view_dir, vec3(0, 1, 0));
		mat4 proj = camera.getProjection();//perspective(radians(90.0f), (float)window_width / window_height, 0.01f, 100.0f);

		mat4 light_view = lookAt(light_position, vec3(0.0f, 0.0f, 0.0f), vec3(0, 1, 0));
		mat4 light_proj = perspective(radians(0.1f), 1.0f, 10000.0f, 20000.0f);
		light_proj = ortho(-16.0f, 16.0f, -16.0f, 16.0f, 5000.0f, 20000.0f);
		

		glBindFramebuffer(GL_FRAMEBUFFER, depth_FBO);

		glViewport(0, 0, shadow_size, shadow_size);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(3.0f, 0.1f);
		
		// Draw Shadow
		for (int j = -5; j <= 5; j++) {
			for (int i = -5; i <= 5; i++) {

				model = translate(mat4(1.0f), vec3(j * -2.0f, 0.0f, i * -2.0f));

				vec2 terrainPosition = -vec2(j, i);

				mat4 MVP = light_proj * light_view * model;

				drawShadow(FBO, depth_FBO, quad_VAO, terrain_VAO, terrain_texture, gradient_texture, shadow_texture, terrainPosition, seed, MVP, tesselationResolution);

			}
		}
		
		glDisable(GL_POLYGON_OFFSET_FILL);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		

		glViewport(0, 0, window_width, window_height);

		glClearColor(environment_color.r, environment_color.g, environment_color.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw Terrain
		for (int j = -5; j <= 5; j++) {
			for (int i = -5; i <= 5; i++) {

				model = translate(mat4(1.0f), vec3(j * -2.0f, 0.0f, i * -2.0f));// *scale(mat4(1.0f), vec3(1.001f));

				vec2 terrainPosition = -vec2(j, i);

				mat4 MVP = proj * view * model;

				mat4 modelViewMatrix = view * model;

				mat4 lightMatrix = translate(mat4(1.0f), vec3(0.5f)) * scale(mat4(1.0f), vec3(0.5f)) * light_proj * light_view * inverse(view);

				drawTerrain(FBO, quad_VAO, terrain_VAO, terrain_texture, gradient_texture, shadow_texture, terrainPosition, seed, MVP, modelViewMatrix, lightMatrix, light_position, position, tesselationResolution);

			}
		}

		mat4 cube_proj = perspective(radians(90.0f), (float)window_width / window_height, 5000.0f, 50000.0f);

		drawLight(cube_VAO, cube_proj * view * light_transform * scale(vec3(1000.0f, 1000.0f, 1000.0f)), vec3(1.0f, 1.0f, 0.5f));

		//std::cout << "Hello: " << cursorPositionX << ", " << cursorPositionY << std::endl;

		

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