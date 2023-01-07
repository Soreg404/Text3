#include "text.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library tlib;
size_t txt::Context::nContexts = 0;

txt::GlyphInfo::~GlyphInfo() {
	unloadTexture();
}

txt::Field::Field() {
}

txt::Field::~Field() {
	cleanup();
}

txt::Context::Context(int w, int h) {

	if(!nContexts) {
		FT_Init_FreeType(&tlib);
	}
	nContexts++;

	initGL();

	if(w && h) setCtxSize(w, h);

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

void txt::Context::loadFont(const char *path, const char *name) {
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
	LOG("loaded font [%s]", name);
}

void txt::Context::unloadFont(const char *name) {
	auto font = m_fonts.find(name);
	if(font == m_fonts.end()) return;
	FT_Face *ft = reinterpret_cast<FT_Face *>(&(font->second.h_ft));
	FT_Done_Face(*ft);

}

const txt::GlyphInfo *txt::Context::fontGetGlyph(const char *name, int size, char32_t c) {
	auto ff= m_fonts.find(name);
	if(ff == m_fonts.end()) return nullptr;
	auto fc = ff->second.glyphs.find(c);
	if(fc == ff->second.glyphs.end()) return nullptr;
	return &fc->second;
}

void txt::Context::Font::add(char32_t c, int size, unsigned int count) {
	FT_Face *ft = reinterpret_cast<FT_Face *>(&h_ft);
	FT_Set_Pixel_Sizes(*ft, 0, 120);
	FT_Error error = FT_Load_Char(*ft, c, FT_LOAD_RENDER);
	if(error) {
		LOG("error loading glyph [%c]", c);
		return;
	}
	FT_GlyphSlot slot = (*ft)->glyph;
	GlyphInfo *gl = &glyphs[c];
	gl->uses += count;
	gl->w = slot->bitmap.width;
	gl->h = slot->bitmap.rows;
	gl->x = slot->bitmap_left;
	gl->y = slot->bitmap_top;
	gl->a = slot->advance.x;
	glyphs[c].loadTexture(slot->bitmap.buffer);
}
void txt::Context::Font::sub(char32_t c, int size, unsigned int count) {
	auto f = glyphs.find(c);
	if(f == glyphs.end()) return;
	if(f->second.uses <= count) {
		glyphs.erase(f);
	} else f->second.uses -= count;
}

void txt::Context::loadField(txt::Field *field) {
	if(!field || !field->fonts.size()) {
		LOG("no fonts selected");
		return;
	}

	{
		std::map<char32_t, Field::CharInfo> currCharsUsage;
		for(unsigned char c : field->text)
			currCharsUsage[c].count++;

		const char *fname = field->fonts[0].c_str();
		if(m_fonts.find(fname) == m_fonts.end()) {
			LOG("font [%s] not loaded", fname);
			return;
		}
		for(auto &ui : currCharsUsage) {
			m_fonts[fname].add(ui.first, 0, ui.second.count);
			ui.second.positions.resize(ui.second.count);
		}
		for(auto &lui : field->lastCharsUsage) m_fonts[field->lastFonts[0]].sub(lui.first, 0, lui.second.count);

		field->lastCharsUsage = currCharsUsage;
		field->lastFonts = field->fonts;
	}

	reposition(field);

	field->createBuffer(m_EBO);
	field->populateBuffer();

}

void txt::Context::drawField(Field *field) {
	if(!field || !field->m_VAO) return;
	prepForDrawing(field);
	field->draw();
}

void::txt::Context::reposition(Field *f) {
	int advance = 0;
	int line = 0;
	for(size_t i = 0; i < f->text.length(); i++) {
		char32_t c = f->text[i];
		const GlyphInfo *gi = fontGetGlyph(f->lastFonts[0].c_str(), 0, c);
		if(!gi) {
			LOG("glyph for [%c] not found in font [%s]", c, f->lastFonts[0].c_str());
			continue;
		}
		Field::CharInfo &cu = f->lastCharsUsage[c];
		cu.positions[cu.posHead++] = { advance + gi->x, -(line * 120) + gi->y };
		advance += gi->a >> 6;
		if(advance > 400) { advance = 0; line++; }
	}
}

void txt::Field::populateBuffer() {
	size_t head = 0;
	for(auto &ui : lastCharsUsage) {
		size_t size = ui.second.positions.size() * sizeof(int) * 2;
		bufferSubData(ui.second.positions.data(), head, size);
		head += size;
	}
}