//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "assimp_model_loading.h"

#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
	GLchar  infoLogBuffer[1024] = {};
	GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
	GLsizei infoLogSize;
	GLint   success;

	char versionString[] = "#version 430\n";
	char shaderNameDefine[128];
	sprintf(shaderNameDefine, "#define %s\n", shaderName);
	char vertexShaderDefine[] = "#define VERTEX\n";
	char fragmentShaderDefine[] = "#define FRAGMENT\n";

	const GLchar* vertexShaderSource[] = {
		versionString,
		shaderNameDefine,
		vertexShaderDefine,
		programSource.str
	};
	const GLint vertexShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(vertexShaderDefine),
		(GLint)programSource.len
	};
	const GLchar* fragmentShaderSource[] = {
		versionString,
		shaderNameDefine,
		fragmentShaderDefine,
		programSource.str
	};
	const GLint fragmentShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(fragmentShaderDefine),
		(GLint)programSource.len
	};

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, vshader);
	glAttachShader(programHandle, fshader);
	glLinkProgram(programHandle);
	glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	glUseProgram(0);

	glDetachShader(programHandle, vshader);
	glDetachShader(programHandle, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return programHandle;
}


u32 CreateProgram(App* app, const char* filepath, const char* programName)
{
	u32 ret = LoadProgram(app, filepath, programName);
	Program& program = app->programs[ret];

	int attributeCount;
	glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

	for (int i = 0; i < attributeCount; ++i)
	{
		char* name = new char[1000];
		int length;
		int size;
		GLenum type;

		glGetActiveAttrib(program.handle, i, 1000, &length, &size, &type, name);
		u8 attributeLocation = glGetAttribLocation(program.handle, name);

		u8 attributeCount = 0;
		switch (type)
		{
		case GL_FLOAT:
			attributeCount = 1;
			break;

		case GL_FLOAT_VEC2:
			attributeCount = 2;
			break;

		case GL_FLOAT_VEC3:
			attributeCount = 3;
			break;

		case GL_FLOAT_VEC4:
			attributeCount = 4;
			break;
		default:
			break;
		}

		program.layout.attributes.push_back(VertexShaderAttribute(attributeLocation, attributeCount));
		delete[] name;
	}

	return ret;
}


u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
	String programSource = ReadTextFile(filepath);

	Program program = {};
	program.handle = CreateProgramFromSource(programSource, programName);
	program.filepath = filepath;
	program.programName = programName;
	program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
	app->programs.push_back(program);

	return app->programs.size() - 1;
}


Image LoadImage(const char* filename)
{
	Image img = {};
	stbi_set_flip_vertically_on_load(true);
	img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
	if (img.pixels)
	{
		img.stride = img.size.x * img.nchannels;
	}
	else
	{
		ELOG("Could not open file %s", filename);
	}
	return img;
}


void FreeImage(Image image)
{
	stbi_image_free(image.pixels);
}


GLuint CreateTexture2DFromImage(Image image)
{
	GLenum internalFormat = GL_RGB8;
	GLenum dataFormat = GL_RGB;
	GLenum dataType = GL_UNSIGNED_BYTE;

	switch (image.nchannels)
	{
	case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
	case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
	default: ELOG("LoadTexture2D() - Unsupported number of channels");
	}

	GLuint texHandle;
	glGenTextures(1, &texHandle);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
	for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
		if (app->textures[texIdx].filepath == filepath)
			return texIdx;

	Image image = LoadImage(filepath);

	if (image.pixels)
	{
		Texture tex = {};
		tex.handle = CreateTexture2DFromImage(image);
		tex.filepath = filepath;

		u32 texIdx = app->textures.size();
		app->textures.push_back(tex);

		FreeImage(image);
		return texIdx;
	}
	else
	{
		return UINT32_MAX;
	}
}


//Init------------------------------------------------------------------
void Init(App* app)
{
	GetAppInfo(app);

	InitResources(app);

	if (GLVersion.major > 4 ||  (GLVersion.major == 4 && GLVersion.minor >= 3))
	{
		glDebugMessageCallback(OnGlError, app);
	}

	app->mode = Mode_Model;
}


void GetAppInfo(App* app)
{
	app->info.version = (char*)glGetString(GL_VERSION);
	app->info.render = (char*)glGetString(GL_RENDERER);
	app->info.vendor = (char*)glGetString(GL_VENDOR);
	app->info.shadingLanguageVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	int extensionCount;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);

	for (int i = 0; i < extensionCount; ++i)
	{
		app->info.extensions.push_back((char*)glGetStringi(GL_EXTENSIONS, i));
	}
}


