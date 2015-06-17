#include "GLUtilsPrecompiled.h"
#include "GLRenderTarget.h"

#include "Helper.h"


namespace GLUtils {


CGLRenderTarget::CGLRenderTarget(void)
  : mUseMultisampling(false)
  , mNumberOfRenderTargets(0)
{
}



CGLRenderTarget::~CGLRenderTarget(void)
{
}



void CGLRenderTarget::Initialize(GLuint width, GLuint height, const GLenum internalFormats[], unsigned int count, bool useDepthBuffer, bool useMultisampling, bool interpolateTexture)
{
  try {
    mUseMultisampling = useMultisampling;
    mNumberOfRenderTargets = count;

    mFrameBuffer.Initialize(width, height, useDepthBuffer, mUseMultisampling);
    mFrameBuffer.AttachTextures(internalFormats, count, interpolateTexture);
    CHelper::CheckForError();

    if (mUseMultisampling) { 
      mResolveBuffer.Initialize(width, height, false, false);
      mResolveBuffer.AttachTexture(GL_RGBA8, false);
    }
  }
  catch (std::exception& ex)
  {
    std::cout << "Failed to initialize render target: " << ex.what() << std::endl;
  }
}



void CGLRenderTarget::ActivateRenderTarget(void)
{
  std::vector<GLenum> renderTargets;
  for (unsigned int i = 0; i < mNumberOfRenderTargets; ++i) {
    renderTargets.push_back(GL_COLOR_ATTACHMENT0 + i);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer.GetHandle());
  glDrawBuffers(mNumberOfRenderTargets, renderTargets.data());
  mFrameBuffer.CheckFrameBuffer();
  if (CHelper::CheckForError()) throw CFailure("failed to activate render target!");

  (mUseMultisampling) ? glEnable( GL_MULTISAMPLE ) : glDisable(GL_MULTISAMPLE);
}



void CGLRenderTarget::DeactivateRenderTarget(void)
{
  glDisable(GL_MULTISAMPLE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



std::vector<GLuint> const & CGLRenderTarget::ActivateTextures(void)
{
  std::vector<GLuint> const & textureIDs = mFrameBuffer.GetColorBuffers();
  
  for (size_t i = 0; i < textureIDs.size(); ++i) {
    glActiveTexture(static_cast<GLenum>(static_cast<size_t>(GL_TEXTURE0) + i));
    glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
  }

  if (CHelper::CheckForError()) throw CFailure("failed to activate textures!");

  return textureIDs;
}



std::vector<GLuint> const & CGLRenderTarget::GetColorBuffers(void) const
{
  return mFrameBuffer.GetColorBuffers();
}



void CGLRenderTarget::Resolve(GLuint source)
{
  if (mUseMultisampling) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFrameBuffer.GetHandle());
    glReadBuffer(GL_COLOR_ATTACHMENT0 + source);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveBuffer.GetHandle());
    glBlitFramebuffer(0, 0, mFrameBuffer.GetWidth(), mFrameBuffer.GetHeight(), 0, 0, mFrameBuffer.GetWidth(), mFrameBuffer.GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
  
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  if (CHelper::CheckForError()) throw CFailure("failed to resolve frame buffers!");
}



void CGLRenderTarget::DrawFullScreen(GLuint source)
{
  Resolve(source);
  CHelper::RenderTexturedFullScreenQuad((mUseMultisampling) ? mResolveBuffer.GetColorBuffer(0) : mFrameBuffer.GetColorBuffer(source));
}



void CGLRenderTarget::DrawFullScreen()
{
  CHelper::RenderTexturedFullScreenQuad(mFrameBuffer.GetColorBuffers());
}

} //namespace Utils