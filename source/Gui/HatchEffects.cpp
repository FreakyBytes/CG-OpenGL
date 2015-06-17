#include "GuiPrecompiled.h"
#include "HatchEffects.h"

#include "GLEffectManager.h"
#include "Helper.h"

namespace Gui {

  /************************************************************************/
/* Simple Hatch Effect                                                  */
/************************************************************************/

  CCrossHatchEffect::CCrossHatchEffect(GLUtils::CGLEffectManager & effectManager)
    : CTerrainEffect(effectManager, "../../../shader/viewer/CrossHatching.glsl")
  {
    RegisterTextures(&mHatchTexture);
  }


  CCrossHatchEffect::~CCrossHatchEffect(void)
  {
  }



  void CCrossHatchEffect::Initialize(vec2 const & minBounding, vec2 const & maxBounding)
  {
    std::cout << "Preparing Simple Cross Hatching Effect" << std::endl;

    mBoundings = vec4(minBounding.x, minBounding.y, maxBounding.x, maxBounding.y);

    //texture paths
    mHatchTexture.SetFileName(sHatchTextures);

    //load textures
    if (GetEffectManager().IsTextureLoadedLazy()) {
      GetEffectManager().AddResourceToLoadingQueue(mHatchTexture, *this);
    }
    else {
      if (!mHatchTexture.IsLoadedToCPU()) {
          mHatchTexture.LoadFromFile(false);
      }
    }
    

    Deactivate();

    mInitialized = true;
  }



  void CCrossHatchEffect::Activate(void)
  {
    if (mInitialized && GetShaderProgram().IsValid() && GetShaderProgram().IsLinked() && IsFullyLoaded()) {

      //textures
      mNormalTexture.LoadToGPU();
      mAOTexture.LoadToGPU();
      mHatchTexture.LoadToGPU();

      if (GLUtils::CHelper::CheckForError()) throw std::runtime_error("SimpleHatchEffect Activation: textures could not be loaded to GPU");

      //Uniforms
      GetShaderProgram().Use();
      GetShaderProgram().SetUniform("minMaxBound", mBoundings);                           //boundings of the terrain
      GetShaderProgram().SetUniform("normalMap",     0);                                  //sampler for normal map
      GetShaderProgram().SetUniform("aoMap",         1);                                  //sampler for ambient occlusion map
      GetShaderProgram().SetUniform("Hatch",         2);                                  //sampler hatch texture
    }
  }



  void CCrossHatchEffect::Deactivate(void)
  {
    mNormalTexture.UnloadFromGPU();
    mAOTexture.UnloadFromGPU();
    mHatchTexture.UnloadFromGPU();
  }


  void CCrossHatchEffect::BindTextures(void)
  {
    //textures
    glActiveTexture(GL_TEXTURE0);
    mNormalTexture.BindTexture();

    glActiveTexture(GL_TEXTURE1);
    mAOTexture.BindTexture();

    glActiveTexture(GL_TEXTURE2);
    mHatchTexture.BindTexture();
  }



  void CCrossHatchEffect::UnbindTextures(void)
  {
    //textures
    glActiveTexture(GL_TEXTURE0);
    mNormalTexture.UnbindTexture();

    glActiveTexture(GL_TEXTURE1);
    mAOTexture.UnbindTexture();

    glActiveTexture(GL_TEXTURE2);
    mHatchTexture.UnbindTexture();
  }



} //end of namespace