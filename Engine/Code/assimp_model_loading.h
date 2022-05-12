#pragma once

#include "ModelStructures.h"
#include "engine.h"
#include "platform.h"

struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiNode;

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

u32 LoadModel(App* app, const char* filename);

u32 LoadPlane(App* app);
u32 LoadCube(App* app);