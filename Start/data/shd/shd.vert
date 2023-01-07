#version 420 core

layout(location = 0) in ivec2 pos;

struct GlyphInfo {
	unsigned int lastInstanceID;
	int x, y, w, h;
};
uniform GlyphInfo glyphs[32];
flat out int nGlyph;

uniform mat4 uProj = mat4(1);

flat out int instID;
out vec2 uv;

void main() {

	instID = gl_InstanceID;
	int x = 0, y = 0, w = 0, h = 0;
	for(int i = 0; i < 32; i++) {
		if(instID < glyphs[i].lastInstanceID) {
			nGlyph = i;
			x = glyphs[i].x;
			y = glyphs[i].y;
			w = glyphs[i].w;
			h = glyphs[i].h;
			break;
		}
	}

	
	vec2 ppos;
	switch(gl_VertexID % 4) {
		case 0: ppos = vec2(x, y - h);		uv = vec2(0, 1); break;
		case 1: ppos = vec2(x, y);			uv = vec2(0, 0); break;
		case 2: ppos = vec2(x + w, y);		uv = vec2(1, 0); break;
		case 3: ppos = vec2(x + w, y - h);	uv = vec2(1, 1); break;
	}

	if(w == 0) ppos = vec2(0, 0);

	gl_Position = uProj * vec4(pos + ppos + vec2(0, 400), -1, 1);
}