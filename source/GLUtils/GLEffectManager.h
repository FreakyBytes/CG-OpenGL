#pragma once

#include "GLUtilsDefines.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#ifdef USETHREADING
#include <thread>
#include <mutex>
#endif

namespace GLUtils {

  class CGLEffect;
  class CGLTextureInterface;

  /**
   * Collectection of GLEffect instances
   */
  class CGLEffectManager {
  public:
    typedef std::map<std::string, std::shared_ptr<CGLEffect>> ContainerType;
    typedef ContainerType::iterator				Iterator;
    typedef ContainerType::const_iterator		ConstIterator;

    GLUTILS_API CGLEffectManager(void);
    GLUTILS_API ~CGLEffectManager(void);

    //Gets effect instance by name
    GLUTILS_API CGLEffect* FindEffect(std::string const & name) const;
    //Makes specified effect current
    GLUTILS_API void MakeEffectCurrent(std::string const & name);
    //register effect to the manager (manager gets owner)
    GLUTILS_API void RegisterEffect(std::string const & name, CGLEffect * effect);
  
    //Deletes all registered effects and clear the effect list
    GLUTILS_API void Clear(void);
    //Compiles all registered effects
    GLUTILS_API void CompileEffects(void);
    //Destroy all registered effects
    GLUTILS_API void DestroyEffects(void);
    //Returns current effect or nullptr, if no effect was registered 
    GLUTILS_API CGLEffect * GetCurrentEffect(void);
    //Returns name of the current effect
    GLUTILS_API std::string const & GetCurrentEffectName(void);
    //Increments Current Effect iterator
    GLUTILS_API void NextEffect(void);
    //Handle framebuffer resize event by notify all registered effects
    GLUTILS_API void OnFrameBufferResize(size_t width, size_t height);

    //activates and deactivates the usage of shader - does not allocate / deallocate resources for the current used shader
    GLUTILS_API void ActivateEffects(void);
    GLUTILS_API void DeactivateEffects(void);

    //loads the resources for all effects
    GLUTILS_API void LoadResources(void);
    GLUTILS_API void LoadResourcesInExtraThread(void);
    GLUTILS_API void AddResourceToLoadingQueue(CGLTextureInterface & texture, CGLEffect & effect);

    bool IsTextureLoadedLazy(void) const {return mLoadTexturesLazy;}
  private:
    CGLEffectManager(const CGLEffectManager&);
    CGLEffectManager& operator=(const CGLEffectManager&);

    ContainerType mContainer;

    Iterator mCurrentEffect;

    //stores last program used for activation and deactivation of shader effects
    GLint mLastProgramUsed;

    //indicates, that textures are not loaded before the program starts, but in parallel during the start
    bool mLoadTexturesLazy;

    //this container of textures will be loaded on the ResourceLoad method
    std::map<CGLEffect *, std::vector<CGLTextureInterface *>> mResourcesToLoad;

#ifdef USETHREADING
    std::mutex mResourceToLoadMutex;

    //thread, that loads the texture
    std::vector<std::shared_ptr<std::thread>> mResourceLoaderArray;
#endif
  };

} //namespace Utils