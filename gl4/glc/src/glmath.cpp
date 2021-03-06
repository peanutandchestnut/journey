#include <glmath.h>

#include <stdexcept>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#include <geometry_test.h>

namespace zxd
{

//--------------------------------------------------------------------
mat4 isometric_projection(GLfloat d, const vec3& center, const vec3& up/* = pza*/)
{
  return glm::lookAt(center + vec3(d, -d, d), center, up);
}

//--------------------------------------------------------------------
vec3 make_floor(const vec3& lhs, const vec3& rhs)
{
  return vec3(
    std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z));
}

//--------------------------------------------------------------------
vec3 make_ceil(const vec3& lhs, const vec3& rhs)
{
  return vec3(
    std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z));
}

//--------------------------------------------------------------------
vec3 axis(const mat4& m, GLuint i) { return vec3(column(m, i)); }

//--------------------------------------------------------------------
mat4 create_mat4(const vec3& xa, const vec3& ya, const vec3& za)
{
  return mat4(
    vec4(xa, 0), vec4(ya, 0), vec4(za, 0), vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

//--------------------------------------------------------------------
GLfloat angle_any(const vec3& lhs, const vec3& rhs)
{
  return angle(normalize(lhs), normalize(rhs));
}

//--------------------------------------------------------------------
GLfloat random()
{
  return min(rand(), RAND_MAX - 1)/static_cast<GLfloat>(RAND_MAX);
}

//--------------------------------------------------------------------
vec3 face_normal(const vec3& v0, const vec3& v1, const vec3& v2)
{
  vec3 v01 = v1 - v0;
  vec3 v02 = v2 - v0;
  vec3 normal = cross(v01, v02);
  return normalize(normal);
}

//--------------------------------------------------------------------
mat4 get_tangetn_basis(const vec3& v0, const vec3& v1, const vec3& v2,
  const vec2& texcoord0, const vec2& texcoord1, const vec2& texcoord2,
  const vec3* normal /* = 0*/)
{
  vec3 dt_pos01 = v1 - v0;
  vec3 dt_pos02 = v2 - v0;
  vec2 dt_tex01 = texcoord1 - texcoord0;
  vec2 dt_tex02 = texcoord2 - texcoord0;

  float r = 1.0 / (dt_tex01.x * dt_tex02.y - dt_tex02.x * dt_tex01.y);
  vec3 tangent = (dt_pos01 * dt_tex02.y - dt_pos02 * dt_tex01.y) * r;
  tangent = glm::normalize(tangent);
  vec3 bitangent = (dt_pos02 * dt_tex01.x - dt_pos01 * dt_tex02.x) * r;
  bitangent = glm::normalize(bitangent);
  return create_mat4(
    tangent, bitangent, normal == 0 ? face_normal(v0, v1, v2) : *normal);
}
//--------------------------------------------------------------------
vec3 transform_position(const mat4& m, const vec3& v)
{
  vec3 r;

  GLfloat inv_w =
    1.0f / (m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3]);
  r.x = (m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0]) * inv_w;
  r.y = (m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1]) * inv_w;
  r.z = (m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2]) * inv_w;

  return r;
}

//--------------------------------------------------------------------
vec3 transform_vector(const mat4& m, const vec3& v)
{
  vec3 r;

  r.x = (m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z);
  r.y = (m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z);
  r.z = (m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z);

  return r;
}

//--------------------------------------------------------------------
mat4 rotate_to(const vec3& v0, const vec3& v1)
{

#ifdef _DEBUG
  assert(abs(length(v0) - 1) < 0.00001);
  assert(abs(length(v1) - 1) < 0.00001);
#endif

  GLfloat cos = glm::dot(v0, v1);
  // the same direction
  if(std::abs(cos - 1) < 0.000001f) // is 0.00001 small enough?
    return mat4(1);

  // reverse directoin
  if(std::abs(cos + 1) < 0.000001f)
  {
    // use pxa or pya to find an axis to rotate v0 180 degrees
    vec3 v2 = pxa;
    // use pxa or pya to rotate 180 degree
    if(1.0f - glm::abs(glm::dot(v0, pxa)) < 0.000001f)
      v2 = pya;

    vec3 axis = glm::cross(v0, v2);
    return glm::rotate(fpi, axis);
  }

  // normal case
  GLfloat theta = glm::acos(cos);
  vec3 axis = glm::cross(v0, v1);
  axis = glm::normalize(axis);
  return glm::rotate(theta, axis);
}

