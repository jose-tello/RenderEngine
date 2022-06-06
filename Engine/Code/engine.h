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
struct Environment;

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
    BLOOM,
    DEPTH,
    REFLECTIVITY,
    MAX
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
    Buffer debugLightUniformBuffer;
    Buffer materialUniformBuffer;
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

    u32 lightProgramIdx;
    
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
    GLuint defaultTexture;
    GLuint bloomTexure;
    GLuint depthTexture;
    GLuint reflectivityTexture;
    GLuint drawModeUniform;

    GLuint geometryUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    u32 sphereModel;
    u32 planeModel;

    FrameBuffer framebuffer;

    //Bloom
    u32 brightPixelProgramIdx;
    u32 bloomBlurrProgramIdx;
    u32 bloomProgramIdx;

    GLuint rtBright = 0;
    GLuint rtBloom = 0;

    FrameBuffer fboBloom1;
    FrameBuffer fboBloom2;
    FrameBuffer fboBloom3;
    FrameBuffer fboBloom4;
    FrameBuffer fboBloom5;

    bool applyBloom = true;
    float bloomIntensity1 = 0.9f;
    float bloomIntensity2 = 0.9f;
    float bloomIntensity3 = 0.9f;
    float bloomIntensity4 = 0.9f;
    float bloomIntensity5 = 0.9f;

    //Skybox
    Environment* skybox = nullptr;
};


u32 CreateProgram(App* app, const char* filepath, const char* programName);
u32 LoadProgram(App* app, const char* filepath, const char* programName);

Image LoadImage(const char* filename);
void FreeImage(Image image);

u32 CreateTexture2DFromImage(Image image);

u32 LoadTexture2D(App* app, const char* filepath);

//Init-----------------------------------------------------------------
void Init(App* app);

void GetAppInfo(App* app);

void InitRect(App* app);
void InitPrograms(App* app);
void InitTextures(App* app);
void InitScene(App* app);
void InitUniformBuffers(App* app);
void InitFramebuffer(App* app);

void InitBloomResources(App* app);
void InitBloomPrograms(App* app);

void InitCubemap(App* app);

//GUI------------------------------------------------------------------
void Gui(App* app);

void DrawModeGui(App* app);
void DrawInfoGui(App* app);
void DrawModelListGui(App* app);
void DrawEntityListGui(App* app);
void DrawEntityGui(App* app);
void DrawCameraGui(App* app);
void DrawLightGui(App* app);
void DrawBloomGui(App* app);

//Update---------------------------------------------------------------
void Update(App* app);

void CheckToUpdateShaders(App* app);
void UpdateCamera(App* app);
void FillUniformLocalParams(App* app);
void FillUniformDebugLightParams(App* app);
void FillUniformMaterialParams(App* app);
void FillUniformGlobalParams(App* app);

//Render----------------------------------------------------------------
void Render(App* app);

u32 FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void RenderModels(App* app);
void DebugDrawLights(App* app);
void LightPass(App* app);
void BloomPass(App* app);
void RenderScene(App* app);

//Bloom
void BrightPixelPass(App* app);
void BlurrBloomPass(App* app);
void Blurr(App* app, FrameBuffer& fbo, int texSizeX, int texSizeY, int attachment, u32 texture, int LOD, float directionX, float directionY);
void ApplyBloomPass(App* app);

//Error callback
void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* useParam);