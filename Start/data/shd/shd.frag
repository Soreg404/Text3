#version 420 core

uniform sampler2D glyphTextures[32];


flat in int instID;
in vec2 uv;

out vec4 fragColor;

void main() {
	 //fragColor = vec4(uv, instID * .01, 1);
	 fragColor = vec4(1, 0, 1, 1);
}