#ifdef LIGHT_PASS

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

uniform sampler2D albedo;
uniform sampler2D normals;
uniform sampler2D worldPos;
uniform sampler2D reflectivity;
uniform samplerCube skyBox;
uniform samplerCube irradianceMap;

float specularStrength = 0.5;

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


vec3 CalculateAmbientLight(vec4 pos, vec4 normal)
{
	vec3 viewDir = normalize(uCameraPosition - pos.xyz);

	return texture(irradianceMap, reflect(-viewDir, normal.xyz)).rgb * uAmbientLightStrength;
}


vec3 CalculateDiffuse(vec4 pos, vec4 normal)
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


vec3 CalculateReflection(vec4 pos, vec4 normal)
{
	vec3 viewDir = normalize(uCameraPosition - pos.xyz);

	return texture(skyBox, reflect(-viewDir, normal.xyz)).rgb;
}


void main()
{

	vec4 pos = texture(worldPos, vTexCoord);
	vec4 normal = texture(normals, vTexCoord);
	float reflectionValue = texture(reflectivity, vTexCoord).x;

	if (pos.xyz == vec3(0.0) && normal.xyz == vec3(0.0))
	{
		color = vec4(texture(albedo, vTexCoord).xyz, 1.0);		
	}

	else
	{
		vec3 ambient = CalculateAmbientLight(pos, normal);
		vec3 diffuse = CalculateDiffuse(pos, normal);
		vec3 reflection = CalculateReflection(pos, normal);

		color = vec4((ambient + diffuse) * mix(texture(albedo, vTexCoord).xyz, reflection, reflectionValue), 1.0);		
	}
	
}

#endif
#endif