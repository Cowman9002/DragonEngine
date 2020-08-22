#version 330

layout (location = 0) in vec3 aPos;

uniform mat4 uVP;

uniform vec3 uPositions[2];

void main()
{
	if(gl_VertexID < 3)
	{
		gl_Position = uVP * vec4(uPositions[gl_VertexID], 1.0);
	}
}
