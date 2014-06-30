#pragma once

#include "Effects.h"

namespace Gui {

  class CNormalAOLightEffect : public CTerrainEffect
  {
  public:
    CNormalAOLightEffect(GLUtils::CGLEffectManager & effectManager);
    CNormalAOLightEffect(GLUtils::CGLEffectManager & effectManager, std::string const & shader);
    virtual ~CNormalAOLightEffect(void);

    //setting boundaries of terrain.
    void Initialize(glm::vec2 const & minBounding, glm::vec2 const & maxBounding);

    virtual void Activate(void) override;
    virtual void Deactivate(void) override;

    virtual void BindTextures(void) override;
    virtual void UnbindTextures(void) override;
  };



  class CNormalAOLightColoredEffect : public CNormalAOLightEffect
  {
  public:
    CNormalAOLightColoredEffect(GLUtils::CGLEffectManager & effectManager);
    ~CNormalAOLightColoredEffect(void);
  };

} //end of namespace