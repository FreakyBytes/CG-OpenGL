#pragma once

#include "GLEffect.h"
#include "GLTexture.h"
#include "GLRenderTarget.h"

//static textures that will be multiply used - effects could also use their member, but this would lead to a memory overhead
//comment the following line out, to use member textures instead of static textures
#define USESTATICTEXTURES

namespace GLUtils{
class CBaseCamera;
class CGLEffectManager;
class CGLShaderProgram;
}

namespace DataRendering {
  class CRadarTrackingDataRenderer;
}


namespace Gui {

  class CTerrainEffect : public GLUtils::CGLSimpleEffect
  {
    public:
      CTerrainEffect(GLUtils::CGLEffectManager & effectManager, std::string const & shaderPath = "");
      virtual ~CTerrainEffect(void) {}

      void SetTerrainBoundings(glm::vec3 const & minBounding, glm::vec3 const & maxBounding);
      void ReloadTerrainTextures(std::string const & terrainPath);

      bool IsInitialized(void) const {return mInitialized;}
      virtual void BindTextures(void) override {}

    protected:
      virtual void ReloadOtherTerrainBasedTextures(std::string const & mediaBasePath) {}


      glm::vec4 mBoundings;                     //2D boundaries of the terrain
      float mMinZ;                              //boundaries of the terrain
      float mMaxZ;                              //boundaries of the terrain
      bool mInitialized;                        //initialize method was called

      //const statics
      static const std::string sTexturePath;
      static const std::vector<std::string> sHatchTextures;

#ifdef USESTATICTEXTURES
      static GLUtils::CGLTexture mNormalTexture;       //texture containing normals of the terrain
      static GLUtils::CGLTexture mAOTexture;           //texture containing ambient occlusion values of the terrain
#else
      GLUtils::CGLTexture mNormalTexture;             //texture containing normals of the terrain
      GLUtils::CGLTexture mAOTexture;                 //texture containing ambient occlusion values of the terrain
#endif
  };

}