#pragma once

#include "TerrainDefines.h"

#include <glm/glm.hpp>

namespace Terrain {


  //helper class to evaluate screen space error
  class CErrorMetric {
  public:
    TERRAIN_API CErrorMetric();
    TERRAIN_API CErrorMetric(float fov, float pixelsonfov, float maxerror);

    //get current view position
    TERRAIN_API glm::vec3 ViewPosition() const {return mEye;}

    //set current view position
    TERRAIN_API void SetViewPosition(glm::vec3 const & eye) {mEye = eye;}
    TERRAIN_API void SetViewPosition(glm::dvec3 const & eye) { mEye.x = static_cast<float>(eye.x); mEye.y = static_cast<float>(eye.y); mEye.z = static_cast<float>(eye.z); }
    //get the view term (lindstrom)
    TERRAIN_API float ViewTerm() const {return mViewterm;}
    //set view params and compute view term
    TERRAIN_API void SetViewparams(float fov, float pixelsonfov, float tau);

    //evaluate the error metric for the provided bounding sphere
    //returns true if the screen space error is below the maximum screen space error tau
    TERRAIN_API bool Evaluate(glm::vec3 const & center, float radius, float error) const;

    //evaluate the error metric for the provided bounding box
    //returns true if the screen space error is below the maximum screen space error tau
    TERRAIN_API bool Evaluate(glm::vec3 const & bbmin, glm::vec3 const & bbmax, float error) const;

  private:
    float BBoxDistance(glm::vec3 const & bbmin, glm::vec3 const & bbmax) const;

    glm::vec3 mEye;
    float mViewterm;
  };

}
