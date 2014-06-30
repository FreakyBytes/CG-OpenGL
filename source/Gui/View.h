#pragma once

#include "GuiDefines.h"

#include <memory>
#include "KeyboardInterface.h"
#include "MouseInterface.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ViewEffects.h"
#include "ViewStates.h"

namespace GLUtils {
  class CBaseCamera;
}

namespace Gui {


  class CView
  {
    public:
      GUI_API CView(void);
      GUI_API virtual ~CView(void) {}

      GUI_API CKeyboardInterface & GetKeyboardInterface(void) { return *mKeyboardInterface.get(); }
      GUI_API CMouseInterface & GetMouseInterface(void) { return *mMouseInterface.get(); }

      GUI_API virtual void Display(void) = 0;
      GUI_API virtual void Reshape(int width, int height) = 0;
      GUI_API virtual void Finalize(void) = 0;
      GUI_API virtual bool Initialize(int argc = 0, char **argv = nullptr) = 0;

      GUI_API int GetWindowID(void) const { return mWindowID; }
      GUI_API int GetWindowWidth(void) const { return mWidth; }
      GUI_API int GetWindowHeight(void) const { return mHeight; }
      GUI_API void SetWindowWidth(int width) { mWidth = width; }
      GUI_API void SetWindowHeight(int height) { mHeight = height; }
      GUI_API void SetWindowSize(int width, int height) { SetWindowWidth(width); SetWindowHeight(height); }

      GUI_API CViewEffects & GetViewEffects(void) { return mEffects; }
      GUI_API CViewEffects const & GetViewEffects(void) const { return mEffects; }
      GUI_API CViewStates & GetViewStates(void) { return mStates; }
      GUI_API CViewStates const & GetViewStates(void) const { return mStates; }
      GUI_API GLUtils::CBaseCamera * GetViewCamera(void) { return mCamera.get(); }
      GUI_API GLUtils::CBaseCamera const * GetViewCamera(void) const { return mCamera.get(); }
      GUI_API void UpdateProjection(void) { mProjectionMatrixDirty = true; }

      GUI_API void GoIntoRenderLoop(void) { glutMainLoop(); }

      GUI_API void InitializeCallbacks(void);


  protected:
      std::shared_ptr<CKeyboardInterface> mKeyboardInterface;
      std::shared_ptr<CMouseInterface> mMouseInterface;

      int mWindowID;
      int mWidth;
      int mHeight;
      bool mProjectionMatrixDirty;
      bool mInitialized;


      std::shared_ptr<GLUtils::CBaseCamera> mCamera; //camera for 3D mode

    private:
      CViewStates mStates;                     //a collection of global variables for this view, e.g. wireframe mode etc.
      CViewEffects mEffects;                   //controls the effect used
  };

}