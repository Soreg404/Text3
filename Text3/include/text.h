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
	};

	class Field {
		friend class Context;
	public:

		std::vector<std::string> fonts;
		int fontSize = 16;
		std::string text;

	private:

		struct CharInfo {
			unsigned int count = 0;
		};

		std::vector<std::string> lastFonts;
		std::map<char32_t, CharInfo> lastCharsUsage;

	};

	class Context {
		static size_t nContexts;
	public:
		Context(int porjectionWidth = 0, int porjectionHeight = 0);
		~Context();
		void setProjectionSize(int width, int height);

		void fontLoad(const char *path, const char *name);
		void fontUnload(const char *name);

		GlyphInfo fontGetGlyph(const char *fontName, char32_t charcter);

		void fieldLoad(Field *textField);
		void fieldDraw(Field *textField);

	private:

		void initGL();
		int nTexUnits = 0;
		unsigned int shdId = 0;
		unsigned int VAO = 0, VBO = 0, EBO = 0;

		struct Font {
			void add(char32_t c, int size, unsigned int count);
			void sub(char32_t c, int size, unsigned int count);
			void *h_ft = nullptr;
			static unsigned int loadTexture(unsigned int x, unsigned int y, void *buff);
			static void unloadTexture(unsigned int *id);
			std::unordered_map<char32_t, GlyphInfo> glyphs;
		};

		std::unordered_map<std::string, Font> m_fonts;

		struct UniformGlyphInfo {
			unsigned int lastInstanceId = 0;
			int x = 0, y = 0, w = 0, h = 0, a = 0;
		};
		std::vector<UniformGlyphInfo> m_uGlyphs;

	};



}