#ifndef GL_GLC_QUAD_H
#define GL_GLC_QUAD_H
#include <program.h>
#include <glm.h>
#include <array>
#include <common.h>
#include <geometry.h>

namespace zxd
{

class quad;
// draw single fullscreen 2d quad
void draw_quad(GLuint tex, GLuint tui = 0);

void draw_depth_quad(GLuint tex, GLuint tui = 0);

// call draw, nothing else. you must build program context before call this
void draw_quad();
quad& get_nd_quad();

class quad : public common_geometry
{

public:
  //quad(GLfloat size = 2) : m_width(size), m_height(size) {}
  quad(const vec3& corner = vec3(-1, -1, 0),
    const vec3& edge0_vec = vec3(2, 0, 0),
    const vec3& edge1_vec = vec3(0, 2, 0));

  void setup(const vec3& corner, const vec3& edge0_vec, const vec3& edge1_vec,
    const vec2& tc0 = vec2(0), const vec2& tc1 = vec2(1));

  void setup(GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1);

  GLfloat left();
  GLfloat right();
  GLfloat bottom();
  GLfloat top();

  const vec3& v0() const { return m_v0; }
  void v0(const vec3& v){ m_v0 = v; }

  const vec3& v1() const { return m_v1; }
  void v1(const vec3& v){ m_v1 = v; }

  const vec3& v2() const { return m_v2; }
  void v2(const vec3& v){ m_v2 = v; }

  const vec3& v3() const { return m_v3; }
  void v3(const vec3& v){ m_v3 = v; }

  const vec2& tc0() const { return m_tc0; }
  void tc0(const vec2& v){ m_tc0 = v; }

  const vec2& tc1() const { return m_tc1; }
  void tc1(const vec2& v){ m_tc1 = v; }

private:

  // ccw order, starts from lb
  vec3 m_v0;
  vec3 m_v1;
  vec3 m_v2;
  vec3 m_v3;
  vec2 m_tc0;
  vec2 m_tc1;

  vertex_build build_vertices();
  array_ptr build_normals(const array& vertices);
  array_ptr build_texcoords(const array& vertices);
  array_ptr build_tangents(const array& vertices);
};

class billboard_quad : public common_geometry
{
protected:
  vec2 m_v0 = vec2(-5);
  vec2 m_v1 = vec2(5);
  vec2 m_texcoord0 = vec2(0);
  vec2 m_texcoord1 = vec2(1);

public:
  billboard_quad(){}

  void rect(const vec2& v0, const vec2& v1);
  void texcoord(const vec2& tc0, const vec2& tc1);

protected:
  vertex_build build_vertices();
  array_ptr build_texcoords(const array& vertices);

};

}

#endif /* GL_GLC_QUAD_H */
