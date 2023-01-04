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

	initGL();

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

void txt::Context::fontLoad(const char *path, const char *name) {
	if(m_fonts.find(name) != m_fonts.end()) {
		LOG("font [%s] already loaded", name);
		return;
	}
	FT_Face *ft = reinterpret_cast<FT_Face *>(&m_fonts[name].h_ft);

	FT_Error error = FT_New_Face(tlib, path, 0, ft);
	if(error) {
		if(error == FT_Err_Unknown_File_Format) 
			LOG("font [%s] unknown file format", name);
		else
			LOG("font [%s] other error", name);
		return;
	}
	error = FT_Set_Pixel_Sizes(*ft, 0, 16);
	LOG("loaded font [%s]", name);
}


void txt::Context::setProjectionSize(int width, int height) {
	/*GLint loc = glGetUniformLocation(shd, "projection");
	if(loc != -1) {
		glm::mat4 proj = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), .1f, 100.f);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
	} else LOG("couldn't set window size in shader");*/
}


void txt::Context::Font::add(char32_t c, int size, unsigned int count) {
	FT_Face *ft = reinterpret_cast<FT_Face *>(&h_ft);
	FT_Error error = FT_Load_Char(*ft, c, FT_LOAD_RENDER);
	if(error) {
		LOG("error loading glyph [%c]", c);
		return;
	}
	FT_GlyphSlot slot = (*ft)->glyph;
	glyphs[c].uses += count;
	glyphs[c].id = loadTexture(slot->bitmap.rows, slot->bitmap.width, slot->bitmap.buffer);
}
void txt::Context::Font::sub(char32_t c, int size, unsigned int count) {
	auto f = glyphs.find(c);
	if(f == glyphs.end()) return;
	if(f->second.uses <= count) {
		unloadTexture(&f->second.id);
		glyphs.erase(f);
	} else f->second.uses -= count;
}

void txt::Context::fieldLoad(txt::Field *field) {
	if(!field->fonts.size()) {
		LOG("no fonts selected");
		return;
	}

	std::map<char32_t, Field::CharInfo> currCharsUsage;
	for(auto c : field->text) currCharsUsage[c].count++;

	const char *fname = field->fonts[0].c_str();
	if(m_fonts.find(fname) == m_fonts.end()) {
		LOG("font [%] not loaded", fname);
		return;
	}
	for(auto &ui : currCharsUsage) m_fonts[fname].add(ui.first, 0, ui.second.count);
	for(auto &lui : field->lastCharsUsage) m_fonts[field->lastFonts[0]].sub(lui.first, 0, lui.second.count);

	field->lastCharsUsage = currCharsUsage;
	field->lastFonts = field->fonts;



}
