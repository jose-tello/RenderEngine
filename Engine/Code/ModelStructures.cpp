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

//Submesh---------------------------------------------------------------------------------------------------------------------
Submesh::Submesh() :
	vertexOffset(0),
	indexOffset(0)
{}
