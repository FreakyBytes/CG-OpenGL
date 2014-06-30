#pragma once

#include "GLUtilsDefines.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
#ifdef USETHREADING
#include <mutex>
#endif
#include "GLShaderCollection.h"
#include "GLShaderProgram.h"

namespace GLUtils {
  class CGLEffectManager;
  class CGLTextureInterface;

  /**
   * Abstract interface for effects. An effect is a set of
   * different render passes to achieve a specific visual 
   * representation or effect.
   * example:
   *		CGLEffect* effect = new MyGLEffect();
   *		...
   *		size_t numPasses = effect.GetPassCount();
   *		for (size_t i=0; i < numPasses; ++i) {
   *			effect->ApplyPass(i);
   *			//render objects
   *		}
   *		effect->Finalize();
   */
  class CGLEffect 
  {
  public:
    GLUTILS_API CGLEffect(CGLEffectManager * effectManager = nullptr);
    GLUTILS_API virtual ~CGLEffect(void);

    //activate effect - internally textures might be loaded, render targets initialized and uniforms be set
    GLUTILS_API virtual void Activate(void) {}

    //deactivates effect - internally resources might be released
    GLUTILS_API virtual void Deactivate(void) {}

    //Get the number of passes
    GLUTILS_API virtual size_t GetPassCount(void) const = 0;
    //Apply a specific render pass by setting up the render pipeline states
    GLUTILS_API virtual void ApplyRenderPass(size_t passIndex) = 0; 
    //Finalize the effect (e.g. applying post processing operations)
    GLUTILS_API virtual void Finalize() = 0; 

    template <typename FUNC> void Apply(FUNC renderFunc) {
      size_t numPasses = GetPassCount();
      for (size_t i=0; i < numPasses; ++i) {
        ApplyRenderPass(i);
        renderFunc();
      }
      Finalize();
    }

    //Compiles the effect or recompile if already compiled
    GLUTILS_API virtual bool Compile() = 0;
    //Destroy effect and deletes opengl resources
    GLUTILS_API virtual void Destroy();
    //Handle framebuffer resize events
    GLUTILS_API virtual void OnFrameBufferResize(size_t width, size_t height)  {};

    GLUTILS_API bool IsFullyLoaded(void);
    GLUTILS_API void RegisterTextures(CGLTextureInterface * arg1, CGLTextureInterface * arg2 = nullptr, CGLTextureInterface * arg3 = nullptr, CGLTextureInterface * arg4 = nullptr, CGLTextureInterface * arg5 = nullptr, CGLTextureInterface * arg6 = nullptr);

    GLUTILS_API std::string const & GetErrorLog(void) const { return mErrorLog; }
  protected:
    CGLShaderCollection & GetShaderCollection(void) {return mShaderCollection;}
    std::vector<std::shared_ptr<CGLShaderProgram>> & GetShaderPrograms(void) {return mShaderPrograms;}
    CGLEffectManager & GetEffectManager(void) {return *mEffectManager;}

    //the effect manager is only needed, if resources should be loaded lazy, else it can ne null
    CGLEffectManager * mEffectManager;

    //indicates that all textures where loaded
#ifdef USETHREADING
    std::mutex mResourcesLoadedMutex;
#endif
    bool mResourcesLoaded;

    std::string mErrorLog;

  private:
    //copy and assign operators are forbidden
    CGLEffect(const CGLEffect&);
    CGLEffect& operator=(const CGLEffect&);

    CGLShaderCollection mShaderCollection;
    std::vector<std::shared_ptr<CGLShaderProgram>> mShaderPrograms;

    std::vector<CGLTextureInterface *> mRegisteredTextures;
  };



  class CGLSimpleEffect : public CGLEffect
  {
  public:
    GLUTILS_API CGLSimpleEffect(CGLEffectManager * effectManager = nullptr, std::string const & shader = "" );
    GLUTILS_API virtual ~CGLSimpleEffect(void);

    //simple effect has only one pass (wraps up a simple shader)
    GLUTILS_API virtual size_t GetPassCount(void) const override {return 1;}

    GLUTILS_API virtual void ApplyRenderPass(size_t passIndex) override; 

    //Finalize the effect (e.g. applying post processing operations)
    GLUTILS_API virtual void Finalize(void) override; 

      //Compiles the effect or recompile if already compiled
    GLUTILS_API virtual bool Compile(void) override;

    //Handle framebuffer resize events
    GLUTILS_API virtual void OnFrameBufferResize(size_t width, size_t height) override;

    //returns the one shader used by simple effect - shader programs must have one entry at this point
    GLUTILS_API virtual CGLShaderProgram & GetShaderProgram(void) {return *GetShaderPrograms()[0];}

    //binds the textures to the appropriate texture slot
    GLUTILS_API virtual void BindTextures(void) = 0;
    GLUTILS_API virtual void UnbindTextures(void) = 0;

  protected:
    std::string mShaderFile;
  private:
    //copy and assign operators are forbidden
    CGLSimpleEffect(const CGLSimpleEffect&);
    CGLSimpleEffect& operator=(const CGLSimpleEffect&);
  };

} //namespace Utils