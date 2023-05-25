#include <glad/gl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
// TODO: Drop and use stdio?
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

char checkGLLinkError(GLuint idx) {
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

char checkGLShaderCompileError(GLuint idx) {
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

double scale = 1.0f;
double x = 0.0, y = 0.0;
double maxInterations = 100.0;

const double megaScale = 5080.0f;
const double speed = 0.10;

void onKeyPress(GLFWwindow *window, int key, int scancode, int action,
                int mods) {
  (void)scancode;
  (void)mods;
  (void)window;

  if (action != GLFW_PRESS)
    return;

  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_UP:
    y += speed / scale;
    break;
  case GLFW_KEY_DOWN:
    y -= speed / scale;
    break;
  case GLFW_KEY_LEFT:
    x -= speed / scale;
    break;
  case GLFW_KEY_RIGHT:
    x += speed / scale;
    break;
  case GLFW_KEY_I:
    scale += megaScale;
    break;
  case GLFW_KEY_O:
    scale -= megaScale;
    break;
  case GLFW_KEY_J:
    maxInterations -= 1.0;
    break;
  case GLFW_KEY_K:
    maxInterations += 1.0;
    break;
  }
}

void onScroll(GLFWwindow *window, double xoffset, double yoffset) {
  (void)window;
  (void)xoffset;
  scale += yoffset;
}

int main(void) {
  GLFWwindow *window;
  GLuint vbo, ebo, vao, vertexShader, fragmentShader, program, width, height,
      fps, avg;
  char title[256];
  time_t tick;
  const char *vertexShaderSource, *fragmentShaderSource;

  if (!glfwInit()) {
    glfwTerminate();
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwSetErrorCallback(glfwError);

  fragmentShaderSource = readFile("src/shader/wilk.frag");
  vertexShaderSource = readFile("src/shader/wilk.vert");
  window = glfwCreateWindow(800, 600, "Wilk", NULL, NULL);
  tick = time(NULL);

  if (!window) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSetFramebufferSizeCallback(window, setFramebufferSize);

  puts("[Info] Creating buffers");
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

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

  puts("[Info] Compiling shaders");
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  if (!checkGLShaderCompileError(vertexShader)) {
    return 1;
  }

  puts(" [Debug] Compiled vertex shader");

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  if (!checkGLShaderCompileError(fragmentShader)) {
    return 1;
  }

  puts(" [Debug] Compiled fragment shader");

  puts("[Info] Attaching shaders");
  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  if (!checkGLLinkError(program)) {
    return 1;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  free((void *)vertexShaderSource);
  free((void *)fragmentShaderSource);

  printf("[Info] Renderer: %s (%s)\n", glGetString(GL_RENDERER),
         glGetString(GL_VENDOR));

  glfwSwapInterval(0);
  glfwSetScrollCallback(window, onScroll);
  glfwSetKeyCallback(window, onKeyPress);

  while (!glfwWindowShouldClose(window)) {

    if (time(NULL) > tick) {
      fps = avg;
      avg = 0;

      sprintf(title, "Wilk (%d FPS, [%.2f, %.2f] xy, %.2f%% scale, %.2f mit)",
              fps, x, y, scale * 100, maxInterations);
      glfwSetWindowTitle(window, title);

      tick = time(NULL);
    }

    glUseProgram(program);
    glfwGetWindowSize(window, (int *)&width, (int *)&height);

    glUniform2d(glGetUniformLocation(program, "limits"), width, height);
    glUniform2d(glGetUniformLocation(program, "loc"), x, y);
    glUniform1d(glGetUniformLocation(program, "scale"), scale);
    glUniform1d(glGetUniformLocation(program, "maxIterations"), maxInterations);

    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    avg++;
  }

  glDeleteProgram(program);

  glfwTerminate();
  return 0;
}
