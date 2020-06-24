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

GLuint CreateTexture(char const* filename);
bool initShaderProgram();
bool defineTextureObject();

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void errorCallback(int errorCode, const char* errorDescription);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void renderScene(GLFWwindow* window);

int framebufferWidth, framebufferHeight;
GLuint g_VAO, g_VBO, g_EBO;
GLuint g_shaderProgramID;

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
    1280,
    720,
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

  if (!defineTextureObject()) {

    cerr << "Error: Shader Program define defineTextureObject error" << endl;

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  if (!initShaderProgram()) {

    cerr << "Error: Shader Program init error" << endl;

    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glfwSwapInterval(1);

  GLuint texureId = CreateTexture("C:/data/test.jpg");
  
  while (!glfwWindowShouldClose(window)) {
    renderScene(window);
  }

  glUseProgram(0);
  glBindVertexArray(0);

  glDeleteTextures(1, &texureId);
  glDeleteProgram(g_shaderProgramID);
  glDeleteBuffers(1, &g_EBO);
  glDeleteBuffers(1, &g_VBO);
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

GLuint CreateTexture(char const* filename)
{
  // load image
  int width, height, channel;

  stbi_set_flip_vertically_on_load(true);

  GLubyte* textureData = stbi_load(filename, &width, &height, &channel, STBI_rgb);

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
    GL_RGB,          // Data format
    GL_UNSIGNED_BYTE, // Type of texture data
    textureData);     // The image data to use for this texture

              // Specify our minification and magnification filters

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(textureData);

  return tempTextureID;
}

void renderScene(GLFWwindow* window)
{
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glfwSwapBuffers(window);
  glfwPollEvents();
}

bool initShaderProgram() {

  //load and compile shaders

  //char* vertex_shader = (char*)"Shaders\\vertex_shader.glsl";
  //char* fragment_shader = (char*)"Shaders\\fragment_shader.glsl";
  //Core::Shader_Loader shaderLoader;
  //g_shaderProgramID = shaderLoader.CreateProgram(vertex_shader, fragment_shader);

  const GLchar* vertexShaderSource =
    "#version 330 core\n"
    "in vec3 aPos;"
    "in vec3 aColor;"
    "in vec2 aTexCoord;"
    "out vec3 Color;"
    "out vec2 TexCoord;"
    "void main()"
    "{"
    "gl_Position = vec4(aPos, 1.0F);"
    "Color = aColor;"
    "TexCoord = aTexCoord;"
    "}";

  const GLchar* fragmentShaderSource =
    "#version 330 core\n"
    "in vec3 Color;"
    "in vec2 TexCoord;"
    "out vec4 outColor;"
    "uniform sampler2D ourTexture;"
    "void main()"
    "{"
    //"outColor = texture(tex, TexCoord) * vec4(Color, 1.0);" 
    "outColor = texture(ourTexture, TexCoord) * vec4(Color, 1.0);"
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

  // Link the vertex and fragment shader into a shader program
  g_shaderProgramID = glCreateProgram();
  glAttachShader(g_shaderProgramID, vertexShader);
  glAttachShader(g_shaderProgramID, fragmentShader);
  glBindFragDataLocation(g_shaderProgramID, 0, "outColor");
  glLinkProgram(g_shaderProgramID);
  glUseProgram(g_shaderProgramID);

  // specify the layout of the vertex data
  GLint posAttrib = glGetAttribLocation(g_shaderProgramID, "aPos");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
  if (posAttrib == -1) {
    cerr << "position attribute error" << endl;
    return false;
  }

  GLint colAttrib = glGetAttribLocation(g_shaderProgramID, "aColor");
  glEnableVertexAttribArray(colAttrib);
  glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
  if (colAttrib == -1) {
    cerr << "position attribute error" << endl;
    return false;
  }

  GLint texAttrib = glGetAttribLocation(g_shaderProgramID, "aTexCoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
  if (texAttrib == -1) {
    cerr << "position attribute error" << endl;
    return false;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glGetProgramiv(g_shaderProgramID, GL_LINK_STATUS, &result);
  if (!result) {
    glGetProgramInfoLog(g_shaderProgramID, 512, NULL, errorLog);
    cerr << "ERROR: shader program result error\n" << errorLog << endl;
    return false;
  }

  return true;
}

bool defineTextureObject() {

  float vertices[] = {
    // positions          // colors           // texture coords
      0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // top right
      0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // top left 
  };

  unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };

  glGenVertexArrays(1, &g_VAO);
  glGenBuffers(1, &g_VBO);
  glGenBuffers(1, &g_EBO);

  glBindVertexArray(g_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  return true;
}