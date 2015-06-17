#include "GuiPrecompiled.h"
#include "Effects.h"

#include <iostream>
#include "GLShaderProgram.h"
#include "Helper.h"
#include "GLEffectManager.h"
#include "ViewEffects.h"



using namespace glm;

namespace Gui {

//paths
const std::string CTerrainEffect::sTexturePath = ("../../../media/texture/");

const std::string cHatchTextures[] = { "../../../media/hatch/hatch_0.png",              //Simple Cross Hatch Texture
                                              "../../../media/hatch/hatch_1.png", 
                                              "../../../media/hatch/hatch_2.png", 
                                              "../../../media/hatch/hatch_3.png", 
                                              "../../../media/hatch/hatch_4.png", 
                                              "../../../media/hatch/hatch_5.png"};

const std::vector<std::string> CTerrainEffect::sHatchTextures(cHatchTextures, cHatchTextures + sizeof(cHatchTextures) / sizeof(cHatchTextures[0]));

/************************************************************************/
/* Terrain Effect                                                       */
/************************************************************************/

#ifdef USESTATICTEXTURES
  GLUtils::CGLTexture CTerrainEffect::mNormalTexture;       //texture containing normals of the terrain
  GLUtils::CGLTexture CTerrainEffect::mAOTexture;           //texture containing ambient occlusion values of the terrain
#endif

  CTerrainEffect::CTerrainEffect(GLUtils::CGLEffectManager & effectManager, std::string const & path)
    : CGLSimpleEffect(&effectManager, path)
    , mInitialized(false)
    , mBoundings(1.0)
    , mMinZ(1.0)
    , mMaxZ(1.0)
  {
    RegisterTextures(&mNormalTexture, &mAOTexture);
  }


  void CTerrainEffect::SetTerrainBoundings(glm::vec3 const & minBounding, glm::vec3 const & maxBounding)
  {
    mBoundings = vec4(minBounding.x, minBounding.y, maxBounding.x, maxBounding.y);
    mMinZ = minBounding.z;
    mMaxZ = maxBounding.z;
  }



  void CTerrainEffect::ReloadTerrainTextures(std::string const & terrainPath)
  {
    //create path
    size_t slashPosition = terrainPath.find_last_of("/");
    if (slashPosition == std::string::npos) {
      slashPosition = terrainPath.find_last_of("\\");
      if (slashPosition == std::string::npos) {
        slashPosition = 0;
      }
    }
    auto extensionPosition = terrainPath.find_last_of(".") - 1;
    std::string file = terrainPath.substr(slashPosition + 1, extensionPosition - slashPosition); //without ending

    std::string path = terrainPath.substr(0, slashPosition);

    slashPosition = path.find_last_of("/");
    if (slashPosition == std::string::npos) {
      slashPosition = path.find_last_of("\\");
      if (slashPosition == std::string::npos) {
        slashPosition = 0;
      }
    }

    std::string mediaFolder = path.substr(0, slashPosition + 1);

    std::string commonPath = mediaFolder + "texture/" + file.c_str();
    std::string normalMapPath = commonPath + "_normals.png";
    std::string aoMapPath = commonPath + "_ao.png";


    //load texture
    mNormalTexture.UnloadFromCPU();
    mNormalTexture.SetFileName(normalMapPath);
    mAOTexture.UnloadFromCPU();
    mAOTexture.SetFileName(aoMapPath);

    mResourcesLoaded = false;

    if (GetEffectManager().IsTextureLoadedLazy()) {
      GetEffectManager().AddResourceToLoadingQueue(mNormalTexture, *this);
      GetEffectManager().AddResourceToLoadingQueue(mAOTexture, *this);
    }
    else {
      if (!mNormalTexture.IsLoadedToCPU()) {
        mNormalTexture.LoadFromFile(false);
      }

      if (!mAOTexture.IsLoadedToCPU()) {
        mAOTexture.LoadFromFile(false);
      }
    }

    ReloadOtherTerrainBasedTextures(commonPath);
  }

  
}