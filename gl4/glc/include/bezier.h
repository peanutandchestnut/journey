#ifndef BEZIER_H
#define BEZIER_H

#include "glad/glad.h"
#include "geometry.h"

namespace zxd {

/*
 * generally bezier class, degree >=1
 */
class bezier : public geometry3313 {
protected:
  GLuint m_partitions;  // number of partitions
  GLfloat m_begin;
  GLfloat m_end;

  vec3_vector m_ctrl_points;

public:
  bezier() : m_partitions(20), m_begin(0), m_end(1) {}

  virtual void build_vertex();
  virtual void build_texcoord();
  virtual void draw(GLuint primcount = 1);

  unsigned int getDegree() { return n(); }
  inline unsigned int n() { return m_ctrl_points.size() - 1; }

  // get point at t
  vec3 get(GLfloat t);

  // get point at ith iteration, jth index
  vec3 get(GLuint i, GLuint j, GLfloat t);

  // degree evevate or drop
  void elevate(bool positive = true);

  // derivative of a bezier curve is still a bezier curve
  bezier derivative(GLuint level = 1);

  // get tanget at t
  vec3 tangent(GLfloat t);

  // subdivide according to current point
  void subdivide(GLfloat t, bezier& lc, bezier& rc);

  static std::vector<vec3_vector> iterate_all(
    vec3_vector& ctrl_points, float t);

  // De Casteljau's algorithm
  static vec3_vector iterate(vec3_vector& ctrl_points, float t);

  static vec3 get(
    vec3_vector::iterator beg, vec3_vector::iterator end, GLfloat t);

  unsigned int partitions() const { return m_partitions; }
  void partitions(unsigned int v) { m_partitions = v; }

  GLfloat begin() const { return m_begin; }
  void begin(GLfloat v) { m_begin = v; }

  GLfloat end() const { return m_end; }
  void end(GLfloat v) { m_end = v; }

  const vec3_vector& ctrl_points() { return m_ctrl_points; }
  void ctrl_points(const vec3_vector& v) { m_ctrl_points = v; }

private:
  // special function used to derivative
  vec3 d(GLuint i, GLuint k);
};
}

#endif /* BEZIER_H */