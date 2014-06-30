#pragma once

#include "PrimaryView.h"

namespace Application {

  class COpenGLWindow : public Gui::CPrimaryView
  {
  public:
    COpenGLWindow(Terrain::CTerrainModel & terrain);
    ~COpenGLWindow(void) {}

    virtual void Display(void) override;
    virtual bool Initialize(int argc = 0, char **argv = nullptr) override;
    virtual bool EvaluateActionKey(unsigned char key, bool ctrl, bool alt) override;
  private:

    virtual void UpdateScene(double time) override;
    virtual void RenderScene(void) override;
    virtual void RenderPalm(void) override;
    virtual void RenderTerrain(glm::dvec3 const & eye, glm::dvec3 const & lookAt, glm::dvec3 const & upVec) override;
  };
}