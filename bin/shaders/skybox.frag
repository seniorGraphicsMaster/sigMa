#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

out vec4 fragColor;

in vec3 tc;

uniform samplerCube cubemap;

void main() {
	fragColor = texture(cubemap, tc);
}