void InitResources(App* app)
{
	//Create screen rect
	Model screenRect = {};
	Mesh mesh = {};

	VertexBufferLayout vertexLayout = {};
	vertexLayout.stride = 5 * sizeof(float);
	vertexLayout.attributes.push_back(VertexBufferAttribute(0, 3, 0));
	vertexLayout.attributes.push_back(VertexBufferAttribute(1, 2, 3 * sizeof(float)));
	
	LoadModel(app, "Patrick/Patrick.obj");


	// - vertex buffers
	glGenBuffers(1, &app->embeddedVertices);
	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// - element/index buffers
	glGenBuffers(1, &app->embeddedElements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndices), rectIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// - vaos
	glGenVertexArrays(1, &app->vao);
	glBindVertexArray(app->vao);
	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
	glBindVertexArray(0);

	// - programs (and retrieve uniform indices)
	app->screenRectProgramIdx = CreateProgram(app, "shaders.glsl", "TEXTURED_RECT");
	app->rectUniformTexture = glGetUniformLocation(app->programs[app->screenRectProgramIdx].handle, "uTexture");

	app->texturedGeometryProgramIdx = CreateProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
	app->geometryUniformTexture = glGetUniformLocation(app->programs[app->texturedGeometryProgramIdx].handle, "uTexture");

	// - textures
	app->diceTexIdx = LoadTexture2D(app, "dice.png");
	app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
	app->blackTexIdx = LoadTexture2D(app, "color_black.png");
	app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
	app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");
}


