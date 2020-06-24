#include <GL/glew.h>		
#include <GLFW/glfw3.h> 
#include <GL/freeglut.h>

#include "stb-master/stb_image.h"
#include "glsl/core/shader_loader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void errorCallback(int errorCode, const char* errorDescription);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void renderScene(GLFWwindow* window);

int framebufferWidth, framebufferHeight;
GLuint g_VAO, g_VBO, g_EBO;
GLuint g_programID;
GLuint trianglePositionVertexBufferObjectID, triangleColorVertexBufferObjectID;
GLuint triangleTextureCoordinateBufferObjectID;

// Method to load an image into a texture using the freeimageplus library. Returns the texture ID or dies trying.
GLuint CreateTexture(char const* filename)
{
  // load image
  int width, height, channel;

  stbi_set_flip_vertically_on_load(true);

  GLubyte* textureData = stbi_load(filename, &width, &height, &channel, 0);

  // Generate a texture ID and bind to it
  GLuint tempTextureID;
  glGenTextures(1, &tempTextureID);
  glBindTexture(GL_TEXTURE_2D, tempTextureID);

  // Construct the texture.
  // Note: The 'Data format' is the format of the image data as provided by the image library. FreeImage decodes images into
  // BGR/BGRA format, but we want to work with it in the more common RGBA format, so we specify the 'Internal format' as such.
  glTexImage2D(GL_TEXTURE_2D,    // Type of texture
    0,                // Mipmap level (0 being the top level i.e. full size)
    GL_RGB,          // Internal format
    width,       // Width of the texture
    height,      // Height of the texture,
    0,                // Border in pixels
    GL_BGR,          // Data format
    GL_UNSIGNED_BYTE, // Type of texture data
    textureData);     // The image data to use for this texture

              // Specify our minification and magnification filters

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glBindTexture(GL_TEXTURE_2D, 0);

  return tempTextureID;
}

bool initShaderProgram() {

  //load and compile shaders

  //char* vertex_shader = (char*)"Shaders\\vertex_shader.glsl";
  //char* fragment_shader = (char*)"Shaders\\fragment_shader.glsl";
  //Core::Shader_Loader shaderLoader;
  //g_programID = shaderLoader.CreateProgram(vertex_shader, fragment_shader);

  //#3
  const GLchar* vertexShaderSource =
    "#version 330 core\n"
    "in vec3 positionAttribute;"
    "in vec2 textureCoordinateAttribute;"
    "out vec2 passTextureCoordinateAttribute;"
    "void main()"
    "{"
    "gl_Position = vec4(positionAttribute, 1.0);"
    "passTextureCoordinateAttribute = textureCoordinateAttribute;"
    "}";


  //#4
  const GLchar* fragmentShaderSource =
    "#version 330 core\n"
    "in vec2 passTextureCoordinateAttribute;"
    "out vec4 fragmentColor;"
    "uniform sampler2D tex;"
    "void main()"
    "{"
    "fragmentColor = texture(tex, passTextureCoordinateAttribute);"
     "}";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint result;
  GLchar errorLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
  if (!result)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
    cerr << "ERROR: vertex shader result error\n" << errorLog << endl;
    glDeleteShader(vertexShader);
    return false;
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
  if (!result)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
    cerr << "ERROR: fragment shader result error\n" << errorLog << endl;

    return false;
  }

  //#5
  g_programID = glCreateProgram();

  glAttachShader(g_programID, vertexShader);
  glAttachShader(g_programID, fragmentShader);

  glLinkProgram(g_programID);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glGetProgramiv(g_programID, GL_LINK_STATUS, &result);
  if (!result) {
    glGetProgramInfoLog(g_programID, 512, NULL, errorLog);
    cerr << "ERROR: shader program result error\n" << errorLog << endl;
    return false;
  }

  return true;
}

