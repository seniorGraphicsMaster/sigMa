#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main() {
	FragColor = texture(skybox, TexCoords);
}