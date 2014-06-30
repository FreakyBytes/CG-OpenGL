#pragma once

#include "GLUtilsDefines.h"
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>

#include "GLFrameBuffer.h"

namespace GLUtils {


class CGLRenderTarget {
public:
  class CFailure : public std::runtime_error {
  public:
    GLUTILS_API CFailure(std::string const & msg):std::runtime_error(msg) {}
  };

  GLUTILS_API CGLRenderTarget(void);
  GLUTILS_API ~CGLRenderTarget(void);

  template <size_t N> void Initialize(GLuint width, GLuint height, const GLenum (&internalFormats)[N], bool useDepthBuffer = true, bool useMultiSampling = false, bool interpolateTexture = false) {Initialize(width, height, internalFormats, N, useDepthBuffer, useMultiSampling, interpolateTexture);}

  GLUTILS_API void ActivateRenderTarget(void);
  GLUTILS_API void DeactivateRenderTarget(void);
  GLUTILS_API void Resolve(GLuint source);
  GLUTILS_API void DrawFullScreen(GLuint source);
  GLUTILS_API void DrawFullScreen(void);
  GLUTILS_API std::vector<GLuint> const & ActivateTextures(void);

  GLUTILS_API std::vector<GLuint> const & GetColorBuffers(void) const;

  GLUTILS_API GLuint GetWidth(void)  const {return mFrameBuffer.GetWidth ();}
  GLUTILS_API GLuint GetHeight(void) const {return mFrameBuffer.GetHeight();}
private:

  GLUTILS_API void Initialize(GLuint width, GLuint height, const GLenum internalFormats[], unsigned int count, bool useDepthBuffer = true, bool useMultiSampling = false, bool interpolateTexture = false);

  bool                    mUseMultisampling;
  unsigned int            mNumberOfRenderTargets;
  CGLFrameBuffer          mFrameBuffer;
  CGLFrameBuffer          mResolveBuffer;
};

} //namespace Utils