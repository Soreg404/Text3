#include "text.h"

#include <fstream>
#include <vector>
#include <glew/glew.h>

std::vector<char> readFile(const char *path) {
	std::fstream f(path, std::ios::in | std::ios::binary | std::ios::ate);
	if(!f.good()) return {0};
	size_t fs = f.tellg();
	f.clear();
	f.seekg(f.beg);

	std::vector<char> tmp(fs + 1);
	f.read(tmp.data(), fs);
	tmp[fs] = 0;

	return tmp;
}

void checkShdErr(GLuint id, bool shd = true) {
	int success;
	char infoLog[512];
	if(shd) glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	else glGetProgramiv(id, GL_LINK_STATUS, &success);
	if(!success) {
		if(shd) glGetShaderInfoLog(id, 512, NULL, infoLog);
		else glGetProgramInfoLog(id, 512, NULL, infoLog);
		LOG("compilation / link error: %s", infoLog);
	}
}

GLuint compileShd(GLenum t, const char *fp) {
	std::vector<char> src = readFile(fp);
	char *psrc = src.data();

	GLuint s = glCreateShader(t);
	glShaderSource(s, 1, &psrc, nullptr);
	glCompileShader(s);

	checkShdErr(s);
	return s;
}

void txt::Context::initGL() {

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nTexUnits);

	m_uGlyphs.resize(nTexUnits);

	// load shader
	shdId = glCreateProgram();
	GLuint vert = compileShd(GL_VERTEX_SHADER, "data/shd/shd.vert");
	GLuint frag = compileShd(GL_FRAGMENT_SHADER, "data/shd/shd.frag");
	glAttachShader(shdId, vert);
	glAttachShader(shdId, frag);
	glDeleteShader(vert);
	glDeleteShader(frag);
	glLinkProgram(shdId);
	checkShdErr(shdId, false);

	// load buffers
	float VBOvalues[] = {
		1, 1,
		-1, 1,
		-1, -1,
		1, -1
	};
	unsigned int EBOvalues[] = {
		0, 1, 2,
		0, 2, 3
	};
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(2, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VBOvalues), VBOvalues, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(EBOvalues), EBOvalues, GL_STATIC_DRAW);


}

unsigned int txt::Context::Font::loadTexture(unsigned int x, unsigned int y, void *buff) {
	unsigned int id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x, y, 0, GL_RED,GL_UNSIGNED_BYTE, buff);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return id;
}

void txt::Context::Font::unloadTexture(unsigned int *id) {
	glDeleteTextures(1, id);
}