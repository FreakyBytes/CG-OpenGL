#pragma once

#include "GLUtilsDefines.h"
#include "Plane.h"
#include <glm/glm.hpp>

using namespace glm;

class CAABox;

namespace GLUtils {

class CViewFrustum 
{
public:

  static enum {OUTSIDE, INTERSECT, INSIDE};

  GLUTILS_API CViewFrustum() : mFrustumCullingOn(true) {}
  GLUTILS_API ~CViewFrustum() {}

  GLUTILS_API void SetFrustum(const float * m);
  GLUTILS_API void SetCamInternals(float angle, float ratio, float nearD, float farD);
  GLUTILS_API void SetCamDef(vec3 const & p, vec3 const & l, vec3 const & u);
  GLUTILS_API void SetCamDef(dvec3 const & p, dvec3 const & l, dvec3 const & u);
  GLUTILS_API bool PointInFrustum(vec3 const & p) const;
  GLUTILS_API bool SphereInFrustum(vec3 const & p, float raito) const;
  GLUTILS_API bool BoxInFrustum(CAABox const & box) const;
  GLUTILS_API CPlane const & GetPlane(unsigned int index) const {return mPlanes[index];};
  GLUTILS_API bool Intersects(const glm::vec3& bbmin, const glm::vec3& bbmax) const;

  GLUTILS_API void DrawPlanes() const;
  GLUTILS_API void DrawNormals() const;
  GLUTILS_API void ToggleFrustumCulling() {mFrustumCullingOn = !mFrustumCullingOn;}
  GLUTILS_API void SetFromGL(void);

private:

  CPlane mPlanes[6];
  vec3 mNearTopLeft;
  vec3 mNearTopRight;
  vec3 mNearBottomLeft;
  vec3 mNearBottomRight;
  vec3 mFarTopLeft;
  vec3 mFarTopRight;
  vec3 mFarBottomLeft;
  vec3 mFarBottomRight;

  float mNearD;
  float mFarD;
  float mRatio;
  float mAngle;
  float mTang;
  float mNW;
  float mNH;
  float mFW;
  float mFH;

  //vec3 mX;
  //vec3 mY;
  //vec3 mZ;
  vec3 mCameraPosition;

  float mSphereFactorX; 
  float mSphereFactorY;

  enum {
    TOP = 0, 
    BOTTOM, 
    LEFT,
    RIGHT,
    NEARP, 
    FARP
  };

  bool mFrustumCullingOn;
};

}