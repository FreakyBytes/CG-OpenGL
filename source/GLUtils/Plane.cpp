#include "GLUtilsPrecompiled.h"
#include "Plane.h"

namespace GLUtils {


CPlane::CPlane(vec3 const & v1,  vec3 const & v2,  vec3 const & v3) 
{
  Set3Points(v1,v2,v3);
}



void CPlane::Set3Points(vec3 const & v1,  vec3 const & v2,  vec3 const & v3) 
{
  vec3 aux1, aux2;

  aux1 = v1 - v2;
  aux2 = v3 - v2;

  mNormal = cross(aux2, aux1);

  mNormal = normalize(mNormal);
  mPoint = v2;
  mD = -(dot(mNormal, mPoint));
}

void CPlane::SetNormalAndPoint(vec3 const & normal, vec3 const & point)
{
  mNormal = normalize(normal);
  mPoint = point;
  mD = -(dot(mNormal, point));
}

void CPlane::SetCoefficients(float a, float b, float c, float d) 
{

  //compute the lenght of the vector
  float l = length(vec3(a,b,c));
  // normalize the vector
  mNormal = vec3(a/l,b/l,c/l);
  // and divide d by th length as well
  mD = d/l;
}


  

float CPlane::Distance(vec3 const & p) const 
{
   
  return (dot(mNormal, p) + mD);
}


}