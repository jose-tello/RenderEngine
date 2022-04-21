#pragma once

#include "platform.h"

struct VertexBufferAttribute
{
	VertexBufferAttribute(u8 location, u8 componentCount, u8 offset);

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
	VertexShaderAttribute(u8 location, u8 componentCount);
	u8 location;
	u8 componentCount;
};


struct VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};


struct Vao
{
	Vao(u32 handle, u32 programHandle);
	u32 handle;
	u32 programHandle;
};


struct Entity
{
	Entity(std::string name, u32 modelIdx);
	glm::mat4 CalculateWorldTransform() const;

	std::string name;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	u32 modelIdx;
	u32 localParamsOffset;
	u32 localParamsSize;

	bool drawInspector = false;
};

struct Model
{
	u32 meshIdx;
	std::string name;
	std::vector<u32> materialIdx;
};

struct Submesh
{
	Submesh();

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