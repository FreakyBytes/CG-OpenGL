#include "GuiPrecompiled.h"
#include "ViewEffects.h"

#ifdef USESHADER

#include "GLEffect.h"
#include "Helper.h"
#include "Effects.h"
#include "ViewStates.h"
#include "PrimaryView.h"
#include "TerrainModel.h"
#include "HatchEffects.h"
#include "NormalAOEffects.h"
#include "TextureEffect.h"

namespace Gui {

    static std::string sEffectAlias[] = { "Normal AO Light Effect",
                                          "Cross Hatch Effect",
                                          "Terrain Textured Effect"};


  CViewEffects::CViewEffects(CView & parent) 
    : mParentView(parent)
    , mEffectManager()
    , mUseShader(true)
    , mUseShaderChanged(false)
    , mPreviousUseShader(true)
    , mTerrainMinBoundings(vec3(1.0f))
    , mTerrainMaxBoundings(vec3(1.0f))
    , mCurrentEffect(EffectName::TexturedTerrainEffect)
    , mLastEffect(EffectName::NumberOfEffects)
    , mEffectStillLoading(false)
  {
  }



  std::string const & CViewEffects::GetEffectAliases(size_t index)
  {
    return sEffectAlias[index];
  }



  void CViewEffects::CheckCurrentEffect(void)
  {
    //changes the variable lazy - SetEffect takes up to much time to be excecuted promptly
    if (mLastEffect != mCurrentEffect) {
      SetEffect(mCurrentEffect);
      mLastEffect = mCurrentEffect;
    }
  }



  void CViewEffects::NewTerrainLoaded(Terrain::CTerrainModel & model)
  {
    //deactivate shading
    GLUtils::CGLEffect * currentEffect = mEffectManager.GetCurrentEffect();
    currentEffect->Deactivate();

    vec3 minBound, maxBound;
    model.GetBoundings(minBound, maxBound);
    ResetTerrainBoundings(minBound, maxBound);

    ResetTerrainTextures(model.GetModelPath());

    currentEffect->Activate();
  }




  void CViewEffects::ResetTerrainBoundings(glm::vec3 const & minBound, glm::vec3 const & maxBounds)
  {
    mTerrainMinBoundings = minBound;
    mTerrainMaxBoundings = maxBounds;

    for (size_t i = 0; i < static_cast<size_t>(NumberOfEffects); ++i) {
      CTerrainEffect * terrainEffect = dynamic_cast<CTerrainEffect *>(mEffectManager.FindEffect(sEffectAlias[i]));

      if (terrainEffect != nullptr) {
        terrainEffect->SetTerrainBoundings(mTerrainMinBoundings, mTerrainMaxBoundings);
      }
    }
  }



  void CViewEffects::ResetTerrainTextures(std::string const & path)
  {
    for (size_t i = 0; i < static_cast<size_t>(NumberOfEffects); ++i) {
      CTerrainEffect * terrainEffect = dynamic_cast<CTerrainEffect *>(mEffectManager.FindEffect(sEffectAlias[i]));

      if (terrainEffect != nullptr) {
        terrainEffect->ReloadTerrainTextures(path);
      }
    }

    if (mEffectManager.IsTextureLoadedLazy()) {
      mEffectManager.LoadResourcesInExtraThread();
    }
  }



  void CViewEffects::SetEffect(EffectName name) 
  {
    //std::cout << "select effect " << name << std::endl;
    GLUtils::CGLEffect * currentEffect = mEffectManager.GetCurrentEffect();

    if (currentEffect != nullptr) {
      currentEffect->Deactivate();
      //std::cout << "deactivate old effect" << std::endl;
    }

    mEffectManager.MakeEffectCurrent(sEffectAlias[static_cast<size_t>(name)]);
    currentEffect = mEffectManager.GetCurrentEffect();

    GLUtils::CHelper::CheckForError();

    if (currentEffect != nullptr) {
      //std::cout << "activate new effect" << std::endl;
      currentEffect->Activate();
    }

  }


  //set shader
  void CViewEffects::SetupEffects() 
  {
    if (glewIsSupported == nullptr)  {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        std::cerr << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }

    CPrimaryView * primaryView = dynamic_cast<CPrimaryView *>(&mParentView);

    if (glewIsSupported("GL_VERSION_2_0") && primaryView != nullptr) {

      const vec2 minBound = vec2(mTerrainMinBoundings.x, mTerrainMinBoundings.y);
      const vec2 maxBound = vec2(mTerrainMaxBoundings.x, mTerrainMaxBoundings.y);

      //create shader - ownership is given to effect manager
      CTexturedTerrainEffect * texturedTerrain = new CTexturedTerrainEffect(mEffectManager);
      texturedTerrain->Initialize(minBound, maxBound);
      mEffectManager.RegisterEffect(sEffectAlias[TexturedTerrainEffect], texturedTerrain);

      mEffectManager.CompileEffects();

      SetEffect(static_cast<EffectName>(0));
    }
  }



  //loops between different shading techniques
  void CViewEffects::NextEffect(void) 
  {
    //loop within enum range
    GLUtils::CGLEffect * currentEffect = mEffectManager.GetCurrentEffect();
    if (currentEffect != nullptr) {
      currentEffect->Deactivate();
    }

    mEffectManager.NextEffect();
    currentEffect = mEffectManager.GetCurrentEffect();

    if (currentEffect != nullptr) {
      currentEffect->Activate();
    }
  
    std::cout << "Using Effect " << mEffectManager.GetCurrentEffectName() << std::endl;
  }



  //recompiles the shader
  void CViewEffects::RecompileEffect(void)
  {
    GLUtils::CGLEffect * currentEffect = mEffectManager.GetCurrentEffect();

    std::cout << "Compiling Effect: " << mEffectManager.GetCurrentEffectName() << "...";
    //SetupShader();
    if (currentEffect != nullptr) {
      currentEffect->Deactivate();
      GLUtils::CHelper::CheckForError();
      if (currentEffect->Compile()) {
        GLUtils::CHelper::CheckForError();
        std::cout << "Successful!" << std::endl;
      }
      currentEffect->Activate();
      GLUtils::CHelper::CheckForError();
    }
  }



  void CViewEffects::ToggleShading(void)
  {
    if (mUseShaderChanged) {
      mUseShaderChanged = false;
      mPreviousUseShader = mUseShader;

      GLUtils::CGLEffect * currentEffect = mEffectManager.GetCurrentEffect();

      if (currentEffect != nullptr) {

        if (mUseShader) {
          currentEffect->Activate();
        }
        else {
          currentEffect->Deactivate();
          mEffectManager.DeactivateEffects();
        }
      }
    }
  }



  void CViewEffects::ApplyToCurrentEffect(void (*renderFunctionpointer)(void))
  {
    GLUtils::CGLEffect * currentEffect = mEffectManager.GetCurrentEffect();
    if (currentEffect != nullptr && mUseShader && currentEffect->IsFullyLoaded()) {
      if (mEffectStillLoading) {
        mEffectManager.ActivateEffects();
        currentEffect->Activate();
        mEffectStillLoading = false;
      }
      currentEffect->Apply(renderFunctionpointer);
    }
    else {
      if (currentEffect != nullptr && !currentEffect->IsFullyLoaded()) {
        mEffectStillLoading = true;
        mEffectManager.DeactivateEffects();
      }
      renderFunctionpointer();
    }
  }

}

#else

namespace Gui {
  CViewEffects::CViewEffects(CView & parent) 
    : mParentView(parent)
  {
  }
}

#endif