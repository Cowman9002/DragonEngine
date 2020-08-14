#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;

varying vec2 vTex;

uniform vec2 uScale = vec2(1);
uniform vec2 uOffset;

void main()
{
	vec2 pos = aPos.xy;
	pos *= uScale * 2;
	pos += uOffset - vec2(1.0);
	
	gl_Position = vec4(pos.x, pos.y, 0.0, 1);
	vTex = aTex;
}
