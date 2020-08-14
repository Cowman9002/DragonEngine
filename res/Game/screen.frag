#version 330 core
out vec4 fragColor;

varying vec2 vTex;

uniform sampler2D uTexture;
uniform bool uSingle;

void main()
{
	vec3 finalColor = texture2D(uTexture, vTex).rgb;
	
	if(uSingle)
	{
		fragColor = vec4(vec3(finalColor.r), 1.0);
	}
	else
	{
		//vec3 mapped = vec3(1.0) - exp(-finalColor * uContrast);
		vec3 mapped = finalColor / (finalColor + vec3(1.0));
		
		// gamma correction 
		mapped = pow(mapped, vec3(1.0 / 2.2));
	
		fragColor = vec4(mapped, 1.0);
	}
}
