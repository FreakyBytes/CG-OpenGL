#pragma once

#include "GuiDefines.h"

#include "PrimaryView.h"
#include <memory>

namespace Terrain {
  class CTerrainModel;
}

namespace Gui {

  //This is the main window manager(singleton)
  class CGuiManager
  {
    public:
      //Get the singleton
      GUI_API static CGuiManager & CreateGUI(CPrimaryView & window);
      GUI_API static CGuiManager & GetGuiManager(void);

      //accessing windows
      CPrimaryView & GetPrimaryView(void) {return mPrimaryView;}
      CPrimaryView const & GetPrimaryView(void) const {return mPrimaryView;}

      static void Destroy(void);
      GUI_API int StartGui(void);
    private:
      CGuiManager(CPrimaryView & window);
      ~CGuiManager(void);


      CPrimaryView & mPrimaryView;
  };
}