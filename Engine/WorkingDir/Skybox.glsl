#ifdef SKYBOX

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPosition;

out vec3 vLocalPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
	vLocalPos = aPosition;

	mat4 rotView = mat4(mat3(uView));

	vec4 pos = uProjection * rotView * vec4(aPosition, 1.0);
	gl_Position = pos.xyww;
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vLocalPos;

uniform samplerCube skyBox;

out vec4 color;

void main()
{
	vec3 col = texture(skyBox, vLocalPos).rgb;

	col = col / (col + vec3(1.0));
    col = pow(col, vec3(1.0/2.2)); 
  
    color = vec4(col, 1.0);
}

#endif
#endif