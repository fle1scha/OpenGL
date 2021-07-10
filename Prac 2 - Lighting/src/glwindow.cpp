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

#include "stb_image.h"

using namespace std;

//global transformation values;
double scale = 1.0f;
double x = 0.0f;
double y = 0.0f;
double z = 0.0f;
double x_angle = 0.0f;
double y_angle = 0.0f;
double z_angle = 0.0f;
double scale_factor = 1.0f;

//global Vertex Array Object ints for render().
GLuint vao_1;
GLuint vbo;
GLuint vao_2;
GLuint texture1;
GLuint texture2;

//global floats for uniform RGB adjustments.
float rgb[3] = {1.0f, 1.0f, 1.0f};
float rgb_c[3] = {1.0f, 1.0f, 1.0f};
int colorLoc;
int lightLoc;
float x1 = 1.0f;
float x2 = 0.0f;

//global ViewProjection matrix.
glm::mat4 Projection;

int bump = 0;

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

    //Load shader
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    //Object color uniform
    colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

    //Ambient color uniform
    lightLoc = glGetUniformLocation(shader, "lightColor");
    glUniform3f(lightLoc,1.0f, 1.0f, 1.0f);
    
    //Light color 1 uniform
    GLuint lightLoc1 = glGetUniformLocation(shader, "lightColor1");
    glUniform3f(lightLoc1,0.3f, 1.0f, 1.0f);

    //Light color 2 uniform
    GLuint lightLoc2 = glGetUniformLocation(shader, "lightColor2");
    glUniform3f(lightLoc2,1.0f, 1.0f, 0.3f);

    //First object:
    glGenVertexArrays(1, &vao_1); //VAO for object 1.
    glBindVertexArray(vao_1);
    GeometryData geometry1;
    geometry1.loadFromOBJFile("../objects/suzanne.obj");

    GLuint vbo;
    glGenBuffers(1, &vbo); //VBO for object 1.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, geometry1.vertexCount() * 3 * sizeof(float), geometry1.vertexData(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT, 
        GL_FALSE,
        0,
        0
    );

    //Load object normal data
    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, geometry1.vertexCount() * 3 * sizeof(float), geometry1.normalData(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT, 
        GL_FALSE,
        0,
        0
    );

    //Load texture image
    int width, height, nrChannels;
    unsigned char *data = stbi_load("brick.jpg",&width, &height,&nrChannels, 0);

    //Create texture
    GLuint textureBuffer;
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, geometry1.vertexCount() * 3 * sizeof(float), geometry1.textureCoordData(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT, 
        GL_FALSE,
        0,
        0
    );

    texture1;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    //Set tangent data
    GLuint tangentbuffer;
    glGenBuffers(1, &tangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, geometry1.vertexCount() * 3 * sizeof(float), geometry1.tangentData(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
        glVertexAttribPointer(
            3,                                
            3,                                
            GL_FLOAT,                         
            GL_FALSE,                         
            0,                                
            0                          
        );


    //Set bitangent data
    GLuint bitangentbuffer;
    glGenBuffers(1, &bitangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, geometry1.vertexCount() * 3 * sizeof(float), geometry1.bitangentData(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
        glVertexAttribPointer(
            4,                               
            3,                                
            GL_FLOAT,                         
            GL_FALSE,                         
            0,                                
            0                          
        );

    //Bump mapping texture
    int n_width, n_height, n_nrChannels;
    unsigned char *normaldata = stbi_load("bump.jpg",&n_width, &n_height,&n_nrChannels, 0);

    texture2;
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, n_width, n_height, 0, GL_RGB, GL_UNSIGNED_BYTE, normaldata);
    glGenerateMipmap(GL_TEXTURE_2D);

    /*
    Projection matrix:
    - Projection matrix maps coordinates from View Space to Homogenous Coordinates
    */
    Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f); // arguments are fov, aspect ratio, front clipping plane and back clipping plane.

    glPrintError("Setup complete", true);
}

void OpenGLWindow::render()
{   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bool quit = false;

    //Automatic light movement
    x1 = x1+0.03;
    x2 = x2+0.015;     

    GLuint light1ID = glGetUniformLocation(shader, "alightPos");
    glUniform3f(light1ID, 4.2f*sin(x2),2.0f,4.0f*cos(x2));

    GLuint light2ID = glGetUniformLocation(shader, "blightPos");
    glUniform3f(light2ID, 3.0f*sin(x2),-3.0f,-3.5f*cos(x2));

    //Bump mapping on or off
    GLuint bumpLoc = glGetUniformLocation(shader, "bump");
    glUniform1i(bumpLoc, bump);

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
    
     /*
    View:
    - positioned at 0, 0, 3 (on the z-axis)
    - looking at the origin down the z-axis (negative)
    - upward is defined by (0, 1,0)
    */
    
    glm::mat4 View = glm::lookAt(glm::vec3(0, 0, 7), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    View = View*Rotate; //COMMENT THIS LINE and UNCOMMENT LINE 378 FOR OBJECT ROTATION

    /*
   ViewProjection:
    - applied in 'reverse' because matrix multiplication is applied in this manner.
    */

    glm::mat4 VP = Projection * View;

    glm::mat4 RotTrans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)); //COMMENT THIS LINE and UNCOMMENT LINE 378 FOR OBJECT ROTATION
    //glm::mat4 RotTrans = glm::translate(Rotate, glm::vec3(x, y, z));
    glm::mat4 RotTransScale = glm::scale(RotTrans, glm::vec3(scale_factor, scale_factor, scale_factor));
    glm::mat4 Model = RotTransScale;
    glm::mat4 MVP = VP * Model;

    glm::mat4 MV = View * Model;

    GLuint MVID = glGetUniformLocation(shader, "MV");
    glUniformMatrix4fv(MVID, 1, GL_FALSE, &MV[0][0]);

    GLuint ModelID = glGetUniformLocation(shader, "Model");
    glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);

    GLuint ViewID = glGetUniformLocation(shader, "View");
    glUniformMatrix4fv(ViewID, 1, GL_FALSE, &View[0][0]);

    //Pass MVP to vertex shader and draw by binding Object 1's VAO.
    GLuint MatrixID = glGetUniformLocation(shader, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform3f(colorLoc, rgb[0], rgb[1], rgb[2]); 
    
    int texLoc = glGetUniformLocation(shader, "myTexture");
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    int normtexLoc = glGetUniformLocation(shader, "normalTexture");
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
        
    glUniform1i(normtexLoc, 1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glDrawArrays(GL_TRIANGLES, 0, 2904);

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
        else if (e.key.keysym.sym == SDLK_RIGHT){x++;} //Positive X Translation
        else if (e.key.keysym.sym == SDLK_LEFT){x--;} //Negative X Translation
        else if (e.key.keysym.sym == SDLK_UP){y++;} //Positive Y Translation
        else if (e.key.keysym.sym == SDLK_DOWN){y--;} //Negative Y Translation
        else if (e.key.keysym.sym == SDLK_BACKSLASH){z++;} //Positive Z Translation(towards View)
        else if (e.key.keysym.sym == SDLK_BACKSPACE){z--;} //Negative Z Translation (away from View)
        else if (e.key.keysym.sym == SDLK_x){x_angle += 0.2617994;} //X-axis Rotation of 15 degrees = 0.2617994 radians 
        else if (e.key.keysym.sym == SDLK_y){y_angle += 0.2617994;} //Y-axis Rotation
        else if (e.key.keysym.sym == SDLK_z){z_angle += 0.2617994;} //Z-axis Rotation
        else if (e.key.keysym.sym == SDLK_1) //Colour change object 1
        {
            rgb[0] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            rgb[1] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            rgb[2] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }
        else if (e.key.keysym.sym == SDLK_b) //Bump mapping
        {
            if (bump == 0){bump = 1;}
            else if (bump == 1){bump = 0;}
        }
        
        render();
    }
    return true;
}
void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao_1);
    SDL_DestroyWindow(sdlWin);
}
