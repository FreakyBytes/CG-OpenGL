#include "GLUtilsPrecompiled.h"
#include "GLDisplayList.h"

#include "Helper.h"


namespace GLUtils {

  CGLDisplayList::CGLDisplayList(void)
  {
    if (glGenLists == nullptr) {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        std::cout << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }

    mID = glGenLists(1);
  }



  CGLDisplayList::~CGLDisplayList(void)
  {
    glDeleteLists(mID, 1);
  }



  bool CGLDisplayList::IsValid(void)
  {
    return mID != 0;
  }



  void CGLDisplayList::BeginDefine(void)
  {
    glNewList(mID, GL_COMPILE);
  }



  void CGLDisplayList::EndDefine(void)
  {
    glEndList();
  }




  void CGLDisplayList::Render(void)
  {
    glCallList(mID);
    GLUtils::CHelper::CheckForError();
  }

}