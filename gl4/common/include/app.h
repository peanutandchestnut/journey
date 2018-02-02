/*
 * Changed from superbible.
 */
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string>
#include "glm.h"

namespace zxd {

void glfw_error_callback(int error, const char *description);
void gl_debug_output(GLenum source, GLenum type, GLuint id, GLenum severity,
  GLsizei length, const GLchar *message, const void *userParam);

class app {
public:
  enum camera_mode {
    CM_YAW_PITCH,  // yaw world, pitch camera
    CM_ARCBALL,
    CM_FREE  // yaw, pitch camera
  };
  enum move_dir {
    MD_LEFT = 1 << 0,
    MD_RIGHT = 1 << 1,
    MD_FOWARD = 1 << 2,
    MD_BACK = 1 << 3,
  };

  struct app_info {
    std::string title;
    GLuint wnd_width;
    GLuint wnd_height;
    GLuint major_version;
    GLuint minor_version;
    GLuint samples;
    GLboolean fullscreen;
    GLboolean vsync;
    GLboolean cursor;
    GLboolean stereo;
    GLboolean debug;
  };

protected:
  GLboolean m_reading;
  GLboolean m_camera_moving;
  GLuint m_move_dir;
  GLuint m_swap_interval{1};
  GLuint m_frame_number;
  GLdouble m_fps;
  GLdouble m_last_time;
  GLdouble m_current_time;
  GLdouble m_delta_time;

  GLdouble m_adtx;
  GLdouble m_adty;

  std::string m_input;
  camera_mode m_camera_mode;
  app_info m_info;
  GLFWwindow *m_wnd;
  mat4 *m_v_mat;
  mat4 m_start_v_mat;  // used to record init rotation in CM_FREE.

  dvec2
    m_last_cursor_position;  // used to rotate camera when mid button pressed
  vec3 m_camera_translation;  // translation during CM_FREE mode

public:
  app()
      : m_camera_mode(CM_YAW_PITCH),
        m_v_mat(0),
        m_reading(GL_FALSE),
        m_camera_moving(GL_FALSE),
        m_move_dir(0),
        m_frame_number(0),
        m_adtx(0),
        m_adty(0) {}

  void init();
  virtual void run();
  GLfloat wnd_aspect() {
    return static_cast<GLfloat>(m_info.wnd_width) / m_info.wnd_height;
  }

  mat4 *v_mat() const { return m_v_mat; }
  void set_v_mat(mat4 *v) { m_v_mat = v; }

  camera_mode get_camera_mode() const { return m_camera_mode; }
  void set_camera_mode(camera_mode v);

  void start_reading();
  void stop_reading();
  void finishe_reading();

protected:
  virtual void init_info();
  virtual void init_gl();
  virtual void debug_message_control();
  virtual void init_wnd();
  virtual void create_scene() {}
  virtual void update() {}
  virtual void display() {}
  virtual void update_time();
  virtual void update_fps();
  virtual void update_camera();

  virtual void loop();
  virtual void shutdown() {}

  virtual void glfw_resize(GLFWwindow *wnd, int w, int h);
  virtual void glfw_key(
    GLFWwindow *wnd, int key, int scancode, int action, int mods);
  virtual void glfw_mouse_button(
    GLFWwindow *wnd, int button, int action, int mods);
  virtual void glfw_mouse_move(GLFWwindow *wnd, double x, double y);
  virtual void glfw_mouse_wheel(
    GLFWwindow *wnd, double xoffset, double yoffset);
  virtual void glfw_char(GLFWwindow *wnd, unsigned int codepoint);
  virtual void glfw_charmod(GLFWwindow *wnd, unsigned int codepoint, int mods);

  GLdouble glfw_to_gl(GLdouble y);
};
}
