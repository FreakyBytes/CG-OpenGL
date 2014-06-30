#pragma once

#include "GuiDefines.h"
#include "View.h"

#include "ViewFrustum.h"

//forwards
namespace GLUtils{
  class CFirstPersonCamera;
}


namespace Terrain {
  class CErrorMetric;
}

namespace Gui {
  class CSkyDome;
  class CModel;


  //this class abstracts the primary 3D view (something like the primary flight display) 
  class CPrimaryView : public CView
  {
  public:
    GUI_API CPrimaryView(Terrain::CTerrainModel & terrain);
    GUI_API virtual ~CPrimaryView(void);

    GUI_API virtual void Display(void) override {};
    GUI_API virtual void Reshape(int width, int height) override;
    GUI_API virtual void Finalize(void) override;
    GUI_API virtual bool Initialize(int argc = 0, char **argv = nullptr) override;

    GUI_API virtual bool EvaluateActionKey(unsigned char key, bool ctrl, bool alt) {/*implement in child class*/ return false; };

  protected:
    GUI_API Terrain::CTerrainModel & GetTerrain(void) { return mTerrain; }

    GUI_API virtual void UpdateScene(double time) {};
    GUI_API virtual void RenderScene(void) {};
    GUI_API virtual void RenderTerrain(glm::dvec3 const & eye, glm::dvec3 const & lookAt, glm::dvec3 const & upVec);
    GUI_API virtual void RenderSkyDome(void);
    GUI_API virtual void RenderPalm(void);
    GUI_API void UpdateProjectionMatrix(void);
    GUI_API GLUtils::CFirstPersonCamera & GetFirstPersonCamera(void) { return dynamic_cast<GLUtils::CFirstPersonCamera &>(*GetViewCamera()); };
    GUI_API GLUtils::CFirstPersonCamera const & GetFirstPersonCamera(void) const { return dynamic_cast<GLUtils::CFirstPersonCamera const &>(*GetViewCamera()); };
    GUI_API void ShowRenderedFrame(void) const;
    GUI_API static void RenderTerrainWrapper(void);

    Terrain::CTerrainModel & mTerrain;
    std::shared_ptr<CSkyDome> mSkyDome;
    std::shared_ptr<CModel> mPalm;
  };



}