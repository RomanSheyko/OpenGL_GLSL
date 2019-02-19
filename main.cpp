#include <iostream>
#include "SDL.h"
#include "gl_core_4_3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using glm::mat4;
using glm::vec3;

const int width = 640;
const int height = 480;
const int FRAMES_PER_SECOND = 60;

SDL_Window *window;

SDL_GLContext maincontext;
GLuint vaoHandle;
GLuint programHandle;

bool SetOpenGLAttributes();
void PrintSDL_GL_Attributes();
void CheckSDLError(int line);
void printVersion();
void Run();
void Cleanup();
GLuint loadShader(const char *path, GLenum shader_type);
GLchar* loadShaderAsString(const char *path);
void makeShape();


bool Init()
{
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ){
        std::cout << "Unable to init SDL, error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if(window == NULL)
    {
        std::cout << "Unable to create window\n";
        CheckSDLError(__LINE__);
        return false;
    }

    SDL_GLContext maincontext = SDL_GL_CreateContext(window);

    int loaded = ogl_LoadFunctions();
    if(loaded == ogl_LOAD_FAILED) {
        std::cout << "OpenGL func loading failed" << std::endl;
        return false;
    }

    int num_failed = loaded - ogl_LOAD_SUCCEEDED;
    std::cout << "Number of functions that failed to load: " << num_failed << std::endl;

    SetOpenGLAttributes();

    SDL_GL_SetSwapInterval(1);

    GLuint vertShader = loadShader("../basic.vert", GL_VERTEX_SHADER);
    GLuint fragShader = loadShader("../base.frag", GL_FRAGMENT_SHADER);

    programHandle = glCreateProgram();
    if(programHandle == 0)
    {
        fprintf(stderr, "Error creating program object.\n");
        exit(1);
    }

    glAttachShader(programHandle, vertShader);
    glAttachShader(programHandle, fragShader);

    //Linking
    glLinkProgram(programHandle);

    //Checking
    GLint status;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    if(status ==GL_FALSE)
    {
        fprintf(stderr, "Failed to link shader program!\n");

        GLint logLen;
        glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLen);
        if(logLen > 0)
        {
            char *log = new char[logLen];
            GLsizei written;
            glGetProgramInfoLog(programHandle, logLen, &written, log);
            fprintf(stderr, "Program log: \n%s", log);
            delete [] log;
        }
    }
    else {
        glUseProgram(programHandle);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    return true;
}

void makeShape()
{
    float positionData[] = {
            -0.8f, -0.8f, 0.0f,
            0.8f, -0.8f, 0.0f,
            -0.8f, 0.8f, 0.0f,

            0.8f, 0.8f, 0.0f,
            0.8f, -0.8f, 0.0f,
            -0.8f, 0.8f, 0.0f
    };

    GLuint vboHandles;
    glGenBuffers(1, &vboHandles);
    GLuint positionBufferHandle = vboHandles;

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), positionData, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

bool SetOpenGLAttributes()
{
    // Set our OpenGL version.
    // SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /*
    // 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    */

    // Turn on double buffering with a 24bit Z buffer.
    // You may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    return true;
}

int eventListener(SDL_Event e)
{
    switch(e.type)
    {
        case SDL_QUIT: return 0;
        case SDL_KEYDOWN:

            break;
        case SDL_KEYUP:

            break;
    }
    return 1;
}

int main(int argc, char ** argv) {
    if (!Init())
        return -1;

    makeShape();

    printVersion();

    Run();
    return 0;
}

void Run()
{
    Uint32 start;
    char running = 1;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(running != 0)
    {
        start = SDL_GetTicks();
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if(eventListener(e) == 0) {
                running = 0;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vaoHandle);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(window);

        if(1000/FRAMES_PER_SECOND > SDL_GetTicks() - start)
            SDL_Delay(1000/FRAMES_PER_SECOND - (SDL_GetTicks() - start));
    }
}

GLchar* loadShaderAsString(const char *path)
{
    const int READ_CONST = 255;
    bool memory_flag = false;
    FILE *f = fopen(path, "r");
    char *shaderCode = new char[READ_CONST];
    int i = 0, symbol;

    while((symbol = fgetc(f)) != EOF)
    {
        if(i < READ_CONST) shaderCode[i] = (char)symbol;
        else memory_flag = true;
        i++;
    }

    if(memory_flag)
    {
        delete [] shaderCode;
        shaderCode = new char[i+1];
        i = 0;
        while((symbol = fgetc(f)) != EOF)
        {
            shaderCode[i] = (char)symbol;
            i++;
        }
    }

    shaderCode[i] = '\0';

    return (GLchar*)shaderCode;
}

GLuint loadShader(const char *path, GLenum type)
{
    GLuint shader = glCreateShader(type);
    if(shader == 0)
    {
        fprintf(stderr, "Error creating shader\n");
        exit(EXIT_FAILURE);
    }

    //Compile source
    const GLchar *shaderCode = loadShaderAsString(path);
    const GLchar *codeArray[] = {shaderCode};
    glShaderSource(shader, 1, codeArray, NULL);

    //Compile shader
    glCompileShader(shader);

    //check
    GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if(result == GL_FALSE)
    {
        fprintf(stderr, "shader compilation failed!");

        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

        if(logLen > 0)
        {
            char *log = new char[logLen];

            GLsizei written;
            glGetShaderInfoLog(shader, logLen, &written, log);

            fprintf(stderr, "Shader log:\n%s", log);
            delete [] log;
        }
    }

    delete [] shaderCode;

    return shader;
}

void CheckSDLError(int line = -1)
{
    std::string error = SDL_GetError();

    if (error != "")
    {
        std::cout << "SLD Error : " << error << std::endl;

        if (line != -1)
            std::cout << "\nLine : " << line << std::endl;

        SDL_ClearError();
    }
}

void PrintSDL_GL_Attributes()
{
    int value = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
    std::cout << "SDL_GL_CONTEXT_MAJOR_VERSION : " << value << std::endl;

    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
    std::cout << "SDL_GL_CONTEXT_MINOR_VERSION: " << value << std::endl;
}

void printVersion()
{
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    printf("GL Vendor           : %s\n", vendor);
    printf("GL Renderer         : %s\n", renderer);
    printf("GL Version (string) : %s\n", version);
    printf("GL Version (integer): %d.%d\n", major, minor);
    printf("GLSL Version        : %s\n", glslVersion);
}