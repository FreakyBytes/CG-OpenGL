#pragma once

#include "GLUtilsDefines.h"
#include "GLTexture.h"
#ifdef USETHREADING
#include <mutex>
#endif

namespace GLUtils {


  class CHelper
  {
  public:
    //draws a full screen textured quad
    GLUTILS_API static void RenderTexturedFullScreenQuad(CGLTexture const & texture);
    GLUTILS_API static void RenderTexturedFullScreenQuad(GLuint texture);
    GLUTILS_API static void RenderTexturedFullScreenQuad(std::vector<GLuint> const & texture);
    
    //Error Handling - returns true, if an error occured
    GLUTILS_API static bool CheckForError(void);
  private:
    CHelper() {}  //static class - forbidden
    ~CHelper(){}  //static class - forbidden
  };
  


  class CMutexReleaser
  {
    public:
#ifdef USETHREADING
      CMutexReleaser(std::mutex & mutex) : mMutexToRelease(mutex) {}
      ~CMutexReleaser(void) {mMutexToRelease.unlock();}

    private:
      std::mutex & mMutexToRelease;
#endif
  };


  //exception class for shader collection errors
  class CFailure : public std::runtime_error {
  public:
    GLUTILS_API CFailure(std::string const & msg) :std::runtime_error(msg) {}
  };
} //namespace Utils