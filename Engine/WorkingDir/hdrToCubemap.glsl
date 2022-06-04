#ifdef HDR_TO_CUBEMAP

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPosition;

out vec3 vLocalPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
	vLocalPos = aPosition;

	gl_Position = uProjection * uView * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vLocalPos;

uniform sampler2D hdrMap;

out vec4 color;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(vLocalPos));
	vec3 col = min(vec3(1000.0), texture(hdrMap, uv).rgb);

	color = vec4(col, 1.0);
}

#endif
#endif