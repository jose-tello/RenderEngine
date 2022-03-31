//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"

#include "ModelStructures.h"

#include <glad/glad.h>

const float vertices[] = {-1.0, -1.0, 0.0, 0.0, 0.0,
                           1.0, -1.0, 0.0, 1.0, 0.0,
                           1.0,  1.0, 0.0, 1.0, 1.0,
                          -1.0,  1.0, 0.0, 0.0, 1.0};

const unsigned int indices[] = {0, 1, 2,
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
};

enum Mode
{
    Mode_TexturedQuad,
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

    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics

    glm::ivec2 displaySize;

    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector <Model> models;
    std::vector<Program>  programs;

    // program indices
    u32 texturedGeometryProgramIdx;
    
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
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;
};

//Init-----------------------------------------------------------------
void Init(App* app);

void GetAppInfo(App* app);
void InitResources(App* app);

//GUI------------------------------------------------------------------
void Gui(App* app);


//Update---------------------------------------------------------------
void Update(App* app);

void CheckToUpdateShaders(App* app);

//Render----------------------------------------------------------------
void Render(App* app);



//Error callback
void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* useParam);