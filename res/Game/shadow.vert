#version 330
layout(location = 0) in vec3 aPos;

uniform mat4 uLight;
uniform mat4 uModel;

void main()
{
	gl_Position = uLight * uModel * vec4(aPos, 1.0); 
}
