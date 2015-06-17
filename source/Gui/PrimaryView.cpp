#include "GuiPrecompiled.h"
#include "PrimaryView.h"

#include "KeyboardInterface.h"
#include "MouseInterface.h"

#include "Helper.h"
#include "GLUtilities.h"
#include "ErrorMetric.h"
#include "TerrainModel.h"
#include "RasterTerrainModel.h"
#include "GuiManager.h"
#include "ViewEffects.h"
#include "SkyDome.h"
#include "Model.h"
#include <gl\freeglut.h>


namespace Gui {

  class CEffectDeactivator
  {
  public:
    #ifdef USESHADER
      CEffectDeactivator(CPrimaryView & view) : mView(view) { mView.GetViewEffects().DeactivateEffects(); }
      ~CEffectDeactivator(void) { mView.GetViewEffects().ActivateEffects(); }
  private:
    CPrimaryView & mView;

    #endif
  };



  class CEffectActivator
  {
  public:
#ifdef USESHADER
    CEffectActivator(CPrimaryView & view) : mView(view) { mView.GetViewEffects().ActivateEffects(); }
    ~CEffectActivator(void) { mView.GetViewEffects().DeactivateEffects(); }
  private:
    CPrimaryView & mView;

#endif
  };



  CPrimaryView::CPrimaryView(Terrain::CTerrainModel & terrain)
    : CView()
    , mTerrain(terrain)
  {
    mWidth = 1680;
    mHeight = 1020;
    mKeyboardInterface.reset(new CKeyboardInterface(*this));
    mMouseInterface.reset(new CMouseInterface(*this));
    mCamera.reset(new GLUtils::CFirstPersonCamera());

    //create gui manager
    CGuiManager::CreateGUI(*this);
  }



  CPrimaryView::~CPrimaryView(void)
  {
    CGuiManager::Destroy();
  }



  bool CPrimaryView::Initialize(int argc, char **argv)
  {
    glutInit(&argc, argv);
    glutInitContextVersion(2, 0);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(mWidth, mHeight);
    mWindowID = glutCreateWindow("SysTAvio Demonstrator");


    if (mWindowID == 0) {
      //creation of the window failed
      assert(false);
      return false;
    }

    //initialize Glew
    GLenum glew_err = glewInit();
    if (glew_err != GLEW_NO_ERROR) {
      std::cout << "Failed to initialize opengl extension wrapper: " << (const char *)glewGetErrorString(glew_err) << std::endl;
      return false;
    }

    //initialize all callbacks (mouse and keyboard control)
    InitializeCallbacks();

    //setup shader
#ifdef USESHADER
    GetViewEffects().SetupEffects();
    GetViewEffects().NewTerrainLoaded(mTerrain);
    GetViewEffects().CheckCurrentEffect();
    GetViewEffects().DeactivateEffects();
#endif
    
    GetFirstPersonCamera().SetZAxisUp(true);
    GetFirstPersonCamera().SetViewParams(GetViewStates().GetInitialEyePosition(), GetViewStates().GetInitialLookAtVector());

    //creating sky dome
    mSkyDome.reset(new CSkyDome("../../../media/texture/skyDome.png"));

    //creating one instance of a palme
    mPalm.reset(new CModel());
    mPalm->Create("../../../media/model/palm.obj", "../../../media/model/palm_diffuse.png", "../../../media/model/palm_normal_ao.png");

    return GLUtils::CHelper::CheckForError();
  }



  void CPrimaryView::RenderTerrainWrapper(void)
  {
    CGuiManager::GetGuiManager().GetPrimaryView().GetTerrain().Render();
  }



  void CPrimaryView::RenderTerrain(glm::dvec3 const & eye, glm::dvec3 const & lookAt, glm::dvec3 const & upVec)
  {
    GLUtils::CGLPushMatrix scopedMatrix(GL_MODELVIEW);

    //const glm::dvec3 eye = GetFirstPersonCamera().GetEyePt();
    //const glm::dvec3 lookAt = GetFirstPersonCamera().GetLookAtPt();
    //const glm::dvec3 upVec = dvec3(0.0f, 0.0f, 1.0f);

    //update error metric
    GetViewStates().SetViewFrustumToCamera(eye, lookAt, upVec);
    Terrain::CErrorMetric emetric;
    emetric.SetViewPosition(eye);
    emetric.SetViewparams(static_cast<float>(glm::radians(GetViewStates().GetFieldOfView())), static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT)), GetViewStates().GetTolerance());

    GetTerrain().Update(emetric, GetViewStates().GetViewFrustum());

#ifdef USESHADER
    {
      CEffectActivator activateShader(*this);
      GetViewEffects().ApplyToCurrentEffect(RenderTerrainWrapper);
    }
#else
    RenderTerrainWrapper();
#endif

  }


  void CPrimaryView::RenderSkyDome(void)
  {
    mSkyDome->Render();
  }



  void CPrimaryView::RenderPalm(void)
  {
    mPalm->RenderWithTextures(false);
  }



  void CPrimaryView::Reshape(int width, int height)
  {
    mWidth = width;
    mHeight = height;

    UpdateProjection();
#ifdef USESHADER
    GetViewEffects().OnFrameBufferResize(width, height);
#endif
  }



  void CPrimaryView::UpdateProjectionMatrix()
  {
    glViewport(0, 0, mWidth, mHeight);

    double aratio = static_cast<double>(mWidth)/static_cast<double>(mHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(GetViewStates().GetFieldOfView(), aratio, GetViewStates().GetNearPlane(), GetViewStates().GetFarPlane());
    GetViewStates().InitializeFrustum(static_cast<float>(GetViewStates().GetFieldOfView()),static_cast<float>(aratio), static_cast<float>(GetViewStates().GetNearPlane()),static_cast<float>(GetViewStates().GetFarPlane()));
    GLUtils::CHelper::CheckForError();
    

    mProjectionMatrixDirty = false;
  }



  void CPrimaryView::Finalize(void)
  {
    GLUtils::CHelper::CheckForError();

#ifdef USESHADER
    GetViewEffects().Clear();
#endif
  }



  void CPrimaryView::ShowRenderedFrame(void) const
  {
    //show rendered content
    glutSwapBuffers();

    //force a redisplay for continues animation
    glutPostRedisplay();

    glutSetWindowTitle(GetViewStates().GetTitelString().c_str());    //show fps
  }



}