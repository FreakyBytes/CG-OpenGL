#include "GLUtilsPrecompiled.h"
#include "GLEffectManager.h"

#include "GLEffect.h"
#include "Helper.h"

static const std::string sEmptyString("");



namespace GLUtils {

  CGLEffectManager::CGLEffectManager() 
    : mCurrentEffect(mContainer.end())
    , mLastProgramUsed(0)
    , mLoadTexturesLazy(false)
  {
  }



  CGLEffectManager::~CGLEffectManager() 
  {
    Clear();
  }



  CGLEffect * CGLEffectManager::FindEffect(const std::string& name) const {
    auto itr = mContainer.find(name);
    if (itr != mContainer.end())
      return (*itr).second.get();
    else
      return nullptr;
  }



  void CGLEffectManager::RegisterEffect(const std::string& name, CGLEffect * effect) {

    if (name.empty() || effect == nullptr)
      throw std::logic_error("invalid arguments!");

    if (mContainer.find(name) != mContainer.end()) {
      throw std::logic_error("duplicated effect name detected!");
    }

    mContainer[name] = std::shared_ptr<CGLEffect>(effect);
  }



  void CGLEffectManager::Clear() {
#ifdef USETHREADING
    std::for_each(begin(mResourceLoaderArray), end(mResourceLoaderArray), [&](std::shared_ptr<std::thread> & resourceLoader) {
      if (resourceLoader.get() != nullptr && resourceLoader->joinable()) {
        resourceLoader->join();
      }
    });
#endif
    mContainer.clear();
    mCurrentEffect = mContainer.end();
  }



  void CGLEffectManager::CompileEffects() {
    auto itr_end = mContainer.end();
    for (auto itr = mContainer.begin(); itr != itr_end; ++itr) {
      std::cout << "Compiling Effect: " << itr->first << " ...";
      if (itr->second->Compile() && !CHelper::CheckForError()) {
        std::cout << "Done!" << std::endl;
      }
      else {
        std::cout << "Failed!" << std::endl;
        std::cout << itr->second->GetErrorLog() << std::endl;
      }
    }

    //load texture resources afterwards
    if (mLoadTexturesLazy) {
      LoadResourcesInExtraThread();
    }
  }


  void CGLEffectManager::LoadResourcesInExtraThread()
  {
#ifdef USETHREADING
    //use this opportunity to delete finished threads (resource loader)
    bool finishedThreadFound = true;

    for (auto iter = mResourceLoaderArray.begin(); iter != mResourceLoaderArray.end(); ++iter) {
      if (!(*iter)->joinable()) { //thread has finished
        mResourceLoaderArray.erase(iter);
        iter = mResourceLoaderArray.begin();  //restart iterating through the array
      }
    }

    //create a new resource loader
    std::shared_ptr<std::thread> newThread(new std::thread(&GLUtils::CGLEffectManager::LoadResources, this));
    mResourceLoaderArray.push_back(newThread);
#endif
  }



  void CGLEffectManager::DestroyEffects() {
    auto itr_end = mContainer.end();
    for (auto itr = mContainer.begin(); itr != itr_end; ++itr) {
      (*itr).second->Destroy();
    }
  }


  void CGLEffectManager::OnFrameBufferResize(size_t width, size_t height) {
    auto itr_end = mContainer.end();
    for (auto itr = mContainer.begin(); itr != itr_end; ++itr) {
      (*itr).second->OnFrameBufferResize(width, height);
    }
  }



  CGLEffect * CGLEffectManager::GetCurrentEffect(void)
  {
    //return effect, that the iterator points to - if iterator is not initialized, do it now
    if (!mContainer.empty()) {
      if (mCurrentEffect == mContainer.end()) {
        mCurrentEffect = mContainer.begin();
      }

      return mCurrentEffect->second.get();
    }

    //return nullptr, if no effect is registered
    return nullptr;
  }



  std::string const & CGLEffectManager::GetCurrentEffectName(void)
  {
    if (mCurrentEffect != mContainer.end()) {
      return mCurrentEffect->first;
    }

    return sEmptyString;
  }



  void CGLEffectManager::NextEffect(void)
  {
    ++mCurrentEffect;
  }


  void CGLEffectManager::MakeEffectCurrent(std::string const & name)
  {
    mCurrentEffect = mContainer.find(name);
  }



  void CGLEffectManager::ActivateEffects(void)
  {
    glUseProgram(mLastProgramUsed);
  }



  void CGLEffectManager::DeactivateEffects(void)
  {
    glGetIntegerv(GL_CURRENT_PROGRAM, &mLastProgramUsed);
    glUseProgram(0);
  }



   void CGLEffectManager::LoadResources(void)
   {
#ifdef USETHREADING
     mResourceToLoadMutex.lock();
#endif
     auto resourcesToLoadCopy = mResourcesToLoad;
#ifdef USETHREADING
     mResourceToLoadMutex.unlock();
#endif
     std::for_each(begin(resourcesToLoadCopy), end(resourcesToLoadCopy), [&](std::pair<CGLEffect *,  std::vector<CGLTextureInterface *>> entry) {
       bool resourceLoaded = true;
       std::for_each(begin(entry.second), end(entry.second), [&] (CGLTextureInterface * texture) {
         if (!texture->IsLoadedToCPU()) {
           resourceLoaded &= texture->LoadFromFile(false);
         }
       });
     });
   }



    void CGLEffectManager::AddResourceToLoadingQueue(CGLTextureInterface & texture, CGLEffect & effect)
    {
#ifdef USETHREADING
      mResourceToLoadMutex.lock();
      CMutexReleaser releaser(mResourceToLoadMutex);
#endif
      std::vector<CGLTextureInterface *> textureArray;
      mResourcesToLoad.insert(std::make_pair(&effect, textureArray)); //will not insert an element, if there is already one
      mResourcesToLoad.find(&effect)->second.push_back(&texture);
    }


} //namespace Utils