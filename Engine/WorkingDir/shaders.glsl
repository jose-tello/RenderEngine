#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

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


layout (binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;
out vec3 vViewDir;

void main()
{
	vTexCoord = aTexCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0)).xyz;
	vNormal = normalize(uWorldMatrix * vec4(aNormal, 0.0)).xyz;
	vViewDir = normalize(uCameraPosition - vPosition);

	gl_Position = uWorldProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normals;
layout (location = 2) out vec4 worldPos;

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

vec3 CalculateAmbientLight()
{
	return uAmbientLightStrength * uAmbientLightCol;
}


vec3 CalculateDiffuse()
{
	vec3 col = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < uLightCount; i++)
	{
		if (uLight[i].type == 0)
		{
			float diff = max(dot(vNormal, normalize(uLight[i].direction)), 0.0);
			col += diff * uLight[i].color;
		}


		else
		{
			vec3 dir = normalize(uLight[i].position - vPosition);
			float diff = max(dot(vNormal, dir), 0.0);
			float atenuation = 1.0 - smoothstep(0.0, uLight[i].maxDistance, length(uLight[i].position - vPosition));
			col += diff * uLight[i].color * atenuation;
		}
	}

	return col;
}


void main()
{
	//vec3 ambient = CalculateAmbientLight();
	//vec3 diffuse = CalculateDiffuse();
	//color = vec4((ambient + diffuse) * texture(uTexture, vTexCoord).xyz, 1.0);
	color = texture(uTexture, vTexCoord);
	normals = vec4(normalize(vNormal), 1.0);
	worldPos = vec4(vPosition, 1.0);
}

#endif
#endif