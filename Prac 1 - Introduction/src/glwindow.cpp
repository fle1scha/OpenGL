#include <iostream>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

//Scaffold code header files
#include "glwindow.h"
#include "geometry.h"

//GLM libraries
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace std;

//global transformation values;
double scale = 1.0f;
double x = 0.0f;
double y = 0.0f;
double z = 0.0f;
double x_angle = 0.0f;
double y_angle = 0.0f;
double z_angle = 0.0f;
//global Vertex Array Object ints for render().
GLuint vao_1;
GLuint vao_2;
//global variables for geometry1 boundaries.
float maxX;
float maxY;
float minZ;
//global floats for uniform RGB adjustments.
float rgb[3] = {1.0f, 1.0f, 1.0f};
float rgb_c[3] = {1.0f, 1.0f, 1.0f};
int colorLoc;
//global ViewProjection matrix.
glm::mat4 VP;

const char *glGetErrorString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char *label = "Unlabelled Error Checkpoint", bool alwaysPrint = false)
{
    GLenum error = glGetError();
    if (alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char *shaderFilename, GLenum shaderType)
{
    FILE *shaderFile = fopen(shaderFilename, "r");
    if (!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char *shaderText = new char[shaderSize + 1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char **)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char *vertShaderFilename,
                         const char *fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow()
{
}

void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 960, SDL_WINDOW_OPENGL);
    if (!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if (glewInitResult != GLEW_OK)
    {
        const GLubyte *errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0, 0, 0, 1);

    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

    int vertexLoc = glGetAttribLocation(shader, "position");

    //First object:
    glGenVertexArrays(1, &vao_1); //VAO for object 1.
    glBindVertexArray(vao_1);

    GeometryData geometry1;
    geometry1.loadFromOBJFile("../objects/doggo.obj");

    GLuint vbo;
    glGenBuffers(1, &vbo); //VBO for object 1.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, geometry1.vertexCount() * 3 * sizeof(float), geometry1.vertexData(), GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    //Compute bounding box of first object using max methods.
    maxX = geometry1.maxX();
    maxY = geometry1.maxY();
    minZ = geometry1.minZ();
    //GEOMETRY 2
    glGenVertexArrays(1, &vao_2);
    glBindVertexArray(vao_2);

    // Load the model that we want to use and buffer the vertex attributes
    GeometryData geometry2;
    geometry2.loadFromOBJFile("../objects/doggo.obj");

    GLuint vbo_2;
    glGenBuffers(1, &vbo_2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_2);
    glBufferData(GL_ARRAY_BUFFER, geometry2.vertexCount() * 3 * sizeof(float), geometry2.vertexData(), GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    /*
    Projection matrix:
    - Projection matrix maps coordinates from Camera Space to Homogenous Coordinates
    */
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f); // arguments are fov, aspect ratio, front clipping plane and back clipping plane.

    /*
    Camera:
    - positioned at 0, 0, 3 (on the z-axis)
    - looking at the origin down the z-axis (negative)
    - upward is defined by (0, 1,0)
    */
    glm::mat4 Camera = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    /*
   ViewProjection:
    - applied in 'reverse' because matrix multiplication is applied in this manner.
    */

    VP = Projection * Camera;

    glPrintError("Setup complete", true);
}

void OpenGLWindow::render(float scale_factor, float x, float y, float z, float x_angle, float y_angle, float z_angle)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bool quit = false;

    /*
    Quaternion from Euler Angles:
    - x, y and z angles are in radians.
    - because Rotate translation is defined first, this can be applied as the Identity Matrix.
    */
    glm::quat MyQuaternion;
    glm::vec3 EulerAngles(x_angle, y_angle, z_angle);
    MyQuaternion = glm::quat(EulerAngles);

    /*
    Model:
    - the model matrix maps the model coordinates to world coordinates. 
    - this is just the identify matrix because the object is centered in the world.
    - by applying the transformations to the model matrix in reverse, the transformations are realised.
    */

    // Incremental Transformations
    glm::mat4 Rotate = glm::mat4_cast(MyQuaternion);
    glm::mat4 RotTrans = glm::translate(Rotate, glm::vec3(x, y, z));
    glm::mat4 RotTransScale = glm::scale(RotTrans, glm::vec3(scale_factor, scale_factor, scale_factor));
    glm::mat4 Model = RotTransScale;
    glm::mat4 MVP = VP * Model;

    /*
    Child Model:
    - the child model is also the identity matrix, but is offset using a translation.
    - the values of the translation are defined by the bounding box of the first object.
    - min Z is used so that the second object is behind the first.
    */
    glm::mat4 Child = glm::translate(glm::mat4(1.0f), glm::vec3(maxX + 0.5, maxY + 0.5, minZ - 0.5));
    glm::mat4 MVP_child = VP * RotTransScale * Child;

    //Pass MVP to vertex shader and draw by binding Object 1's VAO.
    GLuint MatrixID = glGetUniformLocation(shader, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform3f(colorLoc, rgb[0], rgb[1], rgb[2]);
    glBindVertexArray(vao_1);
    glDrawArrays(GL_TRIANGLES, 0, 11226);

    //Pass MVP of the offset child object to the vertex shader amd draw by binding Object 2's VAO.
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_child[0][0]);
    glUniform3f(colorLoc, rgb_c[0], rgb_c[1], rgb_c[2]);
    glBindVertexArray(vao_2);
    glDrawArrays(GL_TRIANGLES, 0, 11226);

    //Apparently best practice to unbind buffers. 
    glBindVertexArray(0);

    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    if (e.type == SDL_KEYDOWN)
    {
        //Transformation Controls:
        if (e.key.keysym.sym == SDLK_ESCAPE){return false;}
        else if (e.key.keysym.sym == SDLK_PERIOD){scale *= 1.25;} //Scale Up
        else if (e.key.keysym.sym == SDLK_COMMA){scale *= 0.75;} //Scale Down
        else if (e.key.keysym.sym == SDLK_RIGHT){x++;} //Positive X Translation
        else if (e.key.keysym.sym == SDLK_LEFT){x--;} //Negative X Translation
        else if (e.key.keysym.sym == SDLK_UP){y++;} //Positive Y Translation
        else if (e.key.keysym.sym == SDLK_DOWN){y--;} //Negative Y Translation
        else if (e.key.keysym.sym == SDLK_BACKSLASH){z++;} //Positive Z Translation(towards camera)
        else if (e.key.keysym.sym == SDLK_BACKSPACE){z--;} //Negative Z Translation (away from camera)
        else if (e.key.keysym.sym == SDLK_x){x_angle += 0.2617994;} //X-axis Rotation of 15 degrees = 0.2617994 radians 
        else if (e.key.keysym.sym == SDLK_y){y_angle += 0.2617994;} //Y-axis Rotation
        else if (e.key.keysym.sym == SDLK_z){z_angle += 0.2617994;} //Z-axis Rotation
        else if (e.key.keysym.sym == SDLK_1) //Colour change object 1
        {
            rgb[0] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            rgb[1] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            rgb[2] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }
        else if (e.key.keysym.sym == SDLK_2) //Colour change object 2
        {
            rgb_c[0] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            rgb_c[1] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            rgb_c[2] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }
        render(scale, x, y, z, x_angle, y_angle, z_angle);
    }
    return true;
}
void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao_1);
    glDeleteVertexArrays(1, &vao_2);
    SDL_DestroyWindow(sdlWin);
}
