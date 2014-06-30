#include "TerrainPrecompiled.h"
#include "ErrorMetric.h"

namespace Terrain {

  CErrorMetric::CErrorMetric() 
  {
    SetViewparams(glm::radians(45.f), 720.f, 1.f);
  }



  CErrorMetric::CErrorMetric(float fov, float pixelsonfov, float maxerror) 
  {
    SetViewparams(fov, pixelsonfov, maxerror);
  }



  void CErrorMetric::SetViewparams(float fov, float pixelsonfov, float tau) 
  {

    float lambda  = pixelsonfov / (2.f * glm::tan(fov * 0.5f));
    mViewterm     = lambda / tau;
  }



  bool CErrorMetric::Evaluate(glm::vec3 const & center, float radius, float error) const 
  {

    glm::vec3 d = center - mEye;
    float mag2  = glm::dot(d, d);
    float dist  = mViewterm * error + radius;

    return (dist*dist) > mag2;
  }



  bool CErrorMetric::Evaluate(glm::vec3 const & bbmin, glm::vec3 const & bbmax, float error) const 
  {

    float mag2 = BBoxDistance(bbmin, bbmax);
    float dist = mViewterm * error;
    return (dist*dist) > mag2;
  }



  float CErrorMetric::BBoxDistance(glm::vec3 const & bbmin, glm::vec3 const & bbmax) const 
  {
    glm::vec3 d= glm::vec3(
        (mEye.x < bbmin.x) ? (bbmin.x - mEye.x) : (mEye.x - bbmax.x),
        (mEye.y < bbmin.y) ? (bbmin.y - mEye.y) : (mEye.y - bbmax.y),
        (mEye.z < bbmin.z) ? (bbmin.z - mEye.z) : (mEye.z - bbmax.z));

    return glm::dot(d, d);
  }

} //namespace hfc
