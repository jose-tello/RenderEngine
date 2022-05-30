#ifdef BRIGHT_DETECTION

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

uniform sampler2D albedoTexture;
uniform float threshold;

out vec4 color;


void main()
{
	vec3 luminances = vec3(0.2126, 0.7152, 0.0722);	
	vec3 albedo = texture2D(albedoTexture, vTexCoord).rgb;
	float luminance = dot(luminances, albedo);

	luminance = max(0.0, luminance - threshold);
	albedo *= sign(luminance);
	
	color = vec4(albedo, 1.0);
}

#endif
#endif