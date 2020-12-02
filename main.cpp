#include <GLFW/glfw3.h>
#include <stdlib.h>

#include "callback.h"

int main(int argc, char *argv[])
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(1);
	}
	// OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow *window = glfwCreateWindow(640, 480, "GLFW", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(window);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glfwSetKeyCallback(window, key_callback);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwWaitEvents();
	}
	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
