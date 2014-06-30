#include "GuiPrecompiled.h"
#include "TextureEffect.h"

#include "GLEffectManager.h"
#include "Helper.h"

namespace Gui {

  /************************************************************************/
  /* Terrain Textured + Normal + Ambient Occlusion Effect                                      */
  /************************************************************************/

  CTexturedTerrainEffect::CTexturedTerrainEffect(GLUtils::CGLEffectManager & effectManager)
    : CTerrainEffect(effectManager, "../../../shader/viewer/shader.glsl")
  {
    RegisterTextures(&mTerrainTexture);
  }



  CTexturedTerrainEffect::~CTexturedTerrainEffect(void)
  {
  }



  void CTexturedTerrainEffect::ReloadOtherTerrainBasedTextures(std::string const & mediaBasePath)
  {
    std::string texturePath = mediaBasePath + "_texture.png";

    //load texture
    mTerrainTexture.UnloadFromCPU();
    mTerrainTexture.SetFileName(texturePath);

    mResourcesLoaded = false;

    if (GetEffectManager().IsTextureLoadedLazy()) {
      GetEffectManager().AddResourceToLoadingQueue(mTerrainTexture, *this);
    }
    else {
      if (!mTerrainTexture.IsLoadedToCPU()) {
        mTerrainTexture.LoadFromFile(false);
      }
    }
  }



  void CTexturedTerrainEffect::Initialize(vec2 const & minBounding, vec2 const & maxBounding)
  {
    std::cout << "Preparing Textured Terrain Shader Effect" << std::endl;

    mBoundings = vec4(minBounding.x, minBounding.y, maxBounding.x, maxBounding.y);

    Deactivate();

    mInitialized = true;
  }



  void CTexturedTerrainEffect::Activate(void)
  {
    if (mInitialized && GetShaderProgram().IsValid() && GetShaderProgram().IsLinked() && IsFullyLoaded()) {

      //textures
      mNormalTexture.LoadToGPU();
      mAOTexture.LoadToGPU();
      mTerrainTexture.LoadToGPU();

      if (GLUtils::CHelper::CheckForError()) throw std::runtime_error("TexturedTerrainEffect Activation: textures could not be loaded to GPU");

      //Uniforms
      GetShaderProgram().Use();
      GetShaderProgram().SetUniform("minMaxBound", mBoundings);                         //boundings of the terrain
      GetShaderProgram().SetUniform("normalMap", 0);                                    //sampler for normal map
      GetShaderProgram().SetUniform("aoMap", 1);                                        //sampler for ambient occlusion map
      GetShaderProgram().SetUniform("terrainTexture", 2);                                   //sampler for terrain texture
    }
  }



  void CTexturedTerrainEffect::Deactivate(void)
  {
    mNormalTexture.UnloadFromGPU();
    mAOTexture.UnloadFromGPU();
    mTerrainTexture.UnloadFromGPU();
  }


  void CTexturedTerrainEffect::BindTextures(void)
  {
    //textures
    glActiveTexture(GL_TEXTURE0);
    mNormalTexture.BindTexture();

    glActiveTexture(GL_TEXTURE1);
    mAOTexture.BindTexture();

    glActiveTexture(GL_TEXTURE2);
    mTerrainTexture.BindTexture();
  }



  void CTexturedTerrainEffect::UnbindTextures(void)
  {
    //textures
    glActiveTexture(GL_TEXTURE0);
    mNormalTexture.UnbindTexture();

    glActiveTexture(GL_TEXTURE1);
    mAOTexture.UnbindTexture();

    glActiveTexture(GL_TEXTURE2);
    mTerrainTexture.UnbindTexture();
  }

} //end of namespace