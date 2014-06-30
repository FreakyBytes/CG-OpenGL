#include "GLUtilsPrecompiled.h"
#include "ViewFrustum.h"

#include "GLUtilities.h"

#define HALF_ANG2RAD 3.14159265358979323846f/360.0f 
#define ANG2RAD 3.14159265358979323846f/180.0f

#define m(col,row)  m[row*4+col]


namespace GLUtils {


void CViewFrustum::SetFrustum(const float * m) 
{

  mPlanes[NEARP].SetCoefficients( m(2,0) + m(3,0),
                                  m(2,1) + m(3,1),
                                  m(2,2) + m(3,2),
                                  m(2,3) + m(3,3));

  mPlanes[FARP].SetCoefficients( -m(2,0) + m(3,0),
                                 -m(2,1) + m(3,1),
                                 -m(2,2) + m(3,2),
                                 -m(2,3) + m(3,3));
  mPlanes[BOTTOM].SetCoefficients(m(1,0) + m(3,0),
                                  m(1,1) + m(3,1),
                                  m(1,2) + m(3,2),
                                  m(1,3) + m(3,3));
  mPlanes[TOP].SetCoefficients(  -m(1,0) + m(3,0),
                                 -m(1,1) + m(3,1),
                                 -m(1,2) + m(3,2),
                                 -m(1,3) + m(3,3));
  mPlanes[LEFT].SetCoefficients(  m(0,0) + m(3,0),
                                  m(0,1) + m(3,1),
                                  m(0,2) + m(3,2),
                                  m(0,3) + m(3,3));
  mPlanes[RIGHT].SetCoefficients(-m(0,0) + m(3,0),
                                 -m(0,1) + m(3,1),
                                 -m(0,2) + m(3,2),
                                 -m(0,3) + m(3,3));
}

#undef m



void CViewFrustum::SetFromGL(void)
{
  float mv[16];
  float p[16];
  float mvp[16];
  glGetFloatv(GL_PROJECTION_MATRIX,p);
  glGetFloatv(GL_MODELVIEW_MATRIX,mv);

  //manual matrix mult for debugging purpose
  for (int i=0;i<4;i++) {
    for (int j = 0;j < 4;j++) {
      mvp[i*4+j] = 0.0;
      for (int k = 0; k < 4; k++) {
        mvp[i*4+j] += mv[i*4+k] * p[k*4+j];
      }
    }
  }

  SetFrustum(mvp);
}



/*
void CFrustum::SetCamInternals(float angle, float ratio, float nearD, float farD) 
{

  // store the information
  mRatio = ratio;
  mAngle = angle * HALF_ANG2RAD;
  mNearD = nearD;
  mFarD = farD;

  // compute width and height of the near and far plane sections
  mTang = tan(mAngle);
  mSphereFactorY = 1.0f/cos(mAngle); //tang * sin(this->angle) + cos(this->angle);

  float anglex = atan(mTang*ratio);
  mSphereFactorX = 1.0f/cos(anglex); //tang*ratio * sin(anglex) + cos(anglex);

  mNH = nearD * mTang;
  mNW = mNH * ratio; 

  mFH = farD * mTang;
  mFW = mFH * ratio;
}



void CFrustum::SetCamDef(vec3 const & p, vec3 const & l, vec3 const & u) 
{
  vec3 mX, mY, mZ;

  mCameraPosition = p;

  // compute the Z axis of camera
  mZ = normalize(p - l);

  // X axis of camera of given "up" vector and Z axis
  mX = normalize(cross(u, mZ));

  // the real "up" vector is the cross product of Z and X
  mY = cross(mZ, mX);

  // compute the center of the near and far planes
  vec3 nc = p - mZ * mNearD;
  vec3 fc = p - mZ * mFarD;

  // compute the 8 corners of the frustum
  mNearTopLeft = nc + mY * mNH - mX * mNW;
  mNearTopRight = nc + mY * mNH + mX * mNW;
  mNearBottomLeft = nc - mY * mNH - mX * mNW;
  mNearBottomRight = nc - mY * mNH + mX * mNW;

  mFarTopLeft = fc + mY * mFH - mX * mFW;
  mFarBottomRight = fc - mY * mFH + mX * mFW;
  mFarTopRight = fc + mY * mFH + mX * mFW;
  mFarBottomLeft = fc - mY * mFH - mX * mFW;

  // compute the six planes
  // the function set3Points asssumes that the points
  // are given in counter clockwise order
  mPlanes[TOP].Set3Points(mNearTopRight,mNearTopLeft, mFarTopLeft);
  mPlanes[BOTTOM].Set3Points(mNearBottomLeft, mNearBottomRight, mFarBottomRight);
  mPlanes[LEFT].Set3Points(mNearTopLeft, mNearBottomLeft, mFarBottomLeft);
  mPlanes[RIGHT].Set3Points(mNearBottomRight, mNearTopRight, mFarBottomRight);

  mPlanes[NEARP].SetNormalAndPoint(-mZ,nc);
  mPlanes[FARP].SetNormalAndPoint(mZ,fc);

  vec3 aux = (nc + mY * mNH) - p;
  vec3 normal = cross(aux, mX);
  mPlanes[TOP].SetNormalAndPoint(normal,nc+mY*mNH);

  aux = (nc - mY*mNH) - p;
  normal = cross(mX, aux);
  mPlanes[BOTTOM].SetNormalAndPoint(normal,nc-mY*mNH);
  
  aux = (nc - mX*mNW) - p;
  normal = cross(aux, mY);
  mPlanes[LEFT].SetNormalAndPoint(normal,nc-mX*mNW);

  aux = (nc + mX*mNW) - p;
  normal = cross(mY, aux);
  mPlanes[RIGHT].SetNormalAndPoint(normal,nc+mX*mNW);
}

*/


void CViewFrustum::SetCamInternals(float angle, float ratio, float nearD, float farD) {

  mRatio = ratio;
  mAngle = angle;
  mNearD = nearD;
  mFarD = farD;

  mTang = tan(angle * HALF_ANG2RAD) ;
  mNH = nearD * mTang;
  mNW = mNH * ratio; 
  mFH = farD  * mTang;
  mFW = mFH * ratio;


}



void CViewFrustum::SetCamDef(dvec3 const & p, dvec3 const & l, dvec3 const & u) 
{
  const vec3 pF = vec3(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z));
  const vec3 lF = vec3(static_cast<float>(l.x), static_cast<float>(l.y), static_cast<float>(l.z));
  const vec3 uF = vec3(static_cast<float>(u.x), static_cast<float>(u.y), static_cast<float>(u.z));
  SetCamDef(pF, lF, uF);
}



