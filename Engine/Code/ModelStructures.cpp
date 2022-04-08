#include "ModelStructures.h"

#include <glad/glad.h>

VertexBufferAttribute::VertexBufferAttribute(u8 location, u8 componentCount, u8 offset) :
	location(location),
	componentCount(componentCount),
	offset(offset)
{}


VertexShaderAttribute::VertexShaderAttribute(u8 location, u8 componentCount) :
	location(location),
	componentCount(componentCount)
{}


Vao::Vao(u32 handle, u32 programHandle) :
	handle(handle),
	programHandle(programHandle)
{}


//Entity----------------------------------------------------------------------------------------------------------------------

Entity::Entity(std::string name, u32 modelIdx) :
	name(name),
	
	position(0.f, 0.f, 0.f),
	rotation(0.f, 0.f, 0.f),
	scale(1.f, 1.f, 1.f),

	modelIdx(modelIdx),
	localParamsOffset(0),
	localParamsSize(0),

	drawInspector(false)
{
}


glm::mat4 Entity::CalculateWorldTransform() const
{
	glm::mat4 transform = glm::translate(position);
	transform = glm::rotate(transform, rotation.x, glm::vec3(1.f, 0.f, 0.f));
	transform = glm::rotate(transform, rotation.y, glm::vec3(0.f, 1.f, 0.f));
	transform = glm::rotate(transform, rotation.z, glm::vec3(0.f, 0.f, 1.f));
	transform = glm::scale(transform, scale);

	return transform;
}


//Submesh---------------------------------------------------------------------------------------------------------------------
Submesh::Submesh() :
	vertexOffset(0),
	indexOffset(0)
{}
