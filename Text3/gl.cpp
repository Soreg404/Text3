#include "text.h"

#include <fstream>
#include <vector>
#include <glew/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

inline void print_gl_errors(const char *func, int line) {
	while(int e = glGetError()) printf("<GL ERROR> [%s] %i: %X\n", func, line, e);
}
#define GLERR print_gl_errors(__FUNCTION__, __LINE__);

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
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, buff);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void txt::GlyphInfo::unloadTexture() {
	if(id) glDeleteTextures(1, &id);
}

void txt::Field::createBuffer(unsigned int CtxEBO) {
	if(!m_VAO) {
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_posBufferId);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_posBufferId);
		glVertexAttribIPointer(0, 2, GL_INT, 0, nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribDivisor(0, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CtxEBO);
	} 

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_posBufferId);
	size_t bs = text.length() * sizeof(int) * 2;
	glBufferData(GL_ARRAY_BUFFER, bs, nullptr, GL_STATIC_DRAW);
}

void txt::Field::bufferSubData(const void *buff, int head, int size) {
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_posBufferId);
	glBufferSubData(GL_ARRAY_BUFFER, head, size, buff);
}

void txt::Field::cleanup() {
	if(m_VAO) {
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_posBufferId);
	}
}

void txt::Field::draw() {
	glBindVertexArray(m_VAO);
	//glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, text.length());
	glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, text.length(), 0, 0);
}

void txt::Context::initGL() {

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_nTexUnits);

	m_uGlyphs.resize(m_nTexUnits);

	// load shader
	m_shd = glCreateProgram();
	GLuint vert = compileShd(GL_VERTEX_SHADER, "data/shd/shd.vert");
	GLuint frag = compileShd(GL_FRAGMENT_SHADER, "data/shd/shd.frag");
	glAttachShader(m_shd, vert);
	glAttachShader(m_shd, frag);
	glLinkProgram(m_shd);
	glDeleteShader(vert);
	glDeleteShader(frag);
	checkShdErr(m_shd, false);

	// init sampler
	glUseProgram(m_shd);
	GLint loc = glGetUniformLocation(m_shd, "glyphTextures");
	for(int i = 0; i < 32; i++) glUniform1i(loc + i, i);

	GLERR;

	// load ebo

	glBindVertexArray(0);
	glGenBuffers(1, &m_EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	unsigned int EBOvalues[] = {
		0, 1, 2,
		0, 2, 3
	};
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(EBOvalues), EBOvalues, GL_STATIC_DRAW);

	GLERR;

}

void txt::Context::setCtxSize(int width, int height) {
	GLint loc = glGetUniformLocation(m_shd, "uProj");
	if(loc != -1) {
		glUseProgram(m_shd);
		glm::mat4 proj = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), 1.f, 100.f);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
	} else LOG("couldn't set window size in shader");
}

void txt::Context::prepForDrawing(Field *field) {
	glUseProgram(m_shd);
	int i = 0;
	unsigned int counter = 0;
	char sprBuff[60]{0};
	for(auto &ui : field->lastCharsUsage) {
		const GlyphInfo *gi = fontGetGlyph(field->lastFonts[0].c_str(), 0, ui.first);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gi->id);
		counter += ui.second.count;

		int stc = sprintf_s(sprBuff, 30, "glyphs[%i].", i);

		strcpy_s(sprBuff + stc, 30, "lastInstanceID");
		GLint uGlyphIns = glGetUniformLocation(m_shd, sprBuff);

		strcpy_s(sprBuff + stc, 30, "x");
		GLint uGlyphX = glGetUniformLocation(m_shd, sprBuff);
		strcpy_s(sprBuff + stc, 30, "y");
		GLint uGlyphY = glGetUniformLocation(m_shd, sprBuff);
		strcpy_s(sprBuff + stc, 30, "w");
		GLint uGlyphW = glGetUniformLocation(m_shd, sprBuff);
		strcpy_s(sprBuff + stc, 30, "h");
		GLint uGlyphH = glGetUniformLocation(m_shd, sprBuff);
		glUniform1ui(uGlyphIns, counter);
		glUniform1i(uGlyphX, gi->x);
		glUniform1i(uGlyphY, gi->y);
		glUniform1i(uGlyphW, gi->w);
		glUniform1i(uGlyphH, gi->h);
		i++;
	}

	glUniform1i(glGetUniformLocation(m_shd, "uCount"), field->text.length());

	GLERR
}