//--------------------------------------------------------------------
mat4 rotate_to_any(const vec3& v0, const vec3& v1)
{
  return rotate_to(glm::normalize(v0), glm::normalize(v1));
}

//--------------------------------------------------------------------
vec3 rotate_in_cone(const vec3& ndir, GLfloat angle)
{
  return ndir * cos(angle) + random_orthogonal(ndir) * sin(angle);
}

//--------------------------------------------------------------------
vec3 random_orthogonal(const vec3& ndir)
{
  if(glm::length2(ndir) < 0.00001)
  {
    std::cout << "failed to get random orthogonal for ndir with tiny magnitude" << std::endl;
    return vec3(1, 0, 0);
  }

  vec3 ref = glm::sphericalRand(1.0);
  while(glm::abs(glm::dot(ndir, ref)) > 0.99999f)
    ref = glm::sphericalRand(1.0);
  return glm::normalize(glm::cross(ndir, ref));
}

//--------------------------------------------------------------------
float gaussian_weight(double x, double mean, double deviation)
{
  static float e = 2.71828182845904523536028747135266249775724709369995;
  static float r = 1 / std::sqrt(2 * fpi);
  float rd = 1 / (deviation * deviation);
  return std::pow(e, -0.5f * std::pow((x - mean) * rd, 2)) * rd * r;
}

//--------------------------------------------------------------------
mat4 rect_ortho(GLfloat hw, GLfloat hh, GLfloat aspect, GLfloat n/* = -1*/, GLfloat f/* = 1*/)
{
  GLfloat project_aspect = hw/hh;
  if(project_aspect >= aspect)
    hh *= project_aspect / aspect;
  else
    hw *= aspect / project_aspect;
  return glm::ortho(-hw, hw, -hh, hh, n, f);
}

//--------------------------------------------------------------------
vec3 clamp_length(const vec3& v, GLfloat min_lenth,  GLfloat max_length)
{
  GLfloat l2 = glm::length2(v);

  if(l2 < min_lenth * min_lenth)
    return v / glm::sqrt(l2) * min_lenth;

  if(l2 > max_length * max_length)
    return v / glm::sqrt(l2) * max_length;

  return v;
}

//--------------------------------------------------------------------
vec2 rotate(GLfloat angle, const vec2& v)
{
  float c = glm::cos(angle);
  float s = glm::sin(angle);
  return vec2(c * v.x - s * v.y, s * v.x  + c * v.y);
}


//------------------------------------------------------------------------------
bool operator<(const vec3& lhs, const vec3& rhs)
{
  return lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z;
}

//--------------------------------------------------------------------
bool operator>(const vec3& lhs, const vec3& rhs)
{
  return lhs != rhs && !operator<(lhs, rhs);
}

//--------------------------------------------------------------------
GLfloat max_abs_component(const vec3& v)
{
  GLfloat x = glm::abs(v[0]);
  GLfloat y = glm::abs(v[1]);
  GLfloat z = glm::abs(v[2]);
  return glm::max(glm::max(x, y), z);
}

//--------------------------------------------------------------------
GLuint gcd(GLuint a, GLuint b)
{
  if(a == b)
    return a;

  GLuint x = a;
  GLuint y = b;

  if(a < b)
  {
    x = b;
    y = a;
  }

  while(true)
  {
    GLuint c = x % y;
    if(c == 0)
      return y;

    x = y;
    y = c;
  }
}

//--------------------------------------------------------------------
GLuint lcm(GLuint a, GLuint b)
{
  return a*b/gcd(a,b);
}

