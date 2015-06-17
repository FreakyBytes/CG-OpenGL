#include "Application.h"

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include "RasterTerrainModel.h"
#include "OpenGLWindow.h"

#include "GuiManager.h"

namespace Application {

  const std::string CApplication::mTerrainModelPath("../../../media/terrain/example.rlod");
  

  int CApplication::Run(int argc, char* argv[])
  {
    //starts application
    std::cout << "Loading terrain data-set..." << std::endl;

    //loading terrain model
    if (LoadTerrainModel(mTerrainModelPath.c_str())) {

      //creating render window
      mWindow.reset(new COpenGLWindow(*mModel.get()));
      mWindow->Initialize();
      mWindow->GoIntoRenderLoop();
    }

    return EXIT_SUCCESS;
  }



  bool CApplication::LoadTerrainModel(std::string const & path)
  {
    mModel.reset(new Terrain::CRasterTerrainModel);

    std::cout << "Loading new terrain model..." << std::endl;
    mModel->Init(path.c_str());
    std::cout << "Finished loading terrain model." << std::endl;

    return mModel.get() != nullptr;
  }
}