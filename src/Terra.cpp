#include <iostream>
#include <conio.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "portaudio/portaudio.h" 

#define SAMPLE_RATE (44100)


typedef struct
{
	float left_phase;
	float right_phase;
}
paTestData;

static paTestData data;

static int audio_callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData) {
	/* Cast data passed through stream to our structure. */
	paTestData* data = (paTestData*)userData;
	float* out = (float*)outputBuffer;
	unsigned int i;
	(void)inputBuffer; /* Prevent unused variable warning. */

	for (i = 0; i < framesPerBuffer; i++)
	{
		*out++ = data->left_phase;  /* left */
		*out++ = data->right_phase;  /* right */
		/* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
		data->left_phase += 0.01f;
		/* When signal reaches top, drop back down. */
		if (data->left_phase >= 1.0f) data->left_phase -= 2.0f;
		/* higher pitch so we can distinguish left and right. */
		data->right_phase += 0.03f;
		if (data->right_phase >= 1.0f) data->right_phase -= 2.0f;
	}
	return 0;
}

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

	if (Pa_Initialize() != paNoError) { 
		std::cerr << "Failed to initialize PortAudio" << std::endl;
		goto exit; 
	}

	PaStream* stream;
	PaError err;

	/* Open an audio I/O stream. */
	err = Pa_OpenDefaultStream(&stream,
		0,          /* no input channels */
		2,          /* stereo output */
		paFloat32,  /* 32 bit floating point output */
		SAMPLE_RATE,
		256,        /* frames per buffer, i.e. the number
						   of sample frames that PortAudio will
						   request from the callback. Many apps
						   may want to use
						   paFramesPerBufferUnspecified, which
						   tells PortAudio to pick the best,
						   possibly changing, buffer size.*/
		audio_callback, /* this is your callback function */
		&data); /*This is a pointer that will be passed to
						   your callback*/
	if (err != paNoError) goto exit;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto exit;

	while (!glfwWindowShouldClose(window)) {
		double time = glfwGetTime();
		processInput(window);

		glClearColor(1.0f, 0.5f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	err = Pa_StopStream(stream);
	if (err != paNoError) goto exit;

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto exit;

	Pa_Terminate();

	glfwDestroyWindow(window);

	glfwTerminate();

exit:
	std::cout << "Press any key to continue. . ." << std::endl;
	_getch();
	return 0;
}