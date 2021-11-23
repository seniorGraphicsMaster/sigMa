layout (location=0) in vec3 position;


out vec2 tc;

uniform mat4 lightMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightMatrix * model * vec4(position,1.0);
}  
