#include "GuiPrecompiled.h"
#include "KeyboardInterface.h"

#include <GL/freeglut.h>
#include "ViewStates.h"
#include "ViewEffects.h"
#include "PrimaryView.h"
#include "View.h"


namespace Gui {
  
  CKeyboardInterface::CKeyboardInterface(CPrimaryView & parentView)
    : mParentView(parentView)
  {
  }



  GLUtils::CameraKeys CKeyboardInterface::MapCameraKey(int key)
  {

    switch( key )
    {
      if (GLUT_ACTIVE_ALT != glutGetModifiers()) {
          case GLUT_KEY_LEFT:
            return  GLUtils::CAM_STRAFE_LEFT;
          case GLUT_KEY_RIGHT:
            return GLUtils::CAM_STRAFE_RIGHT;
          case GLUT_KEY_UP:
            return GLUtils::CAM_MOVE_FORWARD ;
          case GLUT_KEY_DOWN:
            return GLUtils::CAM_MOVE_BACKWARD;
          case GLUT_KEY_PAGE_DOWN:
            return GLUtils::CAM_MOVE_UP ;
          case GLUT_KEY_PAGE_UP:
            return GLUtils::CAM_MOVE_DOWN;
          case GLUT_KEY_HOME:
              return GLUtils::CAM_RESET;
          default:
            return GLUtils::CAM_UNKNOWN;
      } 
    }

    return GLUtils::CAM_UNKNOWN;
  }



  GLUtils::CameraKeys CKeyboardInterface::MapCameraKey(unsigned char key)
  {
    switch( key )
    {
      if (GLUT_ACTIVE_ALT != glutGetModifiers()) {
          case 'a':
              return GLUtils::CAM_STRAFE_LEFT;
          case 'd':
              return GLUtils::CAM_STRAFE_RIGHT;
          case 'w':
              return GLUtils::CAM_MOVE_FORWARD;
          case 's':
              return GLUtils::CAM_MOVE_BACKWARD;
          case 'q':
              return GLUtils::CAM_MOVE_DOWN;
          case 'e':
              return GLUtils::CAM_MOVE_UP;
          case '+':
            return GLUtils::CAM_ACCELERATE;
          case '-':
            return GLUtils::CAM_THROTTLE;
          default:
            return GLUtils::CAM_UNKNOWN;
      }
    }

    return GLUtils::CAM_UNKNOWN;
  }


  
  bool CKeyboardInterface::EvaluateSpecialKey(unsigned char key)
  {
    if (!mParentView.EvaluateActionKey(key, GLUT_ACTIVE_CTRL == glutGetModifiers(), GLUT_ACTIVE_ALT == glutGetModifiers())) {
      if (GLUT_ACTIVE_ALT == glutGetModifiers()) {
        switch (key) {
        case 'w':
          //mParentView.GetViewStates().ToggleWireframeMode();
          return true;
        case 'o':
          mParentView.GetViewStates().ToggleShowOutline();
          return true;
        case 'b':
          mParentView.GetViewStates().ToggleBoundingBoxMode();
          return true;
        case '+':
          mParentView.GetViewStates().IncreaseTolerance();
          return true;
        case '-':
          mParentView.GetViewStates().DecreaseTolerance();
          return true;
        case 'c':
          mParentView.GetViewStates().ToggleFrustumCulling();
          return true;
#ifdef USESHADER
        case 'p':
          mParentView.GetViewEffects().ToggleShading();
          return true;
#endif
        default:
          return false;
        }
      }
    }
    return false;
  }



  bool CKeyboardInterface::EvaluateSpecialKey(int key)
  {
#ifdef USESHADER
    switch(key) {

      case GLUT_KEY_F10:
        //recompile shader
        mParentView.GetViewEffects().RecompileEffect();
        return true;
      default: 
        return false;
    }
#endif
    return false;
  }


  void CKeyboardInterface::KeyPressed(unsigned char key, int x, int y)
  {
    if (!EvaluateSpecialKey(key)) {
      GLUtils::CameraKeys keyPressed = MapCameraKey(key);
      mParentView.GetViewCamera()->HandleKeys(keyPressed, true);
    }
  }



  void CKeyboardInterface::KeyPressed(int key, int x, int y)
  {
    if (!EvaluateSpecialKey(key)) {
      GLUtils::CameraKeys keyPressed = MapCameraKey(key);
      mParentView.GetViewCamera()->HandleKeys(keyPressed, true);
    }
  }


  void CKeyboardInterface::KeyReleased(unsigned char key, int x, int y)
  {
    GLUtils::CameraKeys keyReleased = MapCameraKey(key);
    mParentView.GetViewCamera()->HandleKeys(keyReleased, false);
  }


  void CKeyboardInterface::KeyReleased(int key, int x, int y)
  {
    GLUtils::CameraKeys keyReleased = MapCameraKey(key);
    mParentView.GetViewCamera()->HandleKeys(keyReleased, false);
  }


  //--------------------------------------------------------------------------

}