#version 420 core

uniform sampler2D glyphTextures[32];

flat in int nGlyph;

uniform int uCount;

flat in int instID;
in vec2 uv;

out vec4 fragColor;

void main() {
	
	float texColor = texture(glyphTextures[nGlyph], uv).r;
	fragColor = vec4(uv, float(instID) / float(uCount), texColor);
}