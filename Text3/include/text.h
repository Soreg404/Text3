#pragma once

namespace txt {

	class Context {
		static size_t nContexts;
	public:
		Context(int porjectionWidth = 0, int porjectionHeight = 0);
		~Context();
		void setProjectionSize(int width, int height);
	};

	class Font {

	};

	class Field {

	};

}