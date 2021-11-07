#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// input from vertex shader
in vec2 tc;

// the only output variable
out vec4 fragColor;

// texture sampler
uniform sampler2D TEX;
uniform bool use_texture;
uniform vec4 diffuse;

void main()
{
	fragColor = use_texture ? texture( TEX, tc ) : diffuse;
}