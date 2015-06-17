#include "GLUtilsPrecompiled.h"
#include "GLUtilities.h"

#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

#include "Helper.h"

namespace GLUtils {



  bool CRenderUtils::RenderSphere(vec3 const & origin, float radius, unsigned int rings, unsigned int sectors)
  {
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texcoords;
    std::vector<GLushort> indices;

    const float inverseRings = 1.0f/static_cast<float>(rings-1);
    const float inverseSectors = 1.0f/static_cast<float>(sectors-1);

    vertices.resize(rings * sectors * 3);
    normals.resize(rings * sectors * 3);
    texcoords.resize(rings * sectors * 2);
    std::vector<GLfloat>::iterator v = vertices.begin();
    std::vector<GLfloat>::iterator n = normals.begin();
    std::vector<GLfloat>::iterator t = texcoords.begin();

    unsigned int r = 0;
    unsigned int s = 0;

    for(r = 0; r < rings; r++) {
      for(s = 0; s < sectors; s++) {
        const float rF = static_cast<float>(r);
        const float sF = static_cast<float>(s);
        const float y = static_cast<float>(sin( -M_PI_2 + M_PI * rF * inverseRings ));
        const float x = static_cast<float>(cos(2.0 * M_PI * sF * inverseSectors) * sin( M_PI * rF * inverseRings ));
        const float z = static_cast<float>(sin(2.0 * M_PI * sF * inverseSectors) * sin( M_PI * rF * inverseRings ));

        *t++ = sF * inverseSectors;
        *t++ = rF * inverseRings;

        *v++ = x * radius;
        *v++ = y * radius;
        *v++ = z * radius;

        *n++ = x;
        *n++ = y;
        *n++ = z;
      }
    }

    indices.resize(rings * sectors * 4);
    std::vector<GLushort>::iterator i = indices.begin();
    for(r = 0; r < rings-1; r++) {
      for(s = 0; s < sectors-1; s++) {
        *i++ = r * sectors + s;
        *i++ = r * sectors + (s+1);
        *i++ = (r+1) * sectors + (s+1);
        *i++ = (r+1) * sectors + s;
      }
    }


    {
      CGLPushMatrix scopedMatrix(GL_MODELVIEW);
      glTranslatef(origin.x,origin.y,origin.z);

      glEnableClientState(GL_VERTEX_ARRAY);
      //glEnableClientState(GL_NORMAL_ARRAY);
      //glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
      //glNormalPointer(GL_FLOAT, 0, &normals[0]);
      //glTexCoordPointer(2, GL_FLOAT, 0, &texcoords[0]);
      glDrawElements(GL_QUADS, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_SHORT, &indices[0]);
    }


    return true;
  }



  bool CRenderUtils::RenderPrism(std::vector<vec3> const & bottomContour, std::vector<vec3> const & topContour, const vec3 & bottomCenterPoint, const vec3 & topCenterPoint)
  {
    return RenderPrism(bottomContour, topContour, true, false, &bottomCenterPoint, &topCenterPoint, nullptr, nullptr);
  }



  //if topColor is specified, the color parameter is used for the bottom
  bool CRenderUtils::RenderColoredPrism(std::vector<vec3> const & bottomContour, std::vector<vec3> const & topContour, const vec3 & bottomCenterPoint, const vec3 & topCenterPoint, vec4 const & color, const vec4 & topColor)
  {
    return RenderPrism(bottomContour, topContour, false, true, &bottomCenterPoint, &topCenterPoint, &color, &topColor);
  }



  //Renders a Prism, while the top and the bottom cap are both given (other specifications are future work) and must be symetrical
  //If Caps should be rendered, the center point of both caps must currently be given, tesselation is future work
  bool CRenderUtils::RenderPrism(std::vector<vec3> const & bottomContour, std::vector<vec3> const & topContour, bool renderCaps, bool useColor, const vec3 * bottomCenterPoint, const vec3 * topCenterPoint, const vec4 * bottomColor, const vec4 * topColor)
  {
    if (bottomContour.size() <= 1) {
      assert(false); //contour is not valid, at least two points must be available
      return false;
    }

    if (renderCaps) {
      if (bottomCenterPoint == nullptr || topCenterPoint == nullptr) {
        assert(false); //center points must be given, tesselation is currently not available
        return false;
      }
    }

    if (useColor) {
      if (bottomColor == nullptr) {
        assert(false); //at least one color must be specified
        return false;
      }
    }

    if (bottomContour.size() != topContour.size()) {
      assert(false); //top and bottom caps should be symetrical
      return false;
    }

    //surface
    glBegin(GL_TRIANGLE_STRIP);

      for (size_t i = 0; i < bottomContour.size(); ++i) {

        if (bottomColor != nullptr) AsColor(*bottomColor);
        AsVertex(bottomContour[i]);

        if (topColor != nullptr) AsColor(*topColor);
        AsVertex(topContour[i]);
      }

      if (bottomColor != nullptr) AsColor(*bottomColor);
      AsVertex(bottomContour[0]);

      if (topColor != nullptr) AsColor(*topColor);
      AsVertex(topContour[0]);

    glEnd();

    if (renderCaps) {

      //bottom
      glBegin(GL_TRIANGLE_FAN);
      if (bottomColor != nullptr) AsColor(*bottomColor);
      AsVertex(*bottomCenterPoint);
	  for (auto point = bottomContour.begin(); point != bottomContour.end(); ++point) {
        AsVertex(*point);
      }

      AsVertex(bottomContour[0]);
      glEnd();

      //top
      glBegin(GL_TRIANGLE_FAN);
      if (topColor != nullptr) AsColor(*topColor);
      AsVertex(*topCenterPoint);
      for (auto pointIter = topContour.rbegin(); pointIter != topContour.rend(); ++pointIter) {
        AsVertex(*pointIter);
      }
      AsVertex(topContour[topContour.size()-1]);
      glEnd();
    }

    return true;
  }

//----------------------------------------------------------------------------------------


  CGLPushMatrix::CGLPushMatrix(void)
  {
    GLint mode;
    glGetIntegerv(GL_MATRIX_MODE, &mode);
    mCurrentMatrix = static_cast<GLenum>(mode);
    glPushMatrix();
    GLUtils::CHelper::CheckForError();
  }



  CGLPushMatrix::CGLPushMatrix(GLenum matrix)
  {
    GLint mode;
    glGetIntegerv(GL_MATRIX_MODE, &mode);
    mCurrentMatrix = matrix;
    glMatrixMode(mCurrentMatrix);
    glPushMatrix();
    glMatrixMode(static_cast<GLenum>(mode));
    GLUtils::CHelper::CheckForError();
  }



  CGLPushMatrix::~CGLPushMatrix(void)
  {
    GLUtils::CHelper::CheckForError();
    GLint mode;
    glGetIntegerv(GL_MATRIX_MODE, &mode);
    glMatrixMode(mCurrentMatrix);
    glPopMatrix();
    glMatrixMode(static_cast<GLenum>(mode));
    GLUtils::CHelper::CheckForError();
  }



  CGLPushAttribut::CGLPushAttribut(GLenum attribute)
  {
    glPushAttrib(attribute);
    GLUtils::CHelper::CheckForError();
  }



  CGLPushAttribut::~CGLPushAttribut(void)
  {
    glPopAttrib();
    GLUtils::CHelper::CheckForError();
  }

}