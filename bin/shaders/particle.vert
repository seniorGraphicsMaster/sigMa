// vertex attributes
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

out vec2 tc;	// texture coordinate

// uniforms
uniform mat4	model_matrix;	// 4x4 transformation matrix
uniform mat4	aspect_matrix;	// tricky 4x4 aspect-correction matrix

void main()
{
	gl_Position = aspect_matrix*model_matrix*vec4(position,1);
	tc = texcoord;
}