#version 330
out vec4 fragColor;

varying vec3 vTex;

uniform samplerCube uTexture;
uniform vec3 uSunDir = normalize(vec3(1, 1, 1));

const vec3 sunColor = vec3(4.0, 4.0, 1.4);
const vec3 rimColor = vec3(3.0f, -1.2f, -3.0);
const float sunSize = 0.005;

void main()
{

	vec3 D = normalize(vTex);
	vec3 S = normalize(-uSunDir);

	vec3 linearColor = textureCube(uTexture, D).rgb;
	
	float DdotS = max(0.0, dot(D, S));
	
	float iSunSize = 1 - sunSize;
	float sunMask = DdotS > iSunSize ? 1.0 : 0.0;
	float sunFactor = (DdotS - iSunSize) * (1.0 / sunSize);
	sunFactor = pow(sunFactor, 0.1);
	
	vec3 sun = mix(rimColor, sunColor, sunFactor) * sunMask;
	
	vec3 sky = linearColor + sun;
	
	fragColor = vec4(sky, 1.0);
	//fragColor = vec4(vec3(sunFactor * sunMask), 1.0);
}
