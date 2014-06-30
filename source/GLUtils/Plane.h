#pragma once

#include "GLUtilsDefines.h"

#include <glm/glm.hpp>

using namespace glm;

namespace GLUtils {


class CPlane
{
public:
  GLUTILS_API CPlane::CPlane(vec3 const & v1,  vec3 const & v2,  vec3 const & v3);
  GLUTILS_API CPlane::CPlane(void) {}
  GLUTILS_API CPlane::~CPlane() {}

  GLUTILS_API void Set3Points( vec3 const & v1,  vec3 const & v2,  vec3 const & v3);
  GLUTILS_API void SetNormalAndPoint(vec3 const & normal, vec3 const & point);
  GLUTILS_API void SetCoefficients(float a, float b, float c, float d);
  GLUTILS_API float Distance(vec3 const & p) const;

  GLUTILS_API vec3 const & GetNormal(void) const {return mNormal;}
  GLUTILS_API vec3 const & GetPoint(void) const {return mPoint;}
  GLUTILS_API float GetD(void) const {return mD;} 

private:
  vec3 mNormal;
  vec3 mPoint;
  float mD;
};

} //namespace Utils