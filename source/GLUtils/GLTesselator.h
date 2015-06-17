#pragma once

#include "GLUtilsDefines.h"


namespace GLUtils {
  class CGLTesselator {
    public:
      GLUTILS_API CGLTesselator(void);
      GLUTILS_API ~CGLTesselator(void);

      GLUTILS_API bool TesselateSimplePolygon(std::vector<glm::vec3> const & simplePolygon);
      GLUTILS_API bool TesselatePolygonWithHoles(std::vector<glm::vec3> const & exterior, std::vector<std::vector<glm::vec3>> const & interior);
    private:
      static const char* GetPrimitiveType(GLenum type);
      static void CALLBACK TesselationBeginCB(GLenum which);
      static void CALLBACK TesselationEndCB();
      static void CALLBACK TesselationVertexCB(const GLvoid *data);
      static void CALLBACK TesselationColoredVertexCB(const GLvoid *data);
      static void CALLBACK TesselationCombinedCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4], const GLfloat neighborWeight[4], GLdouble **outData);
      static void CALLBACK TesselationErrorCB(GLenum errorCode);

      static std::vector<std::vector<GLdouble>> mTempVertexData;     //used for self intersecting polygons
  };
}