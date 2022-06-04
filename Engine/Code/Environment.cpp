#include "Environment.h"

#include "engine.h"

#include <stb_image.h>
#include <stb_image_write.h>

Environment::Environment(App* app, const char* cubeMapPath)
{
	InitCubeVAO();
	InitHdrTexture(cubeMapPath);
	InitCubemapBuffers();
	InitCubemap();

	u32 programIdx = CreateProgram(app, "Skybox.glsl", "SKYBOX");
	skyBoxProgram = app->programs[programIdx];

	programIdx = CreateProgram(app, "hdrToCubemap.glsl", "HDR_TO_CUBEMAP");
	Program program = app->programs[programIdx];

	//Generate cubeMap
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};


	glUseProgram(program.handle);
	u32 uniformLoc = glGetUniformLocation(program.handle, "hdrMap");
	glUniform1i(uniformLoc, 0);

	uniformLoc = glGetUniformLocation(program.handle, "uProjection");
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(captureProjection));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture.handle);

	glViewport(0, 0, 512, 512);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (int i = 0; i < 6; ++i)
	{
		uniformLoc = glGetUniformLocation(program.handle, "uView");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(captureViews[i]));

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMap.handle, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCube();
	}

	/*glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.handle);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);*/

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}


Environment::~Environment()
{

}


void Environment::RenderSkybox(App* app)
{
	glBindFramebuffer(GL_FRAMEBUFFER, app->framebuffer.handle);

	u32 drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glDepthFunc(GL_LEQUAL);

	// - set the viewport
	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// - bind program
	glUseProgram(skyBoxProgram.handle);

	u32 uniformLoc = glGetUniformLocation(skyBoxProgram.handle, "skyBox");
	glUniform1i(uniformLoc, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.handle);

	uniformLoc = glGetUniformLocation(skyBoxProgram.handle, "uProjection");
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(app->camera.GetProjectionMatrix()));

	uniformLoc = glGetUniformLocation(skyBoxProgram.handle, "uView");
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(app->camera.GetViewMatrix()));

	RenderCube();

	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
}


void Environment::InitCubeVAO()
{
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void Environment::RenderCube()
{
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


void Environment::InitHdrTexture(const char* cubeMapPath)
{
	Image img = {};
	stbi_set_flip_vertically_on_load(true);
	img.pixels = stbi_loadf(cubeMapPath, &img.size.x, &img.size.y, &img.nchannels, 0);
	if (img.pixels)
	{
		img.stride = img.size.x * img.nchannels;
	}

	//Hdr
	glGenTextures(1, &hdrTexture.handle);
	glBindTexture(GL_TEXTURE_2D, hdrTexture.handle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, img.size.x, img.size.y, 0, GL_RGB, GL_FLOAT, img.pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//stbi_image_free(img.pixels);
}


void Environment::InitCubemapBuffers()
{
	//Capture FBO & RBO
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Environment::InitCubemap()
{
	//Cubemap
	glGenTextures(1, &cubeMap.handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.handle);

	for (u32 i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}