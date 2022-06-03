#ifdef BLOOM_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;

	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D bloomMap;
uniform sampler2D colorMap;
uniform int maxLod;

uniform float lodIntensity[5];

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 bloomColor;

void main()
{
	bloomColor = vec4(0.0);
	for(int i = 0; i < maxLod; ++i)
	{
		bloomColor += textureLod(bloomMap, vTexCoord, float(i)) * lodIntensity[i];
	}

	color = vec4(bloomColor.rgb + texture(colorMap, vTexCoord).rgb, 1.0);

	bloomColor.a = 1.0;
}

#endif
#endif