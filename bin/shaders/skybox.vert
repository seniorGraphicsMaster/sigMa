layout (location = 0) in vec3 position;
out vec3 tc;

uniform mat4 projection_matrix;
uniform mat4 view_matrix; 
void main() 
{
	tc = position;
	gl_Position = projection_matrix * view_matrix * vec4(position, 1.0);

}
