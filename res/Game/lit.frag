#version 330

#include res/std/shadow.glh
#include res/std/lighting.glh

layout (location = 0) out vec4 fragColor;

econst int NUM_CASCADES;
const float CASCADE_BLEND_DIST = 0.7;

varying vec3 vNorm;
varying vec2 vTexCoords;
varying vec3 vFragPos;
varying vec4 vLightFragPos[NUM_CASCADES];
varying float vClipSpacePosZ;

uniform float uTime;

uniform vec3 uColor = vec3(1.0);
uniform vec3 uCamPos;  
uniform sampler2D uTexture;
uniform sampler2D uToonMap;
uniform sampler2D uToonMap2;
uniform bool uHasTexture;
uniform samplerCube uSkybox;
uniform vec3 uLightDir = normalize(vec3(1.0));
uniform sampler2D uShadowMap[NUM_CASCADES];
uniform float uCascadeEnd[NUM_CASCADES];

const vec3 Radiance = vec3(1.0);

const vec3 AmbFactor = vec3(0.1);

uniform float uMetalness = 0.0;
uniform float uShininess = 5;
uniform float uReflectShininess = 0.0;

const float SpecFactor = 0.5;

const float e = 2.71828182846;

float getShadowMultiplier(int cascade, float NdotL);

mat4x4 thresholdMatrix =
{
	{1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0},
	{13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0},
	{4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0},
	{16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0}
};

void clip(float value)
{
	if(value <= 0)
	{
		discard;
	}
}

const float SCREEN_NEAR = 0.1;
const float SCREEN_FAR = 0.3;
const vec2 SHADOW_BIAS = vec2(0.003, 0.0008);

void main()
{
	float alpha = 1.0;
	vec3 albedo = (uColor, vec3(2.2));
	
	if(uHasTexture)
	{
		vec4 t = texture2D(uTexture, vTexCoords);
		albedo *= t.rgb;
		alpha = t.a;
	}

	float distToCam = distance(uCamPos, vFragPos);
	alpha = min(alpha, min(1.0, (distToCam - SCREEN_NEAR) / (SCREEN_FAR - SCREEN_NEAR)));

	clip(alpha - thresholdMatrix[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4]);

	vec3 N = normalize(vNorm);
	vec3 L = normalize(-uLightDir);
	vec3 V = normalize(uCamPos - vFragPos);
	
	float shadow_samples = 6.0;
	float shadow_tile = 0.6;
	float shadowMult = 1.0;
	for (int i = 0 ; i < NUM_CASCADES ; i++)
	{
        if (vClipSpacePosZ <= uCascadeEnd[i])
		{
			float distToCas = uCascadeEnd[i] - vClipSpacePosZ;
			
			shadowMult = getShadowMultiplierRandomBlur(vLightFragPos[i], uShadowMap[i], dot(N, L), SHADOW_BIAS, shadow_samples - i, shadow_tile, vFragPos);
			
			float blendDist = CASCADE_BLEND_DIST * vClipSpacePosZ / 1.414214;
			if(distToCas < blendDist)
			{
				if(i + 1 >= NUM_CASCADES)
				{
					shadowMult = mix(1.0f, shadowMult, distToCas / blendDist);
				}
				else
				{
					float border_shadow = getShadowMultiplierRandomBlur(vLightFragPos[i + 1], uShadowMap[i + 1], dot(N, L), SHADOW_BIAS, shadow_samples - i - 1, shadow_tile, vFragPos);
					shadowMult = mix(border_shadow, shadowMult, distToCas / blendDist);
				}
			}
			
            break;
        }
    }
	
	vec3 diffuse = toonDiffuse(N, L, uToonMap, albedo, Radiance);
	vec3 specular = toonSpecular(N, L, V, 0.5, uToonMap2, Radiance);
	vec3 ambient = AmbFactor * albedo; 

	vec3 light = diffuse + specular;
	vec3 final = ambient + light * shadowMult;

	fragColor = vec4(final, 1.0);
}

/*float getShadowMultiplier(int cascade, float NdotL)
{
	if(cascade >= NUM_CASCADES)
	{
		return 1.0;
	}

	vec4 ls_pos = vLightFragPos[cascade];
	vec3 mapped = ls_pos.xyz / ls_pos.w;
	
	// convert to 0 - 1 space
	mapped = mapped * 0.5 + 0.5;
	
	float currentDepth = mapped.z;
	
	float light = 0.0;
	
	if(mapped.x > 1.0 || mapped.x < 0.0 || mapped.y > 1.0 || mapped.y < 0.0)
	{
		light = 0.0;
	}
	
	float bias = max(0.005 * (1.0 - NdotL), 0.0008); 
	//float bias = 0.003;
	
	//float closestDepth = texture(uShadowMap[cascade], mapped.xy).r; 
	//light += currentDepth - bias > closestDepth ? 0.0 : 1.0;  
	
	vec2 texelSize = 1.0 / textureSize(uShadowMap[cascade], 0);
	
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(uShadowMap[cascade], mapped.xy + vec2(x, y) * texelSize).r; 
			light += currentDepth - bias > pcfDepth ? 0.0 : 1.0;           
		}    
	}
	light /= 9.0;
	
	float nearestDepth = texture2D(uShadowMap[cascade], mapped.xy).r; 
	vec2 texelSize = 1.0 / textureSize(uShadowMap[cascade], 0);
	int sample_size = 4;
	float tile_size = pow(1.414314, abs(currentDepth - nearestDepth) * 10.0);
	
	seed = vFragPos.xy * vFragPos.zx;
	for(int i = 0; i < sample_size; i++)
	{
		for(int j = 0; j < sample_size; j++)
		{
			vec2 offset = tile_size * vec2(i, j);
			//randomize points slightly
			offset += randVec2() * tile_size;
			// center offset
			offset -= offset / 2.0;
			
			float pcfDepth = texture2D(uShadowMap[cascade], mapped.xy + offset * texelSize).r; 
			light += currentDepth - bias > pcfDepth ? 0.0 : 1.0;  
		}
	}
	light /= sample_size * sample_size;
	
	if(ls_pos.z > 1.0)
	{	
		light = 0.0;
	}
	
	return light;
}*/
