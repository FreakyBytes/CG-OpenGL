#include "GLUtilsPrecompiled.h"
#include "GLEffect.h"

#include "Helper.h"
#include <algorithm>

#include "GLEffectManager.h"

namespace GLUtils {


  CGLEffect::CGLEffect(CGLEffectManager * effectManager) 
    : mEffectManager(effectManager)
    , mResourcesLoaded(false)
  {
  }



  CGLEffect::~CGLEffect() 
  {
    Destroy();
  }


  void CGLEffect::Destroy()
  {
    std::for_each(mShaderPrograms.begin(), mShaderPrograms.end(), [](std::shared_ptr<CGLShaderProgram> & program) {
      program->Destroy();
    });

    mShaderCollection.Clear();
  }



  
  bool CGLEffect::IsFullyLoaded(void) 
  {
    bool result = true;
    std::for_each(begin(mRegisteredTextures), end(mRegisteredTextures), [&](CGLTextureInterface * texture){
      result &= texture->IsLoadedToCPU();
    });

    return result;
  }


  void CGLEffect::RegisterTextures(CGLTextureInterface * arg1, CGLTextureInterface * arg2, CGLTextureInterface * arg3, CGLTextureInterface * arg4, CGLTextureInterface * arg5, CGLTextureInterface * arg6)
  {
    if (arg1 != nullptr) mRegisteredTextures.push_back(arg1);
    if (arg2 != nullptr) mRegisteredTextures.push_back(arg2);
    if (arg3 != nullptr) mRegisteredTextures.push_back(arg3);
    if (arg4 != nullptr) mRegisteredTextures.push_back(arg4);
    if (arg5 != nullptr) mRegisteredTextures.push_back(arg5);
    if (arg6 != nullptr) mRegisteredTextures.push_back(arg6);
  }


  //---------------------------------------------------------------------------


  CGLSimpleEffect::CGLSimpleEffect(CGLEffectManager * effectManager, std::string const & shader)
    : CGLEffect(effectManager)
    , mShaderFile(shader)
  {
    if (mShaderFile != "") {
      //create new instance of shader program
      std::shared_ptr<CGLShaderProgram> program(new CGLShaderProgram());
      GetShaderPrograms().push_back(program);
    }
  }



  CGLSimpleEffect::~CGLSimpleEffect(void)
  {
  }



  void CGLSimpleEffect::ApplyRenderPass(size_t passIndex) 
  {
    assert(passIndex == 0);  //simple effect has only one pass (shader effect)

    GetShaderProgram().Use();

    BindTextures();
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }



  void CGLSimpleEffect::Finalize(void)
  {
    UnbindTextures();
    glUseProgram(0);
  }


  //Compiles the effect or recompile if already compiled
  bool CGLSimpleEffect::Compile(void)
  {
    if (mShaderFile != "") {    //using no shader would lead to fixed function pipeline

      try {
        GetShaderCollection().Clear();
        GetShaderCollection().CreateFromSourceFile(mShaderFile);
        CHelper::CheckForError();

        //create new instance of shader program
        if (GetShaderPrograms().empty()) {
          assert(false);  //an instance of a shader program should have been created in the constructor and never been deleted 
          return false;
        } else {
          return GetShaderCollection().LinkShaders(*GetShaderPrograms()[0]);
        }
      }
      catch(std::exception& ex) {
        mErrorLog.append(ex.what());
        //std::cerr << ex.what() << std::endl;
        return false;
      }
    }

    return true;
  }



  //Handle framebuffer resize events
  void CGLSimpleEffect::OnFrameBufferResize(size_t width, size_t height)
  {
    //there will be no frame buffer for simple effects
  }

} //namespace Utils