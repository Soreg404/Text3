#version 420 core

layout(location = 0) in vec2 vquad;
layout(location = 1) in ivec2 pos;

struct GlyphInfo {
	unsigned int lastInstanceId;
	int x, y, w, h;
};
uniform GlyphInfo glyphs[32];
flat out int nGlyph;

uniform mat4 uProj = mat4(1);

flat out int instID;
out vec2 uv;

void main() {

	instID = gl_InstanceID;
	GlyphInfo currGlyph;
	for(int i = 0; i < 32; i++) {
		if(instID < glyphs[i].lastInstanceId) {
			nGlyph = i;
			currGlyph = glyphs[i];
		}
	}

	switch(gl_VertexID) {
		case 0:  break;
	}

	gl_Position = uProj * vec4(vquad, -10, 1);
}