bool defineTextureObject() {

  //#1
  float position[] = {
    0.0f,  0.5f, 0.0f, //vertex 1  
    0.5f, -0.5f, 0.0f, //vertex 2  
    -0.5f, -0.5f, 0.0f //vertex 3  
  };

  float color[] = {
    1.0f, 0.0f, 0.0f, //vertex 1 : RED (1,0,0)
    0.0f, 1.0f, 0.0f, //vertex 2 : GREEN (0,1,0) 
    0.0f, 0.0f, 1.0f  //vertex 3 : BLUE (0,0,1)
  };

  float textureCoordinate[] = {
    0.5f, 1.0f,  //vertex 1  
    1.0f, 0.0f,  //vertex 2
    0.0f, 0.0f   //vertex 3        
  };

  //#2
  //Vertex Buffer Object(VBO)
  glGenBuffers(1, &trianglePositionVertexBufferObjectID);
  glBindBuffer(GL_ARRAY_BUFFER, trianglePositionVertexBufferObjectID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

  glGenBuffers(1, &triangleColorVertexBufferObjectID);
  glBindBuffer(GL_ARRAY_BUFFER, triangleColorVertexBufferObjectID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

  glGenBuffers(1, &triangleTextureCoordinateBufferObjectID);
  glBindBuffer(GL_ARRAY_BUFFER, triangleTextureCoordinateBufferObjectID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinate), textureCoordinate, GL_STATIC_DRAW);

  //#6
  glGenVertexArrays(1, &g_VAO);
  glBindVertexArray(g_VAO);


  GLint positionAttribute = glGetAttribLocation(g_programID, "positionAttribute");
  if (positionAttribute == -1) {
    cerr << "position attribute error" << endl;
    return false;
  }
  glBindBuffer(GL_ARRAY_BUFFER, trianglePositionVertexBufferObjectID);
  glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(positionAttribute);

  GLint textureCoordinateAttribute = glGetAttribLocation(g_programID, "textureCoordinateAttribute");
  if (textureCoordinateAttribute == -1) {
    cerr << "Texture Coordinate attribute error" << endl;
    return false;
  }
  glBindBuffer(GL_ARRAY_BUFFER, triangleTextureCoordinateBufferObjectID);
  glVertexAttribPointer(textureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(textureCoordinateAttribute);

  glBindVertexArray(0);

  return true;
}



int main()
{
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {

    cerr << "Error: GLFW init error" << endl;
    std::exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow* window = glfwCreateWindow(
    800,
    600,
    "OpenGL Example",
    NULL, NULL);
  if (!window) {

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  glfwSetKeyCallback(window, keyCallback);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  glewExperimental = GL_TRUE;
  GLenum errorCode = glewInit();
  if (GLEW_OK != errorCode) {

    cerr << "Error: GLEW init error" << glewGetErrorString(errorCode) << endl;

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  if (!GLEW_VERSION_3_3) {

    cerr << "Error: OpenGL 3.3 API is not available." << endl;

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
  cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
  cout << "Renderer: " << glGetString(GL_RENDERER) << endl;

  if (!initShaderProgram()) {

    cerr << "Error: Shader Program init error" << endl;

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  if (!defineTextureObject()) {

    cerr << "Error: Shader Program define defineTextureObject error" << endl;

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glfwSwapInterval(1);

  glUseProgram(g_programID);
  glBindVertexArray(g_VAO);

  GLuint texureId = CreateTexture("C:/data/test.jpg");

  glUniform1i(glGetUniformLocation(g_programID, "tex"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texureId);

  while (!glfwWindowShouldClose(window)) {
    renderScene(window);
  }

  glUseProgram(0);
  glBindVertexArray(0);

  glDeleteProgram(g_programID);
  glDeleteBuffers(1, &trianglePositionVertexBufferObjectID);
  glDeleteBuffers(1, &triangleColorVertexBufferObjectID);
  glDeleteBuffers(1, &triangleTextureCoordinateBufferObjectID);
  glDeleteVertexArrays(1, &g_VAO);
  glfwTerminate();

  std::exit(EXIT_SUCCESS);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);

  framebufferWidth = width;
  framebufferHeight = height;
}

void errorCallback(int errorCode, const char* errorDescription)
{
  cerr << "Error: " << errorDescription << endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void renderScene(GLFWwindow* window)
{
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glfwSwapBuffers(window);
  glfwPollEvents();
}