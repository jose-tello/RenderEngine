#pragma once
#include "platform.h"

struct FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	void Regenerate(float displaySizeX, float displaySizeY);
	void CheckStatus();

public:
	u32 handle;
	
	u32 albedoTex;
	u32 normalsTex;
	u32 worldPosTex;
	u32 depthTex;
};