#include <iostream>
#include <conio.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

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

int main() {

	if (!glfwInit()) {
		std::cerr << "GLFW failed to initialize!" << std::endl;
		goto exit;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 720, "Terra", nullptr, nullptr);
	if (!window) {
		std::cerr << "failed to create window!" << std::endl;
		goto exit;
	}

	glfwSetKeyCallback(window, key_callback);

	glfwSwapInterval(1);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		goto exit;
	}

	while (!glfwWindowShouldClose(window)) {
		double time = glfwGetTime();
		processInput(window);

		glClearColor(1.0f, 0.5f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();

exit:
	std::cout << "Press any key to continue. . ." << std::endl;
	_getch();
	return 0;
}