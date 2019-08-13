#include "cuboid.h"

#include <algorithm>
#include "common.h"

namespace zxd
{

void add_quad(vec3_array &vertices, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
  vertices.push_back(vertices[v0]);
  vertices.push_back(vertices[v1]);
  vertices.push_back(vertices[v2]);
  vertices.push_back(vertices[v3]);
}

void add_quad(uint_array &elements, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
  elements.push_back(v0);
  elements.push_back(v1);
  elements.push_back(v2);

  elements.push_back(v0);
  elements.push_back(v2);
  elements.push_back(v3);
}


//--------------------------------------------------------------------
void cuboid::build_vertex()
{

  //        1         0
  //        ----------
  //  2     |   3    |
  //  ------|---     |
  //  |     |  |     |
  //  |     |  |     |
  //  |     |  |     |
  //  |    6---|------7
  //  |        |
  // 5----------4
  //
  // 0, 1, 2, 3  --- top
  // 4, 5, 6, 7  --- bottom
  // 2, 5, 4, 3  --- front
  // 0, 7, 6, 1  --- back
  // 4, 7, 0, 3  --- right
  // 2, 1, 6, 5  --- left
  //
  // there is a tetrahedron between 0 2 4 6, it's 6 edges is also the shared
  // edge in each of the 6 faces

  auto vertices = make_array<vec3_array>(0);
  auto num_vertices = m_type == type::CT_8 ? 8 : 24;

  vertices->reserve(num_vertices);

  GLfloat x = abs(m_half_diag.x);
  GLfloat y = abs(m_half_diag.y);
  GLfloat z = abs(m_half_diag.z);

  // top (ccw)
  vertices->push_back( vec3( x,  y,  z) );
  vertices->push_back( vec3(-x,  y,  z) );
  vertices->push_back( vec3(-x, -y,  z) );
  vertices->push_back( vec3( x, -y,  z) );

  // bottom (cw)
  vertices->push_back( vec3( x, -y, -z) );
  vertices->push_back( vec3(-x, -y, -z) );
  vertices->push_back( vec3(-x,  y, -z) );
  vertices->push_back( vec3( x,  y, -z) );

  if ( m_type == type::CT_24)
  {
    add_quad(*vertices, 2, 5, 4, 3); //  --- front
    add_quad(*vertices, 0, 7, 6, 1); //  --- back
    add_quad(*vertices, 4, 7, 0, 3); //  --- right
    add_quad(*vertices, 2, 1, 6, 5); //  --- left
  }

  m_elements = std::make_shared<uint_array>();
  auto elements = std::static_pointer_cast<uint_array>(m_elements);
  elements->reserve(36);

  add_quad(*elements, 0, 1, 2, 3);
  add_quad(*elements, 4, 5, 6, 7);

  if (m_type == type::CT_24)
  {
    add_quad(*elements, 8, 9, 10, 11);
    add_quad(*elements, 12, 13, 14, 15);
    add_quad(*elements, 16, 17, 18, 19);
    add_quad(*elements, 20, 21, 22, 23);
  }
  else
  {
    add_quad(*elements, 2, 5, 4, 3);
    add_quad(*elements, 0, 7, 6, 1);
    add_quad(*elements, 4, 7, 0, 3);
    add_quad(*elements, 2, 1, 6, 5);
  }

  m_primitive_sets.clear();
  add_primitive_set(
    new draw_elements(GL_TRIANGLES, elements->size(), GL_UNSIGNED_INT, 0));
}

//--------------------------------------------------------------------
void cuboid::build_normal()
{
  auto normal = make_array<vec3_array>(num_arrays());
  auto vertices = attrib_vec3_array(0);
  if (m_type == type::CT_8)
  {
    std::transform(vertices->begin(), vertices->end(),
        std::back_inserter(*normal),
        [](const auto &vertex) -> vec3 { return normalize(vertex); });
  }
  else
  {
    for (unsigned i = 0; i < 4; ++i) normal->push_back(vec3( 0,  0,  1));
    for (unsigned i = 0; i < 4; ++i) normal->push_back(vec3( 0,  0, -1));
    for (unsigned i = 0; i < 4; ++i) normal->push_back(vec3( 0, -1,  0));
    for (unsigned i = 0; i < 4; ++i) normal->push_back(vec3( 0,  1,  0));
    for (unsigned i = 0; i < 4; ++i) normal->push_back(vec3( 1,  0,  1));
    for (unsigned i = 0; i < 4; ++i) normal->push_back(vec3(-1,  0,  1));
  }
}

//--------------------------------------------------------------------
void cuboid::build_texcoord()
{
  if (m_type != type::CT_24)
  {
    throw std::runtime_error(
      "CT_24 should be selected if you want to add texcoord to a cuboid");
  }

  vec2_array& texcoords = *(new vec2_array());
  attrib_array(num_arrays(), array_ptr(&texcoords));
  texcoords.reserve(num_vertices());

  // only care about front orientation
  for (int i = 0; i < 6; ++i)
  {
    texcoords.push_back(glm::vec2(0, 1));
    texcoords.push_back(glm::vec2(0, 0));
    texcoords.push_back(glm::vec2(1, 0));
    texcoords.push_back(glm::vec2(1, 1));
  }
}

}