void CViewFrustum::SetCamDef(vec3 const & p, vec3 const & l, vec3 const & u) {

  vec3 dir,nc,fc,X,Y,Z;

  Z = normalize(p - l);

  X = normalize(cross(u, Z));

  Y = cross(Z, X);

  nc = p - Z * mNearD;
  fc = p - Z * mFarD;

  mNearTopLeft =      nc + Y * mNH - X * mNW;
  mNearTopRight =     nc + Y * mNH + X * mNW;
  mNearBottomLeft =   nc - Y * mNH - X * mNW;
  mNearBottomRight =  nc - Y * mNH + X * mNW;

  mFarTopLeft =     fc + Y * mFH - X * mFW;
  mFarTopRight =    fc + Y * mFH + X * mFW;
  mFarBottomLeft =  fc - Y * mFH - X * mFW;
  mFarBottomRight = fc - Y * mFH + X * mFW;


  mPlanes[TOP].Set3Points(mNearTopRight,mNearTopLeft,mFarTopLeft);
  mPlanes[BOTTOM].Set3Points(mNearBottomLeft,mNearBottomRight,mFarBottomRight);
  mPlanes[LEFT].Set3Points(mNearTopLeft,mNearBottomLeft,mFarBottomLeft);
  mPlanes[RIGHT].Set3Points(mNearBottomRight,mNearTopRight,mFarBottomRight);
  mPlanes[NEARP].Set3Points(mNearTopLeft,mNearTopRight,mNearBottomRight);
  mPlanes[FARP].Set3Points(mFarTopRight,mFarTopLeft,mFarBottomLeft);
}


bool CViewFrustum::Intersects(const glm::vec3& bbmin, const glm::vec3& bbmax) const 
{
  if (mFrustumCullingOn) {

    for(int i=0; i < 6; i++) {
      
      const CPlane& plane = GetPlane(i);
      glm::vec3 p = glm::vec3(
        plane.GetNormal().x >= 0.f ? bbmax.x : bbmin.x,
        plane.GetNormal().y >= 0.f ? bbmax.y : bbmin.y,
        plane.GetNormal().z >= 0.f ? bbmax.z : bbmin.z);

      if (plane.Distance(p) < 0.f)  {
        return false;
      }
    }

    /*
    glm::vec3 c = (bbmin + bbmax) * 0.5f;
    float r = glm::distance(bbmin, bbmax)*0.5f;
    return SphereInFrustum(c, r);
    */
  }
  return true;
}



bool CViewFrustum::PointInFrustum(vec3 const & p) const
{
  int result = INSIDE;
  for(int i=0; i < 6; i++) {

    if (mPlanes[i].Distance(p) < 0)
      return OUTSIDE;
  }
  return(result == INSIDE);

}


bool CViewFrustum::SphereInFrustum(vec3 const & p, float raio) const
{
  int result = INSIDE;
  float distance;

  for(int i=0; i < 6; i++) {
    distance = mPlanes[i].Distance(p);
    if (distance < -raio)
      return OUTSIDE;
    else if (distance < raio)
      result =  INTERSECT;
  }
  return (result != OUTSIDE);

}



