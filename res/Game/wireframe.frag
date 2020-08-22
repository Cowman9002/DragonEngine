#version 330

layout (location = 0) out vec4 fragColor;

uniform vec3 uColor = vec3(0.3, 1.0, 0.3);

void main()
{
	fragColor = vec4(uColor, 1.0);
}
