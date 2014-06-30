#pragma once

#include "GuiDefines.h"


namespace Gui {

  class CPrimaryView;
  class CMainMenueView;
  class CNavigationView;
  class CVerticalCutView;
  class CView;

  class CMouseInterface
  {
    public:
      CMouseInterface(CPrimaryView & parentView) : mParentView(parentView) {}
      virtual ~CMouseInterface(void) {}

      virtual void MouseKey(int button, int state, int x, int y);
      virtual void MouseMove(int x, int y);
      virtual void MousePassiveMove(int x, int y);
      CPrimaryView & GetParentView(void) { return mParentView; }

    private:
      CPrimaryView & mParentView;
  };

}