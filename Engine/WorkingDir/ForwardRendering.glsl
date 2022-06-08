#ifdef FORWARD_RENDER

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
uniform samplerCube irradianceMap;

float specularStrength = 0.5;

layout (binding = 2, std140) uniform MaterialParams
{
	vec3 albedo;
	vec3 emissive;
	float reflectivity;
};

layout (location = 0) out vec4 color;

struct Light
{
	unsigned int type;
	float maxDistance;
	vec3 color;
	vec3 direction;
	vec3 position;
};

layout (binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;

	float uAmbientLightStrength;
	vec3 uAmbientLightCol;

	Light uLight[16];
};


vec3 CalculateAmbientLight(vec3 pos, vec3 normal)
{
	vec3 viewDir = normalize(uCameraPosition - pos.xyz);

	return texture(irradianceMap, reflect(-viewDir, normal.xyz)).rgb * uAmbientLightStrength;
}


vec3 CalculateDiffuse(vec3 pos, vec3 normal)
{
	vec3 col = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < uLightCount; ++i)
	{
		vec3 viewDir = normalize(uCameraPosition - pos.xyz);
		vec3 reflectDir;

		if (uLight[i].type == 0)
		{
			reflectDir = reflect(normalize(-uLight[i].direction), normal.xyz);

			float diff = max(dot(normal.xyz, normalize(uLight[i].direction)), 0.0);
			col += diff * uLight[i].color;
		}


		else
		{
			vec3 dir = normalize(uLight[i].position - pos.xyz);
			reflectDir = reflect(-dir, normal.xyz);

			float diff = max(dot(normal.xyz, dir), 0.0);
			float atenuation = 1.0 - smoothstep(0.0, uLight[i].maxDistance, length(uLight[i].position - pos.xyz));
			col += diff * uLight[i].color * atenuation;
		}

		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
		vec3 specular = specularStrength * spec * uLight[i].color;

		col += specular;
	}

	return col;
}


void main()
{

	vec3 ambient = CalculateAmbientLight(vPosition, vNormal);
	vec3 diffuse = CalculateDiffuse(vPosition, vNormal);

	color = vec4((ambient + diffuse) * texture(uTexture, vTexCoord).rgb, 1.0);		
}

#endif
#endif