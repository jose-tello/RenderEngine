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
uniform sampler2D defaultTexture;

layout (location = 0) out vec4 color;


void main()
{
	if (drawMode == 0)
		color = texture(defaultTexture, vTexCoord);

	else if (drawMode == 1)
		color = texture(albedo, vTexCoord);

	else if (drawMode == 2)
		color = texture(normals, vTexCoord);

	else if (drawMode == 3)
		color = texture(worldPos, vTexCoord);

	else if (drawMode == 4)
	{
		float depthValue = texture(depth, vTexCoord).x;
		color = vec4(depthValue.xxx, 1.0);
	}
		
}

#endif
#endif