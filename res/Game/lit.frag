#version 330

layout (location = 0) out vec4 fragColor;

econst int NUM_CASCADES;

varying vec3 vNorm;
varying vec2 vTexCoords;
varying vec3 vFragPos;
varying vec4 vLightFragPos[NUM_CASCADES];
varying float vClipSpacePosZ;

uniform float uTime;

uniform vec3 uColor = vec3(1.0);
uniform vec3 uCamPos;
uniform sampler2D uTexture;
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
const float PI = 3.141592654;

float getShadowMultiplier(int cascade, float NdotL);
float rand(vec2 co);
vec2 seed;
vec2 randVec2();

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
const float SCREEN_FAR = 0.6;

void main()
{
	float alpha = 1.0;
	vec3 albedo = pow(uColor, vec3(2.2));
	
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
	vec3 H = normalize(L + V);

	float NdotL = max(0.0, dot(N, L));
	float specPow = max(0.0, dot(H, N));
	specPow = pow(specPow, uShininess * 16);
	
	vec3 reflection = textureCube(uSkybox, reflect(-V, N), (1 - uReflectShininess) * 9).rgb;
	reflection *= albedo * uMetalness;
	
	vec3 irrad = textureCube(uSkybox, N, 9).rgb;
	
	vec3 ambient = albedo * irrad * AmbFactor + reflection;
	vec3 diffuse = albedo * (1.0 - uMetalness);
	vec3 specular = specPow * mix(vec3(SpecFactor), albedo, uMetalness);

	float shadowMult = 1.0;
	
	for (int i = 0 ; i < NUM_CASCADES ; i++)
	{
        if (vClipSpacePosZ <= uCascadeEnd[i])
		{
            shadowMult = getShadowMultiplier(i, NdotL);
            break;
        }
    }

	vec3 light = (diffuse + specular) * NdotL * Radiance;

	vec3 final = ambient + light * shadowMult;

	fragColor = vec4(final, 1.0);
}

float getShadowMultiplier(int cascade, float NdotL)
{
	vec4 ls_pos = vLightFragPos[cascade];
	vec3 mapped = ls_pos.xyz / ls_pos.w;
	
	// convert to 0 - 1 space
	mapped = mapped * 0.5 + 0.5;
	
	float currentDepth = mapped.z;
	
	float light = 0.0;
	
	/*if(mapped.x > 1.0 || mapped.x < 0.0 || mapped.y > 1.0 || mapped.y < 0.0)
	{
		light = 0.0;
	}*/
	
	float bias = max(0.01 * (1.0 - NdotL), 0.0005); 
	//float bias = 0.003;
	
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
	
	/*float nearestDepth = texture2D(uShadowMap[cascade], mapped.xy).r; 
	
	int sample_size = 8;
	float v = clamp((currentDepth - nearestDepth) / 3.0, 0.0, 1.0);
	float tile_size = mix(1.0, 25.0, v);
	
	seed = mapped.xy * mapped.z;
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
	light /= sample_size * sample_size;*/
	
	if(ls_pos.z > 1.0)
	{	
		light = 0.0;
	}
	
	return light;
}

float rand(vec2 co)
{
    return (fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453) - 0.5) * 2.0;
}

vec2 randVec2()
{
    vec2 v;
	v.x = rand(seed);
	seed = vec2(v.x, seed.x);
	v.y = rand(seed);
	seed = vec2(v.y, v.x);
	return v;
}
