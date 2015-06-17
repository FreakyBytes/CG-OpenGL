#pragma once

#include "GuiDefines.h"
#include "ViewFrustum.h"



namespace Gui {

  enum ViewMode
  {
    PrimaryView,
    NavigationView,
    Count
  };

  class CView;

  //this class is wrapps up a bundle of state variables, controlling the primary view
  class CViewStates
  {
    public:
      GUI_API CViewStates(CView & parentView);
      GUI_API ~CViewStates(void) {}

      GUI_API void ToggleWireframeMode(void) { mWireframeMode = !mWireframeMode; }
      GUI_API void ToggleBoundingBoxMode(void) { mShowBoundingBoxes = !mShowBoundingBoxes; }
      GUI_API void ToggleShowOutline(void) { mShowOutline = !mShowOutline; }
      GUI_API void IncreaseTolerance(void);
      GUI_API void DecreaseTolerance(void);
      GUI_API void ToggleFrustumCulling(void) { mViewFrustum.ToggleFrustumCulling(); }
      GUI_API void InitializeFrustum(float angle, float ratio, float nearD, float farD) { mViewFrustum.SetCamInternals(angle, ratio, nearD, farD); }
      GUI_API void SetViewFrustumToCamera(dvec3 const & p, dvec3 const & l, dvec3 const & u) { mViewFrustum.SetCamDef(p, l, u); }

      GUI_API glm::vec3 const & GetClearColor(void) const { return mClearColor; }
      GUI_API void SetClearColor(glm::vec3 const & clearColor) { mClearColor = clearColor; }

      GUI_API double GetFieldOfView(void) const { return mFieldOfView; }
      GUI_API void SetFieldOfView(double value);
      GUI_API float GetNearPlane(void) const { return mNearPlane; }
      GUI_API void SetNearPlane(float value);
      GUI_API float GetFarPlane(void) const { return mFarPlane; }
      GUI_API void SetFarPlane(float value);
      GUI_API glm::dvec3 GetInitialEyePosition(void) const { return mEyePostion; }
      GUI_API glm::dvec3 GetInitialLookAtVector(void) const { return mLookAt; }

      GUI_API float GetTolerance(void) const { return mTolerance; }
      GUI_API bool ShowOutline(void) const { return mShowOutline; }
      GUI_API bool GetWireframeMode(void) const { return mWireframeMode; }
      GUI_API bool ShowBounds(void) const { return mShowBoundingBoxes; }

      GUI_API GLUtils::CViewFrustum & GetViewFrustum(void) { return mViewFrustum; }

      GUI_API double GetLastUpdate(void) const { return mLastUpdate; }
      GUI_API void SetLastUpdate(double time) { mLastUpdate = time; }
      GUI_API int GetFrame(void) const { return mFrame; }
      GUI_API void IncrementFrame(void) { ++mFrame; }
      GUI_API void ResetFrame(void) { mFrame = 0; }
      GUI_API double GetFPSTimeBase(void) const { return mFPSTimeBase; }
      GUI_API void SetFPSTimeBase(double time) { mFPSTimeBase = time; }

      GUI_API std::string const & GetTitelString(void) const { return mTitelString; }
      GUI_API void SetTitelString(std::string const & titel) { mTitelString = titel; }

      GUI_API glm::vec3 const & GetLightDirection(void) const { return mLightPosition; }
      GUI_API void SetLightDirection(glm::vec3 const & dir);

      GUI_API void SetLight(void);
    private:
      CViewStates(void); //forbidden

      //variables, that change the mode of the visualization
      bool mShowOutline;                //show outline of patches
      bool mWireframeMode;              //wireframe mode
      bool mShowBoundingBoxes;          //shows yellow bounding boxes around each patch
 

      //initial values
      double mFieldOfView;
      float  mNearPlane;
      float  mFarPlane;

      const glm::dvec3 mEyePostion;
      const glm::dvec3 mLookAt;

      //FPS Counter
      double mLastUpdate;           // time of the last update call
      int mFrame;                   //frame counter
      double mFPSTimeBase;          //part of the fps counter
      float mTolerance;

      glm::vec3 mLightPosition;

      GLUtils::CViewFrustum mViewFrustum; //the view frustum

      std::string mTitelString;   //the string on the titel of the main window

      glm::vec3 mClearColor;

      CView & mParentView;
  };

}