//GUI------------------------------------------------------------------------------------------
void Gui(App* app)
{
	ImGui::Begin("Info");
	ImGui::Text("FPS: %f", 1.0f / app->deltaTime);

	if (ImGui::BeginMenu("RenderMode"))
	{
		if (ImGui::MenuItem("Textured quad"))
			app->mode = Mode_TexturedQuad;

		if (ImGui::MenuItem("Model"))
			app->mode = Mode_Model;

		ImGui::EndMenu();
	}

	ImGui::Separator();
	ImGui::Text("OpenGl version: %s", app->info.version.c_str());
	ImGui::Text("GPU: %s", app->info.render.c_str());
	ImGui::Text("Vendor: %s", app->info.vendor.c_str());
	ImGui::Text("Shading language version: %s", app->info.shadingLanguageVersion.c_str());

	int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

	bool open = ImGui::TreeNodeEx("Extensions", flags);

	if (open == true)
	{
		int extensionCount = app->info.extensions.size();
		flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf;

		for (int i = 0; i < extensionCount; ++i)
		{
			ImGui::TreeNodeEx(app->info.extensions[i].c_str(), flags);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	ImGui::End();
}


//Update----------------------------------------------------------------------------
void Update(App* app)
{
	// You can handle app->input keyboard/mouse here
	CheckToUpdateShaders(app);
}


void CheckToUpdateShaders(App* app)
{
	int programCount = app->programs.size();

	for (int i = 0; i < programCount; ++i)
	{
		std::string path = app->programs[i].filepath;
		u64 currentTimeStamp = GetFileLastWriteTimestamp(path.c_str());

		if (app->programs[i].lastWriteTimestamp < currentTimeStamp)
		{
			glDeleteProgram(app->programs[i].handle);

			String source = ReadTextFile(app->programs[i].filepath.c_str());
			app->programs[i].handle = CreateProgramFromSource(source, app->programs[i].programName.c_str());
			app->programs[i].lastWriteTimestamp = currentTimeStamp;
		}
	}
}


//Render----------------------------------------------------------------------------
void Render(App* app)
{
	switch (app->mode)
	{
	case Mode_TexturedQuad:
	{
		RenderTexturedQuad(app);
	}
	break;

	case Mode_Model:
	{
		RenderModels(app);
	}
	break;

	default:;
	}
}


u32 FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
	Submesh& submesh = mesh.submeshes[submeshIndex];

	int vaoCount = submesh.vaos.size();

	for (int i = 0; i < vaoCount; ++i)
	{
		if (submesh.vaos[i].programHandle == program.handle)
			return submesh.vaos[i].handle;
	}

	u32 vaoHandle = 0;

	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

	int vAttribCount = program.layout.attributes.size();

	for (int i = 0; i < vAttribCount; ++i)
	{
		bool attribLinked = false;

		int bAttribCount = submesh.vertexBufferLayout.attributes.size();
		for (int j = 0; j < bAttribCount; ++j)
		{
			if (program.layout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
			{
				u32 index = submesh.vertexBufferLayout.attributes[j].location;
				u32 nComp = submesh.vertexBufferLayout.attributes[j].componentCount;
				u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;
				u32 stride = submesh.vertexBufferLayout.stride;

				glVertexAttribPointer(index, nComp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
				glEnableVertexAttribArray(index);

				attribLinked = true;
				break;
			}
		}

		assert(attribLinked, "Missed attribute link");
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	submesh.vaos.push_back(Vao(vaoHandle, program.handle));
	return vaoHandle;
}


void RenderTexturedQuad(App* app)
{
	// - clear the framebuffer
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	Program programTexGeo = app->programs[app->screenRectProgramIdx];
	glUseProgram(programTexGeo.handle);
	glBindVertexArray(app->vao);

	// - set the blending state
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// - bind the texture into unit 0
	glUniform1i(app->rectUniformTexture, 0);
	glActiveTexture(GL_TEXTURE0);

	unsigned int texHandle = app->textures[app->diceTexIdx].handle;
	glBindTexture(GL_TEXTURE_2D, texHandle);

	// - draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}


void RenderModels(App* app)
{
	// - clear the framebuffer
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	Program programTexGeo = app->programs[app->texturedGeometryProgramIdx];
	glUseProgram(programTexGeo.handle);
	
	int modelCount = app->models.size();
	for (int i = 0; i < modelCount; ++i)
	{
		Model& model = app->models[i];
		Mesh& mesh = app->meshes[model.meshIdx];

		int submeshCount = mesh.submeshes.size();

		for (int j = 0; j < submeshCount; ++j)
		{
			u32 vao = FindVAO(mesh, j, programTexGeo);
			glBindVertexArray(vao);

			u32 materialIdx = model.materialIdx[j];
			Material& material = app->materials[materialIdx];

			glBindTexture(GL_TEXTURE_2D, app->textures[material.albedoTextureIdx].handle);
			glUniform1i(app->geometryUniformTexture, 0);
			glActiveTexture(GL_TEXTURE0);

			Submesh& submesh = mesh.submeshes[j];
			glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		}
	}

	glBindVertexArray(0);
	glUseProgram(0);
}


void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* useParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	ELOG("OpenGL debug message: %s", message);

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:				ELOG(" - source: GL_DEBUG_SOURCE_API"); break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM"); break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:	ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER"); break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:		ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY"); break;
	case GL_DEBUG_SOURCE_APPLICATION:		ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION");	break;
	case GL_DEBUG_SOURCE_OTHER:				ELOG(" - source: GL_DEBUG_SOURCE_OTHER"); break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:				ELOG(" - type: GL_DEBUG_TYPE_ERROR"); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ELOG(" - type: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	ELOG(" - type: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_PORTABILITY:			ELOG(" - type: GL_DEBUG_TYPE_PORTABILITY"); break;
	case GL_DEBUG_TYPE_PERFORMANCE:			ELOG(" - type: GL_DEBUG_TYPE_PERFORMANCE"); break;
	case GL_DEBUG_TYPE_MARKER:				ELOG(" - type: GL_DEBUG_TYPE_MARKER"); break;
	case GL_DEBUG_TYPE_PUSH_GROUP:			ELOG(" - type: GL_DEBUG_TYPE_PUSH_GROUP"); break;
	case GL_DEBUG_TYPE_POP_GROUP:			ELOG(" - type: GL_DEBUG_TYPE_POP_GROUP"); break;
	case GL_DEBUG_TYPE_OTHER:				ELOG(" - type: GL_DEBUG_TYPE_OTHER");  break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:			ELOG(" - severity: GL_DEBUG_SEVERITY_HIGH"); break;
	case GL_DEBUG_SEVERITY_MEDIUM:			ELOG(" - severity: GL_DEBUG_SEVERITY_MEDIUM"); break;
	case GL_DEBUG_SEVERITY_LOW:				ELOG(" - severity: GL_DEBUG_SEVERITY_LOW"); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	ELOG(" - severity: GL_DEBUG_SEVERITY_NOTIFICATION"); break;
	}

}