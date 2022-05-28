#pragma once
#include "platform.h"

struct TexObj
{
	TexObj(u32 handle, float sizeX, float sizeY, int internalFormat, int format, int type);

	u32 handle;
	float sizeX;
	float sizeY;
	int internalFormat;
	int format;
	int type;
};


struct FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	//Render fbo
	void Regenerate(float displaySizeX, float displaySizeY);
	void PushTexture(float sizeX, float sizeY, int internalFormat, int format, int type);
	void AttachTextures();

	//General fbos
	void Create();
	void PushColorAttachment(int attachment, u32 texture, int LOD);
	void ClearColorAttachments();

	void CheckStatus();

public:
	u32 handle;
	
	std::vector<TexObj> textures;
};