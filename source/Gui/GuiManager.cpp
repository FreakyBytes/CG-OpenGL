#include "GuiPrecompiled.h"
#include "GuiManager.h"

#include "PrimaryView.h"


namespace Gui {

  //singleton
  static CGuiManager * sGuiManager = nullptr;

  static const std::string sNoDataLoaded ("Nothing Loaded");


  CGuiManager::CGuiManager(CPrimaryView & window)
    : mPrimaryView(window)
  {
  }



  CGuiManager::~CGuiManager(void)
  {
  }



  void CGuiManager::Destroy(void)
  {
    delete sGuiManager;
    sGuiManager = nullptr;
  }



  CGuiManager & CGuiManager::CreateGUI(CPrimaryView & window)
  {
    //singleton pattern
    if (sGuiManager == nullptr) {
      sGuiManager = new CGuiManager(window);
    }

    return *sGuiManager;
  }



  CGuiManager & CGuiManager::GetGuiManager(void)
  {
    assert(sGuiManager != nullptr);   //window does not exist (anymore)
    return *sGuiManager;
  }

}