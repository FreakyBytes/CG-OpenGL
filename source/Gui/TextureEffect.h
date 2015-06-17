#include "Effects.h"

namespace Gui {

  class CTexturedTerrainEffect : public CTerrainEffect
  {
  public:
    CTexturedTerrainEffect(GLUtils::CGLEffectManager & effectManager);
    virtual ~CTexturedTerrainEffect(void);

    //setting boundaries of terrain.
    void Initialize(glm::vec2 const & minBounding, glm::vec2 const & maxBounding);

    virtual void Activate(void) override;
    virtual void Deactivate(void) override;

    virtual void BindTextures(void) override;
    virtual void UnbindTextures(void) override;
  protected:
    virtual void ReloadOtherTerrainBasedTextures(std::string const & mediaBasePath) override;
  private:
    GLUtils::CGLTexture mTerrainTexture;
  };
} //end of namespace