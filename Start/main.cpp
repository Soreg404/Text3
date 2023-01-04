#include <text.h>

#include <iostream>
#include <glew/glew.h>
#include <glfw/glfw3.h>

int WWIDTH = 800, WHEIGHT = 600;

using std::cout;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	fprintf(stderr, "GL CALLBACK [%s]: type: 0x%x, severity: 0x%x, message: %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "ERROR" : "info"),
			type, severity, message);
}

int main() {

	if(glfwInit() != GLFW_TRUE) {
		cout << "error initializing GLFW\n";
		return -1;
	}

	GLFWwindow *wnd = glfwCreateWindow(WWIDTH, WHEIGHT, "context", nullptr, nullptr);
	glfwMakeContextCurrent(wnd);
	glViewport(0, 0, WWIDTH, WHEIGHT);

	if(glewInit() != GLEW_OK) {
		cout << "error initializing GLEW\n";
		return -1;
	}

	const GLubyte *glVer = glGetString(GL_VERSION);
	cout << "gl version: " << glVer << "\n";

	glClearColor(.7f, .7f, .7f, 1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);


	/* === text context === */

	txt::Context ctx(WWIDTH, WHEIGHT);
	ctx.fontLoad("data/fonts/arial.ttf", "arial");



	/* === */

	while(!glfwWindowShouldClose(wnd)) {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(wnd);

		glfwWaitEvents();

	}

	glfwTerminate();

	return 0;
}