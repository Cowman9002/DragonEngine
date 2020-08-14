#version 330

layout (location = 0) out vec4 fragColor;

varying vec2 vTexCoord;

uniform sampler2D uNoise[2];
uniform float uTime;

void main()
{
	float speed = 20.0;
	vec2 offset = vec2(uTime / speed);

	float dval = texture2D(uNoise[1], vTexCoord * 0.7 - offset).r;
	vec2 displacement = dval * 100.0 / textureSize(uNoise[0], 0);

	speed = 6.0;
	offset = vec2(cos(uTime / speed), sin(uTime / speed));
	offset *= 0.1;
	float noiseVal = texture2D(uNoise[0], vTexCoord * 1.0 + vec2(displacement) + offset).r;
	
	speed = -20.0;
	offset = vec2(uTime / speed);
	noiseVal += texture2D(uNoise[0], -vTexCoord * 1.2 + vec2(displacement) + offset).r;

	noiseVal /= 2.0;

	float low = noiseVal < 0.4 ? noiseVal : 0.0;
	float high = noiseVal > 0.7 ? 1.0 : 0.0;

	fragColor = vec4(low + high);
}
