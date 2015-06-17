#include "GuiPrecompiled.h"
#include "ViewStates.h"

#include <glm/glm.hpp>
#include <glm/gtx/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Helper.h"
#include "View.h"
#include "GLCamera.h"

namespace Gui {


  CViewStates::CViewStates(CView & parentView)
    : mParentView(parentView)
    , mShowOutline(false)
    , mWireframeMode(false)
    , mShowBoundingBoxes(false)
#ifdef USELOD
    , mTolerance(0.9f)
#else
    , mTolerance(0.0f)
#endif
    , mLastUpdate(0.0)
    , mFrame(0)
    , mFPSTimeBase(0.0)
    , mTitelString("SysTAvio Demonstrator")
    , mFieldOfView(20.0)
    , mNearPlane(1.0f)
    , mFarPlane(100000.0f)
    , mEyePostion(0.0, 1.0, 400.0)
    , mLookAt(-0.5, 0.1, 399.90)
    , mLightPosition(1.0f, -1.0f, 1.0f)
    , mClearColor(0.0f,0.5f,0.6f)
  {
  }



  void CViewStates::IncreaseTolerance(void)
  {
    mTolerance += 0.1f;
    std::cout << "error tolerance is set to: " << mTolerance << std::endl;
  }



  void CViewStates::DecreaseTolerance(void)
  {
    mTolerance = glm::max(0.1f, mTolerance - 0.1f);
    std::cout << "error tolerance is set to: " << mTolerance << std::endl;
  }



  void CViewStates::SetLightDirection(glm::vec3 const & direction)
  {
    mLightPosition = direction;
  }



  void CViewStates::SetLight(void)
  {
    //transform light into world space
    const float convertedLight[4] = {mLightPosition.x, mLightPosition.z, -mLightPosition.y, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, convertedLight);
    GLUtils::CHelper::CheckForError();
  }

    
  void CViewStates::SetNearPlane(float value)
  { 
    mNearPlane = value; 
    mParentView.UpdateProjection(); 
  }



  void CViewStates::SetFarPlane(float value)
  { 
    mFarPlane = value; 
    mParentView.UpdateProjection(); 
  }


  void CViewStates::SetFieldOfView(double value)
  {
    mFieldOfView = value;
    mParentView.UpdateProjection();
  }
}

