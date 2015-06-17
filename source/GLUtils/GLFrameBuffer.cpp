#include "GLUtilsPrecompiled.h"
#include "GLFrameBuffer.h"

#include "Helper.h"

namespace GLUtils {


CGLFrameBuffer::CGLFrameBuffer()
  :mInitialized(false)
  ,mHandle(0)
  ,mWidth(0)
  ,mHeight(0)
  ,mDepthBuffer(0) 
  ,mMultisampling(false)
  ,mAttachedTarget(NothingAttached)
{
}



CGLFrameBuffer::~CGLFrameBuffer() {
  Clear();
}



void CGLFrameBuffer::Initialize(GLuint w, GLuint h, bool allocDepths, bool useMulti) {

  if (w == 0 || h == 0) throw std::logic_error("invalid arguments");

  mMultisampling = useMulti;

  if (mInitialized) {
    Clear();
  }

  if (glGenFramebuffers == nullptr) {
    GLenum glew_err = glewInit();
    if (glew_err != GLEW_NO_ERROR) {
      //std::cerr << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
      throw CFailure("failed to initalize glew!");
    }
  }

  glGenFramebuffers(1, &mHandle);
  if (mHandle == 0) throw CFailure("failed to allocated framebuffer object!");

  mWidth = w;
  mHeight = h;

  //allocate depth buffer, if specified
  if (allocDepths) {

    glGenRenderbuffers(1, &mDepthBuffer);
    if (mDepthBuffer == 0) throw CFailure("failed to create depth buffer render target!");

    glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
    if (CHelper::CheckForError()) throw CFailure("failed to bind render buffer!");

    if (mMultisampling) {
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, mWidth, mHeight);
    }
    else
    {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
    }

    if (CHelper::CheckForError()) throw CFailure("failed to create render buffer storage!");

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
    //CheckFrameBuffer(); --> cause problems on ATI graphic cards
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  CHelper::CheckForError();
  mInitialized = true;
}



void CGLFrameBuffer::Clear() {
  if (mInitialized) {

    if (mDepthBuffer != 0) { //depth buffer was initialized
      glDeleteRenderbuffers(1, &mDepthBuffer);
      mDepthBuffer = 0;
    }
   
    DeleteRenderTargets();
   
    glDeleteFramebuffers(1, &mHandle);
    mHandle = 0;
    mInitialized = false;
    mWidth = mHeight = 0;
    CHelper::CheckForError();
  }
}  



void CGLFrameBuffer::AttachRenderBuffers(const GLenum internalFormats[], glm::uint count) {
  
  if (!mInitialized) throw std::logic_error("Framebuffer was not initialized!");

  DeleteRenderTargets();
  mAttachedRenderBuffers.resize(count);
  glGenRenderbuffers(count, mAttachedRenderBuffers.data());
  if (mAttachedRenderBuffers[0] == 0) throw CFailure("failed to create render buffers!");

  glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
  if (CHelper::CheckForError()) throw CFailure("failed to bind frame buffer!");

  for(glm::uint i = 0; i < count; ++i) {
      glBindRenderbuffer(GL_RENDERBUFFER, mAttachedRenderBuffers[i]);
      if (CHelper::CheckForError()) throw CFailure("failed to bind render buffer!");

      if (mMultisampling) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, internalFormats[i], mWidth, mHeight);
      }
      else {
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormats[i], mWidth, mHeight);
      }
      if (CHelper::CheckForError()) throw CFailure("failed to create render buffer storage!");

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
      CheckFrameBuffer();
  }

  mAttachedTarget = RenderBufferAttached;
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void CGLFrameBuffer::AttachTextures(const GLenum internalFormats[], glm::uint count, bool interpolateTexture) {
  
  if (!mInitialized) throw std::logic_error("Framebuffer is not initialized!");

  DeleteRenderTargets();
  mAttachedTextures.resize(count);
  glGenTextures(count, mAttachedTextures.data());
  if (mAttachedTextures[0] == 0) throw CFailure("failed to create textures!");

  glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
  if (CHelper::CheckForError()) throw CFailure("failed to bind frame buffer!");

  for(glm::uint i = 0; i < count; ++i) {
    if (mMultisampling) {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mAttachedTextures[i]);
      glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      CHelper::CheckForError();
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, internalFormats[i], mWidth, mHeight, false);
      CHelper::CheckForError();
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D_MULTISAMPLE, mAttachedTextures[i], 0);
    }
    else {
      glBindTexture(GL_TEXTURE_2D, mAttachedTextures[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolateTexture ? GL_LINEAR : GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolateTexture ? GL_LINEAR : GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, internalFormats[i], mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glBindTexture(GL_TEXTURE_2D, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, mAttachedTextures[i], 0);
    }
    CheckFrameBuffer();
  }

  mAttachedTarget = TextureAttached;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void CGLFrameBuffer::CheckFrameBuffer() 
{
  GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  switch(error) {
    case GL_FRAMEBUFFER_COMPLETE:
      return;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      throw CFailure("Incomplete attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        throw CFailure("Missing attachment");
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        throw CFailure("Incomplete dimensions");
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        throw CFailure("Incomplete formats");
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        throw CFailure("Incomplete draw buffer");
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        throw CFailure("Incomplete read buffer");
    case GL_FRAMEBUFFER_UNSUPPORTED:
        throw CFailure("Framebuffer objects unsupported");
    default:
        throw CFailure("Unknown error");
    } 
}



void CGLFrameBuffer::DeleteRenderTargets() 
{
  mAttachedTarget = NothingAttached;
  if (!mAttachedTextures.empty()) {
    glDeleteTextures(static_cast<GLsizei>(mAttachedTextures.size()), mAttachedTextures.data());
    mAttachedTextures.clear();
    CHelper::CheckForError();
  }
  if (!mAttachedRenderBuffers.empty()) {
    glDeleteRenderbuffers(static_cast<GLsizei>(mAttachedRenderBuffers.size()), mAttachedRenderBuffers.data());
    mAttachedRenderBuffers.clear();
    CHelper::CheckForError();
  }
}


} //namespace Utils