//--------------------------------------------------------------------
GLuint ilcm(GLuint a, GLuint b)
{
  return lcm(a, b) / (a * b);
}

//--------------------------------------------------------------------
GLfloat cubic_in_out(GLfloat x)
{
  if(x <= 0.5)
    return 4 * x * x * x;

  x = 2 * x - 2;
  return 0.5 * x * x * x + 1;
}

//--------------------------------------------------------------------
vec3 wnd2ball(const vec2& p, const vec2& center, GLfloat radius)
{
  auto pos = p - center;
  auto l2 = length2(pos);
  auto r2 = radius * radius;
  if (l2 <= r2 * 0.5f)
    return vec3(pos, sqrt(r2 - l2));
  else
    return vec3(pos, r2 * 0.5f / sqrt(l2));
}

//--------------------------------------------------------------------
mat4 trackball_rotate(
  const vec2& p0, const vec2& p1, const vec2& center, GLfloat radius)
{
  auto sp0 = wnd2ball(p0, center, radius);
  auto sp1 = wnd2ball(p1, center, radius);
  return rotate_to(normalize(sp0), normalize(sp1));
}

//--------------------------------------------------------------------
glm::mat4 compute_window_mat(GLint x, GLint y, GLint width, GLint height,
                             GLfloat depth_near, GLfloat depth_far)
{
  glm::mat4 m(1);

  // m = glm::translate(vec3(x, y, depth_near)) * glm::scale(vec3(width*0.5,
  // height*0.5, 0.5f * (depth_far - depth_near))) * glm::translate(vec3(1));

  m[0][0] = width / 2.0;
  m[1][1] = height / 2.0;
  m[2][2] = (depth_far - depth_near) / 2.0;
  m[3][0] = x + m[0][0];
  m[3][1] = y + m[1][1];
  m[3][2] = (depth_far + depth_near) / 2.0;

  return m;
}

//--------------------------------------------------------------------
glm::mat4 compute_window_mat_i(
  GLint x, GLint y, GLint width, GLint height, GLfloat n, GLfloat f)
{
  glm::mat4 m(1);
  m[0][0] = 2.0 / width;
  m[1][1] = 2.0 / height;
  m[2][2] = 2.0 / (f - n);
  m[3][0] = -1 - 2 * x / width;
  m[3][1] = -1 - 2 * y / height;
  m[3][2] = -(f + n) / (f - n);

  return m;
}

//--------------------------------------------------------------------
void set_row(mat4& m, GLushort index, const vec4& v)
{
  m[0][index] = v[0];
  m[1][index] = v[1];
  m[2][index] = v[2];
  m[3][index] = v[3];
}

//--------------------------------------------------------------------
void set_row(mat4& m, GLushort index, const vec3& v)
{
  m[0][index] = v[0];
  m[1][index] = v[1];
  m[2][index] = v[2];
}

//--------------------------------------------------------------------
void set_col(mat4& m, GLushort index, const vec3& v)
{
  m[index][0] = v[0];
  m[index][1] = v[1];
  m[index][2] = v[2];
}

//--------------------------------------------------------------------
mat4 erase_translation(const glm::mat4& m)
{
  return mat4(m[0], m[1], m[2], hzp);
}

//--------------------------------------------------------------------
mat4 make_mat4_row(
  const vec4& r0, const vec4& r1, const vec4& r2, const vec4& r3)
{
  // clang-format off
  return mat4(
      r0[0], r1[0], r2[0], r3[0], 
      r0[1], r1[1], r2[1], r3[1], 
      r0[2], r1[2], r2[2], r3[2], 
      r0[3], r1[3], r2[3], r3[3]
      );
  // clang-format on
}

//--------------------------------------------------------------------
mat4 make_mat4_row(
const vec3& r0, const vec3& r1, const vec3& r2)
{
  // clang-format off
  return mat4(
      r0[0], r1[0], r2[0], 0,
      r0[1], r1[1], r2[1], 0,
      r0[2], r1[2], r2[2], 0,
      0,     0,     0,     1
      );
  // clang-format on
}

