#pragma once

#include "Effects.h"

namespace Gui {

  
  class CCrossHatchEffect : public CTerrainEffect
  {
  public:
    CCrossHatchEffect(GLUtils::CGLEffectManager & effectManager);
    virtual ~CCrossHatchEffect(void);

    //setting boundaries of terrain.
    virtual void Initialize(glm::vec2 const & minBounding, glm::vec2 const & maxBounding);

    virtual void Activate(void) override;
    virtual void Deactivate(void) override;

    virtual void BindTextures(void) override;
    virtual void UnbindTextures(void) override;
  protected:
    GLUtils::CGLMultiTexture mHatchTexture;     //texture for hatches
  };


} //end of namespace