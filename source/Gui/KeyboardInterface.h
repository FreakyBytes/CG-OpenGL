#pragma once

#include "GuiDefines.h"

#include "GLCamera.h"

namespace Gui {

  class CView;
  class CPrimaryView;
  class CMainMenueView;
  class CNavigationView;
  class CVerticalCutView;


  class CKeyboardInterface
  {
    public:
      CKeyboardInterface(CPrimaryView & parentView);
      virtual ~CKeyboardInterface(void) {}

      //glut interfaces
      virtual void KeyPressed(unsigned char key, int x, int y);
      virtual void KeyPressed(int key, int x, int y);
      virtual void KeyReleased(unsigned char key, int x, int y);
      virtual void KeyReleased(int key, int x, int y);

      CPrimaryView & GetParentView(void) { return mParentView; }

    private:

      GLUtils::CameraKeys MapCameraKey(int key);
      GLUtils::CameraKeys MapCameraKey(unsigned char key);

      bool EvaluateSpecialKey(unsigned char key);
      bool EvaluateSpecialKey(int key);


      CPrimaryView & mParentView;     //the view, owning this keyboard interface
  };

  
}