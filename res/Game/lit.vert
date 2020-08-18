#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

econst int NUM_CASCADES;

varying vec3 vNorm;
varying vec2 vTexCoords;
varying vec3 vFragPos;
varying vec4 vLightFragPos[NUM_CASCADES];
varying float vClipSpacePosZ;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightMat[NUM_CASCADES];

void main()
{
	vec4 fragPos = uModel * vec4(aPos, 1.0);
	
	for(int i = 0; i < NUM_CASCADES; i++)
	{
		vLightFragPos[i] = uLightMat[i] * fragPos;
	}
	
	gl_Position = uProjection * uView * fragPos;
	vClipSpacePosZ = gl_Position.z;
	
	vNorm = mat3(transpose(inverse(uModel))) * aNorm;
	vTexCoords = aTex;
	vFragPos = fragPos.xyz;
}
