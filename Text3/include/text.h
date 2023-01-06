#pragma once

#include <unordered_map>
#include <map>
#include <vector>

#include <iostream>
#define LOG(x, ...) printf(__FUNCTION__ ": " x "\n", ## __VA_ARGS__)

namespace txt {

	struct GlyphInfo {
		size_t uses = 0;
		int x = 0, y = 0, w = 0, h = 0, a = 0;
		unsigned int id = 0;
		~GlyphInfo();
	private:
		friend class Context;
		void loadTexture(const void *buffer);
		void unloadTexture();
	};

	class Field {
		friend class Context;
	public:

		std::vector<std::string> fonts;
		int fontSize = 16;
		std::string text;


		Field();
		~Field();

	private:

		struct CharInfo {
			unsigned int count = 0, posHead = 0;
			std::vector<std::pair<int, int>> positions;
		};

		std::vector<std::string> lastFonts;
		std::map<char32_t, CharInfo> lastCharsUsage;

		unsigned int m_VAO = 0, m_posBufferId = 0;
		void createBuffer(unsigned int contextQuadVBO, unsigned int contextEBO);
		void deleteBuffer();
		void bufferSubData(const void *buffer, int head, int size);

		void populateBuffer();

		void draw();

	};

	class Context {
		static size_t nContexts;
	public:
		Context(int porjectionWidth = 0, int porjectionHeight = 0);
		~Context();
		void setCtxSize(int width, int height);

		void fontLoad(const char *path, const char *name);
		void fontUnload(const char *name);

		const GlyphInfo *fontGetGlyph(const char *fontName, int size, char32_t charcter);

		void fieldLoad(Field *textField);
		void fieldDraw(Field *textField);

	private:

		void initGL();
		int m_nTexUnits = 0;
		unsigned int m_shdId = 0;
		unsigned int m_VBO = 0, m_EBO = 0;

		struct Font {
			void add(char32_t c, int size, unsigned int count);
			void sub(char32_t c, int size, unsigned int count);
			void *h_ft = nullptr;
			std::unordered_map<char32_t, GlyphInfo> glyphs;
		};

		std::unordered_map<std::string, Font> m_fonts;

		struct UniformGlyphInfo {
			unsigned int lastInstanceId = 0;
			int x = 0, y = 0, w = 0, h = 0;
		};
		std::vector<UniformGlyphInfo> m_uGlyphs;

		void reposition(Field *textField);

		void prepForDrawing();

	};



}