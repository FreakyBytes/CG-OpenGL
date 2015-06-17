#pragma once

#include "GLUtilsDefines.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>
#include <vector>
#include <algorithm>


using namespace glm;


namespace GLUtils {

  class CRenderUtils
  {
  public:
    GLUTILS_API static void AsColor(vec3 const & color)
    {
      glColor3f(color.x, color.y, color.z);
    }


    GLUTILS_API static void AsColor(vec4 const & color)
    {
      glColor4f(color.r, color.g, color.b, color.a);
    }


    GLUTILS_API static void AsNormal(vec3 const & normal)
    {
      glNormal3f(normal.x, normal.y, normal.z);
    }


    GLUTILS_API static void AsVertex(vec3 const & vertex)
    {
      glVertex3f(vertex.x, vertex.y, vertex.z);
    }


    GLUTILS_API static vec3 GetNormal(vec3 const & vertexA, vec3 const & vertexB, vec3 const & vertexC)
    {
      return triangleNormal(vertexA, vertexB, vertexC);
    }



    GLUTILS_API static void RenderQuadStrip(std::vector<vec3> const & vertices)
    {
      assert(vertices.size() % 2 == 0 && vertices.size() > 3);
      glBegin(GL_QUAD_STRIP);
        AsNormal(GetNormal(vertices[0], vertices[1], vertices[2]));
        AsVertex(vertices[0]);
        AsVertex(vertices[1]);
        for (size_t i = 2; i < vertices.size(); i = i + 2) {
          AsNormal(GetNormal(vertices[i-2], vertices[i-1], vertices[i]));
          AsVertex(vertices[i]);
          AsVertex(vertices[i+1]);
        }
      glEnd();
    }


    GLUTILS_API static void RenderLine(std::vector<vec3> const & vertices)
    {
      assert(vertices.size() > 1);
      glBegin(GL_LINE_STRIP);
      std::for_each(begin(vertices), end(vertices), [&](vec3 const & vertex) {
        AsVertex(vertex);
      });
      glEnd();
    }


    //note: slow
    GLUTILS_API static void RenderPolygon(std::vector<vec3> const & vertices)
    {
      assert(vertices.size() > 1);
      glBegin(GL_POLYGON);
      std::for_each(begin(vertices), end(vertices), [&](vec3 const & vertex) {
        AsVertex(vertex);
      });
      glEnd();
    }



    GLUTILS_API static bool RenderSphere(vec3 const & origin, float radius, unsigned int rings, unsigned int sectors);

    GLUTILS_API static bool RenderPrism(std::vector<vec3> const & bottomContour, std::vector<vec3> const & topContour, const vec3 & bottomCenterPoint, const vec3 & topCenterPoint);

    //if topColor is specified, the color parameter is used for the bottom
    GLUTILS_API static bool RenderColoredPrism(std::vector<vec3> const & bottomContour, std::vector<vec3> const & topContour, const vec3 & bottomCenterPoint, const vec3 & topCenterPoint, const vec4 & color, const vec4 & topColor);

  private:
    CRenderUtils(void){}; //static class
    ~CRenderUtils(void){}; //static class

    GLUTILS_API static bool RenderPrism(std::vector<vec3> const & bottomContour, std::vector<vec3> const & topContour, bool renderCaps, bool useColor, const vec3 * bottomCenterPoint, const vec3 * topCenterPoint, const vec4 * bottomColor, const vec4 * topColor);

  };



  //this class can be used to push a matrix on the stack and pop it back on destruction
  class CGLPushMatrix
  {
  public:
     GLUTILS_API CGLPushMatrix(void);
     GLUTILS_API CGLPushMatrix(GLenum matrix);
     GLUTILS_API ~CGLPushMatrix(void);

  private:
    GLenum mCurrentMatrix;
  };



  //this class can be used to push an attribut on the stack and pop it back on destruction
  class CGLPushAttribut
  {
  public:
     GLUTILS_API CGLPushAttribut(GLenum attribute);
     GLUTILS_API ~CGLPushAttribut(void);
  };
}