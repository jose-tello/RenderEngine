#include "ModelStructures.h"

#include <glad/glad.h>

VertexBufferAttribute::VertexBufferAttribute(u8 location, u8 componentCount, u8 offset) :
	location(location),
	componentCount(componentCount),
	offset(offset)
{}


//Submesh---------------------------------------------------------------------------------------------------------------------
Submesh::Submesh() :
	vertexOffset(0),
	indexOffset(0)
{
}
