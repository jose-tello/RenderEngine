//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"

#include "ModelStructures.h"
#include "Camera.h"

#include <glad/glad.h>

#define MAX_GO_NAME_LENGTH 100

const float rectVertices[] = {-1.0, -1.0, 0.0, 0.0, 0.0,
                           1.0, -1.0, 0.0, 1.0, 0.0,
                           1.0,  1.0, 0.0, 1.0, 1.0,
                          -1.0,  1.0, 0.0, 0.0, 1.0};

const unsigned int rectIndices[] = {0, 1, 2,
                                0, 2, 3};

struct Image
{
    void* pixels;
    glm::ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?

    VertexShaderLayout layout;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Model,
    Mode_Count
};

struct OpenGLInfo
{
    std::string version;
    std::string render;
    std::string vendor;
    std::string shadingLanguageVersion;
    std::vector<std::string> extensions;
};

struct App
{
    OpenGLInfo info;

    Camera camera;

    u32 entityIdCount = 0;

    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics

    glm::ivec2 displaySize;
    u32 uniformBufferHandle;
    int uniformAlignment;

    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;
    std::vector<Entity> entities;
    std::vector<Program>  programs;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 screenRectProgramIdx;
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint rectUniformTexture;
    GLuint geometryUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;
};

u32 CreateProgram(App* app, const char* filepath, const char* programName);
u32 LoadProgram(App* app, const char* filepath, const char* programName);


GLuint CreateTexture2DFromImage(Image image);
u32 LoadTexture2D(App* app, const char* filepath);

//Init-----------------------------------------------------------------
void Init(App* app);

void GetAppInfo(App* app);
void InitResources(App* app);

//GUI------------------------------------------------------------------
void Gui(App* app);

void DrawInfoGui(App* app);
void DrawModelListGui(App* app);
void DrawEntityListGui(App* app);
void DrawEntityGui(App* app);
void DrawCameraGui(App* app);

//Update---------------------------------------------------------------
void Update(App* app);

void CheckToUpdateShaders(App* app);
void UpdateCamera(App* app);
void FillUniformShader(App* app);

//Render----------------------------------------------------------------
void Render(App* app);

u32 FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void RenderTexturedQuad(App* app);
void RenderModels(App* app);

//Error callback
void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* useParam);