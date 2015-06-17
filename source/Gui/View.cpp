#include "GuiPrecompiled.h"
#include "View.h"
#include "GlutCallbacks.h"

namespace Gui {

  CView::CView(void)
    : mWindowID(0)
    , mWidth(0)
    , mHeight(0)
    , mInitialized(false)
    , mStates(*this)
    , mEffects(*this)
    , mProjectionMatrixDirty(true)
  {
  }




  void CView::InitializeCallbacks(void)
  {
    // register callbacks
    //glutIgnoreKeyRepeat(1);
    glutDisplayFunc(CGlutCallbacks::DisplayCallback);
    glutReshapeFunc(CGlutCallbacks::ReshapeCallback);
    glutKeyboardFunc(CGlutCallbacks::KeyboardPressedCallback);
    glutKeyboardUpFunc(CGlutCallbacks::KeyboardReleasedCallback);
    glutSpecialFunc(CGlutCallbacks::KeyboardPressedCallback);
    glutSpecialUpFunc(CGlutCallbacks::KeyboardReleasedCallback);
    glutMouseFunc(CGlutCallbacks::MouseKeyCallback);
    glutMotionFunc(CGlutCallbacks::MouseMoveCallback);
    glutPassiveMotionFunc(CGlutCallbacks::MousePassiveMoveCallback);
    glutCloseFunc(CGlutCallbacks::FinalizeCallback);
  }
}