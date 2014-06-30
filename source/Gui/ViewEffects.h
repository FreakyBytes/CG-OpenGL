#pragma once

#include "GuiDefines.h"

#include "GLEffectManager.h"

namespace Terrain {
  class CTerrainModel;
}


namespace Gui {

  enum EffectName
  {
    NormalAOLightEffect = 0,
    CrossHatchEffect,
    TexturedTerrainEffect,
    NumberOfEffects
  };

  class CView;


  class CViewEffects
  {
    public:
      CViewEffects(CView & parent);
      ~CViewEffects(void) {}
#ifdef USESHADER
      void SetEffect(EffectName name);
      void SetupEffects(void); 
      void ToggleShading(void);
      void CheckCurrentEffect(void);

      static std::string const & GetEffectAliases(size_t index);

      bool GetUseShader(void) const {return mUseShader;}
      void SetUseShader(bool value) {mUseShader = value; mUseShaderChanged = true;}

      EffectName GetCurrentEffect(void) const {return mCurrentEffect;}
      void SetCurrentEffect(EffectName value) {mCurrentEffect = value;}

      void NextEffect(void);
      void RecompileEffect(void);
      void ActivateEffects(void) {mEffectManager.ActivateEffects();}      //can only activate effect, if this effect was deactivated by the following method
      void DeactivateEffects(void) {mEffectManager.DeactivateEffects();}  //deactivates the current effect
      void ApplyToCurrentEffect(void (*renderFunctionpointer)(void));

      void NewTerrainLoaded(Terrain::CTerrainModel & model);

      //Handle framebuffer resize event by notify all registered effects
      void OnFrameBufferResize(size_t width, size_t height) {mEffectManager.OnFrameBufferResize(width, height);}
      void Clear(void) {mEffectManager.Clear();}

      GLUtils::CGLEffectManager & GetEffectManager(void) {return mEffectManager;}

    private:
      void ResetTerrainBoundings(glm::vec3 const & minBound, glm::vec3 const & maxBounds);
      void ResetTerrainTextures(std::string const & path);

      bool mUseShader;              //uses shader or not
      bool mUseShaderChanged;
      bool mPreviousUseShader;      //last state of the previous mUseShader - state

      EffectName mCurrentEffect;
      EffectName mLastEffect;
      glm::vec3 mTerrainMinBoundings;
      glm::vec3 mTerrainMaxBoundings;
      
      GLUtils::CGLEffectManager mEffectManager;  //manages all effects
      bool mEffectStillLoading;
#endif
      CView & mParentView;
  };

}
