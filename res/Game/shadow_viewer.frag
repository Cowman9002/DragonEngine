#version 330
layout (location = 0) out vec3 fragColor;

econst int NUM_CASCADES;
const float CASCADE_BLEND_DIST = 1.0;

varying vec2 vTexCoords;
varying vec4 vLightFragPos[NUM_CASCADES];
varying float vClipSpacePosZ;

uniform sampler2D uShadowMap[NUM_CASCADES];
uniform float uCascadeEnd[NUM_CASCADES];

const vec3 CascadeColors[] = 
{
	vec3(1.0, 0.6, 0.6),
	vec3(0.6, 1.0, 0.6),
	vec3(0.6, 0.6, 1.0),
	vec3(1.0, 1.0, 0.6),
	vec3(0.6, 1.0, 1.0),
	vec3(1.0, 0.6, 1.0),
	vec3(1.0, 1.0, 1.0)
};


const float PI = 3.141592654;

float getShadowMultiplier(int cascade);
float rand(vec2 co);
vec2 seed;
vec2 randVec2();

void main()
{
	float shadowMult = 1.0;
	int cascade = 7;
	
	for (int i = 0 ; i < NUM_CASCADES ; i++)
	{

        if (vClipSpacePosZ <= uCascadeEnd[i])
		{
			float distToCas = uCascadeEnd[i] - vClipSpacePosZ;
            shadowMult = getShadowMultiplier(i);
			cascade = i;
			if(distToCas < CASCADE_BLEND_DIST)
			{
				if(i + 1 >= NUM_CASCADES)
				{
					break;
				}
				shadowMult = mix(getShadowMultiplier(i + 1), shadowMult, distToCas / CASCADE_BLEND_DIST);
			}
			
            break;
        }
    }

	fragColor = 0.5 * shadowMult * CascadeColors[cascade];
}

float getShadowMultiplier(int cascade)
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
	
	float bias = 0.003;
	
	float closestDepth = texture(uShadowMap[cascade], mapped.xy).r; 
	light += currentDepth - bias > closestDepth ? 0.0 : 1.0;  
	
	/*vec2 texelSize = 1.0 / textureSize(uShadowMap[cascade], 0);
	
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(uShadowMap[cascade], mapped.xy + vec2(x, y) * texelSize).r; 
			light += currentDepth - bias > pcfDepth ? 0.0 : 1.0;           
		}    
	}
	light /= 9.0;*/
	
	/*float nearestDepth = texture2D(uShadowMap[cascade], mapped.xy).r; 
	vec2 texelSize = 1.0 / textureSize(uShadowMap[cascade], 0);
	int sample_size = 5;
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
