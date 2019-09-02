#ifndef GL_GLC_ARRAY_H
#define GL_GLC_ARRAY_H
#include <memory>
#include <iostream>

#include "glm.h"
#include "mixinvector.h"

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(bytes) ((GLubyte *)NULL + (bytes))
#endif

namespace zxd
{

using namespace glm;

class array
{
protected:
  GLuint m_buffer;
  GLenum m_target;
  GLenum m_usage;
  array(GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW)
    : m_buffer(-1),
    m_target(target),
    m_usage(usage)
  {}
  
  virtual ~array() {}
  void bind_buffer()
  {
    if (m_buffer == -1)
    {
      glGenBuffers(1, &m_buffer);
    }
    glBindBuffer(m_target, m_buffer);
  }

public:
  virtual void update_buffer() {}
  virtual void bind(GLint location) {}
  virtual GLuint num_elements() const = 0;
  virtual void read_buffer() = 0;

  GLenum target() const { return m_target; }
  void target(GLenum v){ m_target = v; }

  GLenum usage() const { return m_usage; }
  void usage(GLenum v){ m_usage = v; }

  GLuint buffer() const { return m_buffer; }
};

template <typename T>
class template_array : public array, public mixin_vector<T>
{

public:
  template_array(GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW);
  void bind(GLint location);
  void update_buffer();
  virtual GLuint num_elements() const { return this->size(); }
  virtual void read_buffer();
};

using uint_array = template_array<uint>;
using uvec1_array = template_array<uvec1>;
using vec1_array = template_array<vec1>;
using vec2_array = template_array<vec2>;
using vec3_array = template_array<vec3>;
using vec4_array = template_array<vec4>;

using array_ptr = std::shared_ptr<array>;
using vec1_array_ptr = std::shared_ptr<vec1_array>;
using vec2_array_ptr = std::shared_ptr<vec2_array>;
using vec3_array_ptr = std::shared_ptr<vec3_array>;
using vec4_array_ptr = std::shared_ptr<vec4_array>;

template <typename T> constexpr GLenum gl_type_enum = GL_FLOAT;
template<> constexpr GLenum gl_type_enum<GLchar> = GL_BYTE;
template<> constexpr GLenum gl_type_enum<GLubyte> = GL_UNSIGNED_BYTE;
template<> constexpr GLenum gl_type_enum<GLshort> = GL_SHORT;
template<> constexpr GLenum gl_type_enum<GLushort> = GL_UNSIGNED_SHORT;
template<> constexpr GLenum gl_type_enum<GLint> = GL_INT;
template<> constexpr GLenum gl_type_enum<GLuint> = GL_UNSIGNED_INT;
template<> constexpr GLenum gl_type_enum<GLdouble> = GL_DOUBLE;

template <length_t L, typename T, qualifier Q>
constexpr GLenum gl_type_enum<vec<L, T, Q>> = gl_type_enum<T>;

template <typename T> constexpr GLuint glm_type_components = 1;
template <length_t L, typename T, qualifier Q>
GLuint glm_type_components<vec<L, T, Q>> = L;

//--------------------------------------------------------------------
template <typename T>
template_array<T>::template_array(
  GLenum target /* = GL_ARRAY_BUFFER*/, GLenum usage /* = GL__DRAW*/)
    : array(target, usage)
{
}

//--------------------------------------------------------------------
template <typename T>
void template_array<T>::bind(GLint location)
{
  bind_buffer();
  glVertexAttribPointer(location, glm_type_components<T>, gl_type_enum<T>,
    GL_FALSE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(location);
}

//--------------------------------------------------------------------
template <typename T>
void template_array<T>::update_buffer()
{
  bind_buffer();
  glBufferData(m_target, this->size() * sizeof(decltype(this->front())),
    &this->front(), m_usage);
}

//--------------------------------------------------------------------
template<typename T>
void template_array<T>::read_buffer()
{
  bind_buffer();
  GLint buffer_size;
  glGetBufferParameteriv(m_target, GL_BUFFER_SIZE, &buffer_size);
  glGetBufferSubData(m_target, 0, buffer_size, &this->get_vector().front());
}
}

#endif /* GL_GLC_ARRAY_H */