void CViewFrustum::DrawPlanes() const{
  
  glBegin(GL_QUADS);

  //near plane
    glColor4d(1.0,1.0,0.0, 0.3);
    glVertex3f(mNearTopLeft.x,mNearTopLeft.y,mNearTopLeft.z);
    glVertex3f(mNearTopRight.x,mNearTopRight.y,mNearTopRight.z);
    glVertex3f(mNearBottomRight.x,mNearBottomRight.y,mNearBottomRight.z);
    glVertex3f(mNearBottomLeft.x,mNearBottomLeft.y,mNearBottomLeft.z);

  //far plane
    glColor4d(0.0,0.0,1.0, 0.3);
    glVertex3f(mFarTopRight.x,mFarTopRight.y,mFarTopRight.z);
    glVertex3f(mFarTopLeft.x,mFarTopLeft.y,mFarTopLeft.z);
    glVertex3f(mFarBottomLeft.x,mFarBottomLeft.y,mFarBottomLeft.z);
    glVertex3f(mFarBottomRight.x,mFarBottomRight.y,mFarBottomRight.z);

  //bottom plane
    glColor4d(0.0,1.0,0.0, 0.3);
    glVertex3f(mNearBottomLeft.x,mNearBottomLeft.y,mNearBottomLeft.z);
    glVertex3f(mNearBottomRight.x,mNearBottomRight.y,mNearBottomRight.z);
    glVertex3f(mFarBottomRight.x,mFarBottomRight.y,mFarBottomRight.z);
    glVertex3f(mFarBottomLeft.x,mFarBottomLeft.y,mFarBottomLeft.z);

  //top plane
    glColor4d(0.0,1.0,1.0, 0.3);
    glVertex3f(mNearTopRight.x,mNearTopRight.y,mNearTopRight.z);
    glVertex3f(mNearTopLeft.x,mNearTopLeft.y,mNearTopLeft.z);
    glVertex3f(mFarTopLeft.x,mFarTopLeft.y,mFarTopLeft.z);
    glVertex3f(mFarTopRight.x,mFarTopRight.y,mFarTopRight.z);

  //left plane
    glColor4d(1.0,0.0,0.0, 0.3);
    glVertex3f(mNearTopLeft.x,mNearTopLeft.y,mNearTopLeft.z);
    glVertex3f(mNearBottomLeft.x,mNearBottomLeft.y,mNearBottomLeft.z);
    glVertex3f(mFarBottomLeft.x,mFarBottomLeft.y,mFarBottomLeft.z);
    glVertex3f(mFarTopLeft.x,mFarTopLeft.y,mFarTopLeft.z);

  // right plane
    glColor4d(1.0,0.0,1.0, 0.3);
    glVertex3f(mNearBottomRight.x,mNearBottomRight.y,mNearBottomRight.z);
    glVertex3f(mNearTopRight.x,mNearTopRight.y,mNearTopRight.z);
    glVertex3f(mFarTopRight.x,mFarTopRight.y,mFarTopRight.z);
    glVertex3f(mFarBottomRight.x,mFarBottomRight.y,mFarBottomRight.z);

  glEnd();

}

void CViewFrustum::DrawNormals() const {

  vec3 a,b;
  GLUtils::CGLPushAttribut scopedLineWidth(GL_LINE_BIT);
  glLineWidth(3.f);
  glColor3d(1.0,1.0,1.0);
  glBegin(GL_LINES);

    for (int i=0; i < 6; ++i) {
      const CPlane& plane = GetPlane(i);

      a = -plane.GetD()*plane.GetNormal();
      b = a+plane.GetNormal();
      glVertex3fv(&a.x);
      glVertex3fv(&b.x);
    }

    /*

    // near
    a = (mNearTopRight + mNearTopLeft + mNearBottomRight + mNearBottomLeft) * 0.25f;
    b = a + mPlanes[NEARP].GetNormal();
    glVertex3f(a.x,a.y,a.z);
    glVertex3f(b.x,b.y,b.z);

    // far
    a = (mFarTopRight + mFarTopLeft + mFarBottomRight + mFarBottomLeft) * 0.25f;
    b = a + mPlanes[FARP].GetNormal();
    glVertex3f(a.x,a.y,a.z);
    glVertex3f(b.x,b.y,b.z);

    // left
    a = (mFarTopLeft + mFarBottomLeft + mNearBottomLeft + mNearTopLeft) * 0.25f;
    b = a + mPlanes[LEFT].GetNormal();
    glVertex3f(a.x,a.y,a.z);
    glVertex3f(b.x,b.y,b.z);
    
    // right
    a = (mFarTopRight + mNearBottomRight + mFarBottomRight + mNearTopRight) * 0.25f;
    b = a + mPlanes[RIGHT].GetNormal();
    glVertex3f(a.x,a.y,a.z);
    glVertex3f(b.x,b.y,b.z);
    
    // top
    a = (mFarTopRight + mFarTopLeft + mNearTopRight + mNearTopLeft) * 0.25f;
    b = a + mPlanes[TOP].GetNormal();
    glVertex3f(a.x,a.y,a.z);
    glVertex3f(b.x,b.y,b.z);
    
    // bottom
    a = (mFarBottomRight + mFarBottomLeft + mNearBottomRight + mNearBottomLeft) * 0.25f;
    b = a + mPlanes[BOTTOM].GetNormal();
    glVertex3f(a.x,a.y,a.z);
    glVertex3f(b.x,b.y,b.z);

    */

  glEnd();
}


}