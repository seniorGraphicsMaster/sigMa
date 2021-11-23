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
uniform sampler2D	TEX;
uniform vec4		color;

void main()
{
	fragColor = texture( TEX, tc ); if(fragColor.a < 0.001) discard;
	fragColor = vec4(fragColor.rgb,fragColor.r)*color; // enable alpha blending
}
