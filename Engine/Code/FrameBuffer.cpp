#include "FrameBuffer.h"
#include "glad/glad.h"

TexObj::TexObj(u32 handle, float sizeX, float sizeY, int internalFormat, int format, int type) :
	handle(handle),
	sizeX(sizeX),
	sizeY(sizeY),
	internalFormat(internalFormat),
	format(format),
	type(type)
{
}

FrameBuffer::FrameBuffer() : 
	handle(0u)
{
}


FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &handle);

	int textureCount = textures.size();
	for (int i = 0; i < textureCount; ++i)
	{
		glDeleteTextures(1, &textures[0].handle);
	}
}


void FrameBuffer::Regenerate(float displaySizeX, float displaySizeY)
{
	glDeleteFramebuffers(1, &handle);

	int textureCount = textures.size();
	for (int i = 0; i < textureCount; ++i)
	{
		PushTexture(displaySizeX, displaySizeY, textures[0].internalFormat, textures[0].format, textures[0].type);

		glDeleteTextures(1, &textures[0].handle);
		textures.erase(textures.begin());
	}

	AttachTextures();

	CheckStatus();
}


void FrameBuffer::PushTexture(float sizeX, float sizeY, int internalFormat, int format, int type)
{
	u32 texHandle;
	glGenTextures(1, &texHandle);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, sizeX, sizeY, 0, format, type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	textures.push_back(TexObj(texHandle, sizeX, sizeY, internalFormat, format, type));
}


void FrameBuffer::AttachTextures()
{
	glDeleteFramebuffers(1, &handle);

	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	
	int textureCount = textures.size();
	for (int i = 0; i < textureCount; ++i)
	{
		if (textures[i].format == GL_DEPTH_COMPONENT)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textures[i].handle, 0);
		}
		else
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textures[i].handle, 0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CheckStatus();
}


void FrameBuffer::CheckStatus()
{
	glBindFramebuffer(GL_FRAMEBUFFER, handle);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (status)
		{
		case GL_FRAMEBUFFER_UNDEFINED:						ELOG("FRAMEBUFFER UNDEFINED"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			ELOG("FRAMEBUFFER INCOMPLETE ATTACHMENT"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	ELOG("FRAMEBUFFER INCOMPLETE MISSING ATTACHMENT"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			ELOG("FRAMEBUFFER INCOMPLETE DRAW BUFFER"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			ELOG("FRAMEBUFFER INCOMPLETE READ BUFFER"); break;
		case GL_FRAMEBUFFER_UNSUPPORTED:					ELOG("FRAMEBUFFER UNSUPORTED"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			ELOG("FRAMEBUFFER INCOMPLETE MULTISAMPLE"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:		ELOG("FRAMEBUFFER INCOMPLETE LAYER TARGETS"); break;

		default:
			ELOG("FRAMEBUFFER UNKNOWN ERROR");
			break;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
