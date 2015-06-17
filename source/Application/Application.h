
#include "TerrainModel.h"
#include <memory>

namespace Application {

  //forward declaration
  class COpenGLWindow;

  //this class encapsulates the whole application 
  //class is static
  class CApplication
  {
    public:
      CApplication(void) : mModel(nullptr){}
      ~CApplication(void) {}

      //starts the application (called in the main method)
      int Run(int argc, char* argv[]);
      bool LoadTerrainModel(std::string const & path);

    private:
      static const std::string mTerrainModelPath;       //default path to terrain model
      std::shared_ptr<Terrain::CTerrainModel> mModel;   //the terrain

      std::shared_ptr<COpenGLWindow> mWindow;           //OpenGL window



      
  };

}