#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;


layout (binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;

void main()
{
	vTexCoord = aTexCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0)).xyz;
	vNormal = normalize(uWorldMatrix * vec4(aNormal, 0.0)).xyz;

	gl_Position = uWorldProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;

uniform sampler2D uTexture;

layout (binding = 2, std140) uniform MaterialParams
{
	vec3 albedo;
	vec3 emissive;
	float reflectivity;
};

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normals;
layout (location = 2) out vec4 worldPos;
layout (location = 3) out float reflectiveTex;


void main()
{
	color = vec4(texture(uTexture, vTexCoord).xyz * albedo, 1.0);
	normals = vec4(normalize(vNormal), 1.0);
	worldPos = vec4(vPosition, 1.0);
	reflectiveTex = reflectivity;
}

#endif
#endif