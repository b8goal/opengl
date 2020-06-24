#include <GL/glew.h>		
#include <GLFW/glfw3.h> 
#include <GL/freeglut.h>
#include "stb-master/stb_image.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;


int framebufferWidth, framebufferHeight;
GLuint triangleVertexArrayObject;
GLuint triangleShaderProgramID;
GLuint trianglePositionVertexBufferObjectID, triangleColorVertexBufferObjectID;
GLuint triangleTextureCoordinateBufferObjectID;

//CreateTexture
//https://r3dux.org/2014/10/how-to-load-an-opengl-texture-using-the-freeimage-library-or-freeimageplus-technically/
//
// Method to load an image into a texture using the freeimageplus library. Returns the texture ID or dies trying.
GLuint CreateTexture(char const* filename)
{
  // load image
  int width, height, channel;

  stbi_set_flip_vertically_on_load(true);

  GLubyte* textureData = stbi_load("C:/data/test.jpg", &width, &height, &channel, 0);

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
  triangleShaderProgramID = glCreateProgram();

  glAttachShader(triangleShaderProgramID, vertexShader);
  glAttachShader(triangleShaderProgramID, fragmentShader);

  glLinkProgram(triangleShaderProgramID);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glGetProgramiv(triangleShaderProgramID, GL_LINK_STATUS, &result);
  if (!result) {
    glGetProgramInfoLog(triangleShaderProgramID, 512, NULL, errorLog);
    cerr << "ERROR: shader program result error\n" << errorLog << endl;
    return false;
  }

  return true;
}

bool defineVertexArrayObject() {

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
  glGenVertexArrays(1, &triangleVertexArrayObject);
  glBindVertexArray(triangleVertexArrayObject);


  GLint positionAttribute = glGetAttribLocation(triangleShaderProgramID, "positionAttribute");
  if (positionAttribute == -1) {
    cerr << "position attribute error" << endl;
    return false;
  }
  glBindBuffer(GL_ARRAY_BUFFER, trianglePositionVertexBufferObjectID);
  glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(positionAttribute);

  GLint textureCoordinateAttribute = glGetAttribLocation(triangleShaderProgramID, "textureCoordinateAttribute");
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

  if (!defineVertexArrayObject()) {

    cerr << "Error: Shader Program define defineVertexArrayObject error" << endl;

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glfwSwapInterval(1);

  double lastTime = glfwGetTime();
  int numOfFrames = 0;
  int count = 0;

  glUseProgram(triangleShaderProgramID);
  glBindVertexArray(triangleVertexArrayObject);

  GLuint texureId = CreateTexture("number-board.jpg");

  glUniform1i(glGetUniformLocation(triangleShaderProgramID, "tex"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texureId);

  while (!glfwWindowShouldClose(window)) {

    double currentTime = glfwGetTime();
    numOfFrames++;
    if (currentTime - lastTime >= 1.0) {

      printf("%f ms/frame  %d fps \n", 1000.0 / double(numOfFrames), numOfFrames);
      numOfFrames = 0;
      lastTime = currentTime;
    }

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    count++;

    glfwSwapBuffers(window);
    glfwPollEvents();

  }

  glUseProgram(0);
  glBindVertexArray(0);

  glDeleteProgram(triangleShaderProgramID);
  glDeleteBuffers(1, &trianglePositionVertexBufferObjectID);
  glDeleteBuffers(1, &triangleColorVertexBufferObjectID);
  glDeleteBuffers(1, &triangleTextureCoordinateBufferObjectID);
  glDeleteVertexArrays(1, &triangleVertexArrayObject);
  glfwTerminate();

  std::exit(EXIT_SUCCESS);
}
