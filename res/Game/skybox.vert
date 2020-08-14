#version 330
layout(location = 0) in vec3 aPos;

varying vec3 vTex;

uniform mat4 uVP;

void main()
{
	vec4 pos = uVP * vec4(aPos, 1.0);
	gl_Position = pos.xyww; 
	vTex = aPos;
}
