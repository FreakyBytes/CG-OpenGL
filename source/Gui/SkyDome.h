#pragma once

#include "GuiDefines.h"
#include <string>
#include <memory>
#include "GLDisplayList.h"
#include "GLTexture.h"

namespace Gui {
  class CSkyDome
  {
  public:
    CSkyDome(std::string const & texturePath) : mTexturePath(texturePath) {}
    ~CSkyDome(void) {}

    void Render();
  private:
    glm::vec3 EvalParametricSphere(float r, float u, float v);

    std::string mTexturePath;
    std::shared_ptr<GLUtils::CGLDisplayList> mDisplayList;
    std::shared_ptr<GLUtils::CGLTexture> mTexture;
  };
}