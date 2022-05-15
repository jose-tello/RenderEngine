//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"

#include "ModelStructures.h"
#include "Light.h"
#include "Camera.h"
#include "BufferManagement.h"
#include "FrameBuffer.h"

#include <glad/glad.h>

#define MAX_GO_NAME_LENGTH 100

struct Light;

const float rectVertices[] = {-1.0, -1.0, 0.0, 0.0, 0.0,
                           1.0, -1.0, 0.0, 1.0, 0.0,
                           1.0,  1.0, 0.0, 1.0, 1.0,
                          -1.0,  1.0, 0.0, 0.0, 1.0};

const unsigned int rectIndices[] = {0, 1, 2,
                                0, 2, 3};

enum class DRAW_MODE : int
{
    DEFAULT = 0,
    ALBEDO,
    NORMALS,
    WORLD_POS,
    DEPTH,
    MAX
};

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
    DRAW_MODE drawMode = DRAW_MODE::DEFAULT;
    bool debugDrawLights = true;

    Camera camera;

    u32 entityIdCount = 0;

    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics

    glm::ivec2 displaySize;
    Buffer localUniformBuffer;
    Buffer globalUniformBuffer;

    int globalParamsOffset = -1;
    int globalParamsSize = -1;

    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;

    std::vector<Entity> entities;
    std::vector<Light> lights;

    std::vector<Program>  programs;

    //Ambient light
    float ambientLightStrength = 0.05;
    glm::vec3 ambientLightColor = {0.95, 0.8, 0.8};

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

    // Location of texture uniforms
    GLuint albedoTexture;
    GLuint normalsTexture;
    GLuint worldPositionTexture;
    GLuint depthTexture;
    GLuint drawModeUniform;

    GLuint geometryUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    u32 sphereModel;
    u32 planeModel;

    FrameBuffer framebuffer;
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

void DrawModeGui(App* app);
void DrawInfoGui(App* app);
void DrawModelListGui(App* app);
void DrawEntityListGui(App* app);
void DrawEntityGui(App* app);
void DrawCameraGui(App* app);
void DrawLightGui(App* app);

//Update---------------------------------------------------------------
void Update(App* app);

void CheckToUpdateShaders(App* app);
void UpdateCamera(App* app);
void FillUniformLocalParams(App* app);
void FillUniformGlobalParams(App* app);

//Render----------------------------------------------------------------
void Render(App* app);

u32 FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void RenderModels(App* app);
void DebugDrawLights(App* app);
void RenderScene(App* app);

//Error callback
void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* useParam);