#include "GuiPrecompiled.h"
#include "NormalAOEffects.h"

#include "GLEffectManager.h"
#include "Helper.h"

namespace Gui {

/************************************************************************/
/* Normal Ambient Occlusion Effect                                      */
/************************************************************************/

  CNormalAOLightEffect::CNormalAOLightEffect(GLUtils::CGLEffectManager & effectManager)
    : CTerrainEffect(effectManager, "../../../shader/viewer/NormalAO.glsl")
  {
  }



  CNormalAOLightEffect::CNormalAOLightEffect(GLUtils::CGLEffectManager & effectManager, std::string const & shader)
    : CTerrainEffect(effectManager, shader)
  {   
  }


  CNormalAOLightEffect::~CNormalAOLightEffect(void)
  {
  }



  void CNormalAOLightEffect::Initialize(vec2 const & minBounding, vec2 const & maxBounding)
  {
    std::cout << "Preparing Normal + Ambient Occlusion Shader Effect" << std::endl;

    mBoundings = vec4(minBounding.x, minBounding.y, maxBounding.x, maxBounding.y);

   
    Deactivate();

    mInitialized = true;
  }



  void CNormalAOLightEffect::Activate(void)
  {
    if (mInitialized && GetShaderProgram().IsValid() && GetShaderProgram().IsLinked() && IsFullyLoaded()) {

      //textures
      mNormalTexture.LoadToGPU();
      mAOTexture.LoadToGPU();

      if (GLUtils::CHelper::CheckForError()) throw std::runtime_error("NormalAOLightEffect Activation: textures could not be loaded to GPU");

      //Uniforms
      GetShaderProgram().Use();
      GetShaderProgram().SetUniform("minMaxBound", mBoundings);                           //boundings of the terrain
      GetShaderProgram().SetUniform("normalMap",     0);                                  //sampler for normal map
      GetShaderProgram().SetUniform("aoMap",         1);                                  //sampler for ambient occlusion map
    }
  }



  void CNormalAOLightEffect::Deactivate(void)
  {
    mNormalTexture.UnloadFromGPU();
    mAOTexture.UnloadFromGPU();
  }


  void CNormalAOLightEffect::BindTextures(void)
  {
    //textures
    glActiveTexture(GL_TEXTURE0);
    mNormalTexture.BindTexture();

    glActiveTexture(GL_TEXTURE1);
    mAOTexture.BindTexture();
  }



  void CNormalAOLightEffect::UnbindTextures(void)
  {
    //textures
    glActiveTexture(GL_TEXTURE0);
    mNormalTexture.UnbindTexture();

    glActiveTexture(GL_TEXTURE1);
    mAOTexture.UnbindTexture();
  }


} //end of namespace