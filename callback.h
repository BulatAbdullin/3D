#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#ifdef __cplusplus
extern "C" {
#endif


void error_callback(int error, const char* description);

void key_callback(
		GLFWwindow *window,
		int key, int scancode, int action, int mods);


#ifdef __cplusplus
}
#endif

#endif /* ifndef CALLBACK_HPP */
