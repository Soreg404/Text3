#include "text.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library tlib;
size_t txt::Context::nContexts = 0;


txt::Context::Context(int w, int h) {

	if(!nContexts) {
		FT_Init_FreeType(&tlib);
	}
	nContexts++;

	if(w && h) setProjectionSize(w, h);

	// load shader

	// load VAO, VBO

}

txt::Context::~Context() {
	nContexts--;
	if(!nContexts) {
		FT_Done_FreeType(tlib);
		tlib = nullptr;
	}

	// unload shader

	// unload VAO, VBO

}


void txt::Context::setProjectionSize(int width, int height) {
	/*GLint loc = glGetUniformLocation(shd, "projection");
	if(loc != -1) {
		glm::mat4 proj = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), .1f, 100.f);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
	} else LOG("couldn't set window size in shader");*/
}
