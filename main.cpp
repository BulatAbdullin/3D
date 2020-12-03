#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <stdlib.h>

#include <stdio.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define INFO_LOG_SIZE 512

static void errorCallback(int error, const char* description);

static void keyCallback(
		GLFWwindow *window,
		int key, int scancode, int action, int mods);

static const char *read_file(const char *filepath)
{
	FILE *f = fopen(filepath, "r");
	if (!f) {
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	long filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = new char[filesize];
	fread(buf, sizeof(char), filesize, f);
	fclose(f);
	return buf;
}

int main(int argc, char *argv[])
{
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) {
		exit(1);
	}
	// OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(
			WINDOW_WIDTH, WINDOW_HEIGHT, "GLFW", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(window);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetKeyCallback(window, keyCallback);

	const char *vertex_shader_source = read_file("src.vert");
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	GLint compilation_success;
	GLchar info_log[INFO_LOG_SIZE];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compilation_success);
	if (!compilation_success) {
		glGetShaderInfoLog(vertex_shader, sizeof(info_log), NULL, info_log);
		fprintf(stderr, info_log);
	}
	delete[] vertex_shader_source;

	const char *fragment_shader_source = read_file("src.frag");
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCreateShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compilation_success);
	if (!compilation_success) {
		glGetShaderInfoLog(fragment_shader, sizeof(info_log), NULL, info_log);
		fprintf(stderr, info_log);
	}
	delete[] fragment_shader_source;

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	//glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &compilation_success);
	if (!compilation_success) {
		glGetProgramInfoLog(shader_program, sizeof(info_log), NULL, info_log);
		fprintf(stderr, info_log);
	}
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	GLfloat vertices[] = {
		-0.5f,  0.0f, 0.0f,
		 0.5f,  0.0f, 0.0f,
		 0.0f, -0.5f, 0.0f
	};

	GLuint vao; // Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo; // Vertex Buffer Object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
			GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(
			0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *) 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // unbind the VAO
	while (!glfwWindowShouldClose(window)) {
        glClearColor(0.5f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader_program);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(
		GLFWwindow *window,
		int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}
