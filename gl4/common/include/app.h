/*
 * Changed from superbible.
 */
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>
#include "glm.h"

namespace zxd {

void glfwErrorCallback(int error, const char *description);
void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
  GLsizei length, const GLchar *message, const void *userParam);

class App {
public:
  struct AppInfo {
    std::string title;
    GLuint wndWidth;
    GLuint wndHeight;
    GLuint majorVersion;
    GLuint minorVersion;
    GLuint samples;
    GLboolean fullscreen;
    GLboolean vsync;
    GLboolean cursor;
    GLboolean stereo;
    GLboolean debug;
  };

protected:
  AppInfo mInfo;
  GLFWwindow *mWnd;
  mat4 *mViewMatrix;

  dvec2 mLastButtonPosition; //used to rotate camera when right button pressed

public:
  App() : mViewMatrix(0) {}

  void init();
  virtual void run();
  inline GLfloat getWndAspect() {
    return static_cast<GLfloat>(mInfo.wndWidth) / mInfo.wndHeight;
  }

  mat4 *getViewMatrix() const { return mViewMatrix; }
  void setViewMatrix(mat4 *v) { mViewMatrix = v; }

protected:
  virtual void initInfo();
  virtual void initGL();
  virtual void initWnd();
  virtual void createScene() {}
  virtual void update() {}
  virtual void display() {}

  virtual void loop();
  virtual void shutdown() {}

  virtual void glfwResize(GLFWwindow *wnd, int w, int h);
  virtual void glfwKey(
    GLFWwindow *wnd, int key, int scancode, int action, int mods);
  virtual void glfwMouseButton(
    GLFWwindow *wnd, int button, int action, int mods) ;
  virtual void glfwMouseMove(GLFWwindow *wnd, double x, double y);
  virtual void glfwMouseWheel(GLFWwindow *wnd, double xoffset, double yoffset) ;
};
}