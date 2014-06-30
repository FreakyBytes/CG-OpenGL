#include "GuiPrecompiled.h"
#include "MouseInterface.h"

#include <GL/freeglut.h>
#include "PrimaryView.h"


namespace Gui {


  void CMouseInterface::MouseKey(int button, int state, int x, int y)
  {
    {
      GLUtils::CameraMouseKeys key = GLUtils::MouseUnknown;
      switch (button) {
      case GLUT_LEFT_BUTTON:
        key = GLUtils::MouseLeft;
        break;
      case GLUT_RIGHT_BUTTON:
        key = GLUtils::MouseRight;
        break;
      case GLUT_MIDDLE_BUTTON:
        key = GLUtils::MouseMiddle;
        break;
      default: 
        key = GLUtils::MouseUnknown;
      }

      mParentView.GetViewCamera()->HandleMouse(key, state == GLUT_DOWN, x, y);
    }
  }



  void CMouseInterface::MouseMove(int x, int y)
  {
    {
      mParentView.GetViewCamera()->HandleMouse(GLUtils::MouseUnknown, false, x, y);
    }
  }



  void CMouseInterface::MousePassiveMove(int x, int y)
  {

  }


  //--------------------------------------------------------------------------------------------------

}