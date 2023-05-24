#include "glad/gl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void glfwError(int id, const char *description) {
  fprintf(stderr, "Error: %d (%s)\n", id, description);
}

const char *readFile(const char *path) {
  struct stat st;
  char *buffer;
  FILE *fp;

  if (stat(path, &st) < 0) {
    return NULL;
  }

  fp = fopen(path, "r");

  if (!fp) {
    return NULL;
  }

  buffer = (char *)malloc(st.st_size + 1 * sizeof(char));
  fread(buffer, sizeof(char), st.st_size, fp);
  buffer[st.st_size] = '\0';

  fclose(fp);
  return buffer;
}

int checkGLLinkError(GLuint idx) {
  char log[1024];
  GLint ok;

  glGetProgramiv(idx, GL_LINK_STATUS, &ok);

  if (!ok) {

    glGetProgramInfoLog(idx, 1024, NULL, log);

    printf("Shader link failed with error: %s\n", log);

    return 0;
  }

  return 1;
}

int checkGLShaderCompileError(GLuint idx) {
  char log[1024];
  GLint compiled;

  glGetShaderiv(idx, GL_COMPILE_STATUS, &compiled);

  if (!compiled) {
    glGetShaderInfoLog(idx, 1024, NULL, log);
    printf("Shader compile failed with error: %s\n", log);
    glDeleteShader(idx);
    return 0;
  }

  return 1;
}

void setFramebufferSize(GLFWwindow *window, int width, int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

float vertices[] = {
    +1.0f, +1.0f, 0.0f, // top right
    +1.0f, -1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 0.0f, // bottom left
    -1.0f, +1.0f, 0.0f  // top left
};

unsigned int indices[] = {
    // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

float scale = 1.0f;
float x = 0.0;
float y = 0.0;
float kk = 0.10;
const int TAU = M_PI * 2.0;
void processInput(GLFWwindow *window, float fixup) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    scale += 128.0 * fixup;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    scale -= 0.40;
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    y += fixup * 8.8 / scale;
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    y -= fixup * 8.8 / scale;
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    x -= fixup * 8.8 / scale;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    x += fixup * 8.8 / scale;
}

#define MAX_ITER 2048.0f
int main(void) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwSetErrorCallback(glfwError);

  GLFWwindow *window = glfwCreateWindow(800, 600, "Wilk", NULL, NULL);
  GLuint vbo, ebo, vao, vertexShader, fragmentShader, program, width, height,
      fps, avg;
  time_t tick = time(NULL);
  double lastFrame, currentFrame, deltaFrame;
  const char *vertexShaderSource = readFile("src/shader/wilk.vert");
  const char *fragmentShaderSource = readFile("src/shader/wilk.frag");

  if (!window) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSetFramebufferSizeCallback(window, setFramebufferSize);

  puts("buffers and arrays, are you?");
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  /* VBO */
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  /* Sets location of the vertex to 0 */
  //                    POS LEN TYPE    NORMALIZE STRIDE            POINTER:
  //                                                                Position
  //                                                                 data
  //                                                                begins
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  /* EBO */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  puts("Building shaders");
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  if (!checkGLShaderCompileError(vertexShader)) {
    return 1;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  if (!checkGLShaderCompileError(fragmentShader)) {
    return 1;
  }

  puts("Programming shaders!");
  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  if (!checkGLLinkError(program)) {
    return 1;
  }

  free((void *)vertexShaderSource);
  free((void *)fragmentShaderSource);

  glfwSwapInterval(0);
  while (!glfwWindowShouldClose(window)) {

    currentFrame = glfwGetTime();
    deltaFrame = currentFrame - lastFrame;
    lastFrame = currentFrame;
    if (deltaFrame > TAU)
      deltaFrame -= TAU;

    processInput(window, deltaFrame);
    char buffer[256];
    if (time(NULL) > tick) {
      fps = avg;
      avg = 0;

      glfwSetWindowTitle(window, buffer);

      tick = time(NULL);
    }

    sprintf(buffer, "Wilk (%d FPS, [%.2f, %.2f] xy, %.2f%% scale)", fps, x, y,
            scale);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glfwGetWindowSize(window, (int *)&width, (int *)&height);

    glUniform2f(glGetUniformLocation(program, "limits"), width, height);
    glUniform2f(glGetUniformLocation(program, "loc"), x, y);
    glUniform1f(glGetUniformLocation(program, "scale"), scale);
    glUniform1f(glGetUniformLocation(program, "maxIterations"), MAX_ITER);

    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    avg++;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glfwTerminate();
  return 0;
}
