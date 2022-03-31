#pragma once

#include "platform.h"

struct VertexBufferAttribute
{
	u8 location;
	u8 componentCount;
	u8 offset;
};


struct VertexBufferLayout
{
	std::vector<VertexBufferAttribute> attributes;
	u8 stride;
};


struct VertexShaderAttribute
{
	u8 location;
	u8 componentCount;
};


struct VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};


struct Vao
{
	u32 handle;
	u32 programHandle;
};


struct Model
{
	u32 meshIdx;
	std::vector<u32> materialIdx;
};

struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<float> vertices;
	std::vector<u32> indices;
	u32 vertexOffset;
	u32 indexOffset;

	std::vector<Vao> vaos;
};

struct Mesh
{
	std::vector<Submesh> submeshes;
	u32 vertexBufferHandle;
	u32 indexBufferHandle;
};


struct Material
{
	std::string name;
	glm::vec3 albedo;
	glm::vec3 emissive;

	float smoothness;
	u32 albedoTextureIdx;
	u32 emissiveTextureIdx;
	u32 specularTextureIdx;
	u32 normalsTextureIdx;
	u32 bumpTextureIdx;
};