//--------------------------------------------------------------------
mat3 make_mat3_row(const vec3& r0, const vec3& r1, const vec3& r2)
{
  // clang-format off
  return mat3(
      r0[0], r1[0], r2[0],
      r0[1], r1[1], r2[1],
      r0[2], r1[2], r2[2]
      );
  // clang-format on
}

//--------------------------------------------------------------------
GLfloat pi(GLfloat a0, GLfloat d, uint n) 
{
  GLfloat m = 1;
  while (n--) m *= a0 + n * d;  // reverse order of pi
  return m;
}

//--------------------------------------------------------------------
GLfloat triangle_area(const vec3& p0, const vec3& p1, const vec3& p2)
{
  return glm::length(glm::cross(p1 - p0, p2 - p0)) * 0.5f;
}

//--------------------------------------------------------------------
vec3 eye_pos(const mat4& v_mat)
{
  return vec3(-v_mat[3]) * mat3(v_mat);
}

//--------------------------------------------------------------------
mat4 reflect_point(const vec3& p)
{
  // A' = p * 2 - A
  // return translate(scale(translate(p), vec3(-1)), -p);
  return scale(translate(p*2.0f), vec3(-1));
}

//--------------------------------------------------------------------
mat4 reflect_plane(const vec4& p)
{
	//@TODO implement
	throw new std::runtime_error("unimplemented ___FUNC___ called");
}

template<length_t L, typename T, glm::qualifier Q>
vec<L,T,Q> scale_vec(const vec<L,T,Q>& v, const vec<L,T,Q>& n, GLfloat k)
{
  // v_on_n = v·n*n * k
  // v_perp_n = v - v·n*n
  return v + (k - 1.0f) * dot(v, n) * n;
}

template vec3 scale_vec(const vec3& v, const vec3& n, GLfloat k);
template vec2 scale_vec(const vec2& v, const vec2& n, GLfloat k);

//--------------------------------------------------------------------
mat4 scale_along(const vec3& n, GLfloat k)
{
  GLfloat km1 = k - 1;
  // based on apply scale_vec on e0, e1, e2
  return mat4(
      1 + km1 * n.x * n.x, km1 * n.x * n.y,     km1 * n.x * n.z,     0,
      km1 * n.x * n.y,     1 + km1 * n.y * n.y, km1 * n.y * n.z,     0,
      km1 * n.x * n.z,     km1 * n.y * n.z,     1 + km1 * n.z * n.z, 0,
      0,                   0,                   0,                   1
  );
}

//--------------------------------------------------------------------
mat4 project(const vec4& p)
{
  vec3 n(p);
  vec3 origin_to_plane = -p.w * n;
  mat4 T = glm::translate(origin_to_plane);
  mat4 T_i = glm::translate(-origin_to_plane);
  mat4 A = scale_along(n, 0);
  return T * A * T_i;
}

//--------------------------------------------------------------------
mat4 reflect(const vec4& p)
{
  vec3 n(p);
  vec3 origin_to_plane = -p.w * n;
  mat4 T = glm::translate(origin_to_plane);
  mat4 T_i = glm::translate(-origin_to_plane);
  mat4 A = scale_along(n, -1);
  return T * A * T_i;
}

//--------------------------------------------------------------------
mat4 reflect(const mat4& v_mat, const vec4& p)
{
  auto m = reflect(p);
  auto v_mat_i = glm::inverse(v_mat);
  auto eye = vec3(v_mat_i[3]);
  auto eye_dir = -vec3(v_mat_i[2]);
  auto eye_up = vec3(v_mat_i[1]);
  auto reflected_eye = vec3(m * vec4(eye, 1));
  auto reflected_dir = vec3(m * vec4(eye_dir, 0));
  auto reflected_up = vec3(m * vec4(eye_up, 0));
  return lookAt(reflected_eye, reflected_eye + reflected_dir * 10.0f, reflected_up);
}

}

