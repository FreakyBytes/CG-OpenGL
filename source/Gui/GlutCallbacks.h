#pragma once

#include "GuiDefines.h"
#include "GuiManager.h"
#include "PrimaryView.h"

namespace Gui {

  //static class for Glut callbacks
  class CGlutCallbacks
  {
    public:

      //callbacks for glut
      static void ReshapeCallback(int width, int height)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().Reshape(width, height);
      }



      static void DisplayCallback(void)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().Display();
      }



      static void KeyboardPressedCallback(unsigned char key, int x, int y)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().GetKeyboardInterface().KeyPressed(key, x, y);
      }


      static void KeyboardPressedCallback(int key, int x, int y)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().GetKeyboardInterface().KeyPressed(key, x, y);
      }



      static void KeyboardReleasedCallback(unsigned char key, int x, int y)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().GetKeyboardInterface().KeyReleased(key, x, y);
      }



      static void KeyboardReleasedCallback(int key, int x, int y)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().GetKeyboardInterface().KeyReleased(key, x, y);
      }



      static void MouseKeyCallback(int button, int state, int x, int y)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().GetMouseInterface().MouseKey(button, state, x, y);
      }



      static void MouseMoveCallback(int x, int y)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().GetMouseInterface().MouseMove(x, y);
      }


      static void MousePassiveMoveCallback(int x, int y)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().GetMouseInterface().MousePassiveMove(x, y);
      }



      static void FinalizeCallback(void)
      {
        CGuiManager::GetGuiManager().GetPrimaryView().Finalize();
      }


    private:
      CGlutCallbacks(void);   //forbidden - static class
      ~CGlutCallbacks(void);  //forbidden - static class
  };
}