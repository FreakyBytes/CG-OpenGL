#pragma once

#include "GLUtilsDefines.h"
#include <glm/glm.hpp>
#include <GL/glew.h>

#include <vector>

namespace GLUtils {


  class CGLFrameBuffer {
  public:
    //exception class for framebuffer errors
    class CFailure : public std::runtime_error {
    public:
      GLUTILS_API CFailure(std::string const & msg):std::runtime_error(msg) {}
    };

    GLUTILS_API CGLFrameBuffer();
    GLUTILS_API ~CGLFrameBuffer();

    GLUTILS_API GLuint GetHandle() const {return mHandle;}
    GLUTILS_API GLuint GetWidth() const {return mWidth;}
    GLUTILS_API GLuint GetHeight() const {return mHeight;}
    GLUTILS_API GLuint GetColorBuffer(GLuint i) const {return (mAttachedTarget == TextureAttached) ? mAttachedTextures[i] : (mAttachedTarget == RenderBufferAttached) ? mAttachedRenderBuffers[i] : 0;}
    GLUTILS_API std::vector<GLuint> const & GetColorBuffers(void) const {return (mAttachedTarget == TextureAttached) ? mAttachedTextures : mAttachedRenderBuffers;}

    GLUTILS_API void AttachRenderBuffer(const GLenum internalFormat) {AttachRenderBuffers(&internalFormat, 1);}

    GLUTILS_API void AttachTexture(const GLenum internalFormat, bool interpolateTexture) {AttachTextures(&internalFormat, 1, interpolateTexture);}
  
    template <size_t N> void AttachRenderBuffers(const GLenum (&internalFormats)[N]) {AttachRenderBuffers(internalFormats, N);}
    template <size_t N> void AttachTextures(const GLenum (&internalFormats)[N], bool interpolateTexture = false) {AttachTextures(internalFormats, N, interpolateTexture);}

    GLUTILS_API void AttachRenderBuffers(const GLenum internalFormats[], glm::uint count);
    GLUTILS_API void AttachTextures(const GLenum internalFormats[], glm::uint count, bool interpolateTexture);
    GLUTILS_API void Initialize(GLuint w, GLuint h, bool allocDepths, bool useMulti = false);
    GLUTILS_API void Clear();

    void CheckFrameBuffer();

  private:
    CGLFrameBuffer(const CGLFrameBuffer&);
    CGLFrameBuffer& operator=(const CGLFrameBuffer&);

    enum AttachedTarget
    {
      TextureAttached,
      RenderBufferAttached,
      NothingAttached
    };

    void DeleteRenderTargets();
  
    bool                mInitialized;
    AttachedTarget      mAttachedTarget;
    GLuint              mHandle;
    GLuint              mWidth;
    GLuint              mHeight;
    GLuint              mDepthBuffer;
    std::vector<GLuint>   mAttachedTextures;
    std::vector<GLuint>   mAttachedRenderBuffers;

    bool                mMultisampling;
  };

} //namespace Utils