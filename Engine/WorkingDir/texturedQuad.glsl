#ifdef TEXTURED_QUAD

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

uniform int drawMode;

uniform sampler2D albedo;
uniform sampler2D normals;
uniform sampler2D worldPos;
uniform sampler2D depth;

float specularStrength = 0.5;

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


layout (location = 0) out vec4 color;

vec3 CalculateAmbientLight()
{
	return uAmbientLightStrength * uAmbientLightCol;
}


vec3 CalculateDiffuse()
{
	vec3 col = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < uLightCount; ++i)
	{
		vec4 pos = texture(worldPos, vTexCoord);
		vec4 normal = texture(normals, vTexCoord);

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
	if (drawMode == 0)
	{
		vec3 ambient = CalculateAmbientLight();
		vec3 diffuse = CalculateDiffuse();

		color = vec4((ambient + diffuse) * texture(albedo, vTexCoord).xyz, 1.0);
	}

	else if (drawMode == 1)
		color = texture(albedo, vTexCoord);

	else if (drawMode == 2)
		color = texture(normals, vTexCoord);

	else if (drawMode == 3)
		color = texture(worldPos, vTexCoord);

	else if (drawMode == 4)
	{
		float depthValue = texture(depth, vTexCoord).x;
		color = vec4(depthValue, depthValue, depthValue, 1.0);
	}
		
}

#endif
#endif