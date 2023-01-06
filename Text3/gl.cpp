#include "text.h"

#include <fstream>
#include <vector>
#include <glew/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void txt::GlyphInfo::loadTexture(const void *buff) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, x, y, 0, GL_RED, GL_UNSIGNED_BYTE, buff);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void txt::GlyphInfo::unloadTexture() {
	if(id) glDeleteTextures(1, &id);
}

void txt::Field::createBuffer(unsigned int CtxQVBO, unsigned int CtxEBO) {
	if(!m_VAO) {
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_posBufferId);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, CtxQVBO);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_posBufferId);
		glVertexAttribIPointer(1, 2, GL_INT, 0, nullptr);
		glEnableVertexAttribArray(1);
		glVertexAttribDivisor(1, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CtxEBO);
	} 

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_posBufferId);
	size_t bs = text.length() * sizeof(int) * 2;
	glBufferData(GL_ARRAY_BUFFER, bs, nullptr, GL_STATIC_DRAW);
}

void txt::Field::deleteBuffer() {
	if(m_VAO) {
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_posBufferId);
	}
}

void txt::Field::bufferSubData(const void *buff, int head, int size) {
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_posBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, head, size, buff);
}

void txt::Field::draw() {
	glBindVertexArray(m_VAO);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, 100);
}

void txt::Context::initGL() {

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_nTexUnits);

	m_uGlyphs.resize(m_nTexUnits);

	// load shader
	m_shdId = glCreateProgram();
	GLuint vert = compileShd(GL_VERTEX_SHADER, "data/shd/shd.vert");
	GLuint frag = compileShd(GL_FRAGMENT_SHADER, "data/shd/shd.frag");
	glAttachShader(m_shdId, vert);
	glAttachShader(m_shdId, frag);
	glLinkProgram(m_shdId);
	glDeleteShader(vert);
	glDeleteShader(frag);
	checkShdErr(m_shdId, false);

	// load buffers
	float VBOvalues[] = {
		100, 100,
		50, 100,
		50, 50,
		100, 50
	};
	unsigned int EBOvalues[] = {
		0, 1, 2,
		0, 2, 3
	};

	glBindVertexArray(0);
	glGenBuffers(2, &m_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VBOvalues), VBOvalues, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(EBOvalues), EBOvalues, GL_STATIC_DRAW);

}

void txt::Context::setCtxSize(int width, int height) {
	GLint loc = glGetUniformLocation(m_shdId, "uProj");
	if(loc != -1) {
		glUseProgram(m_shdId);
		glm::mat4 proj = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), 1.f, 100.f);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
	} else LOG("couldn't set window size in shader");
}

void txt::Context::prepForDrawing() {
	glUseProgram(m_shdId);
}
