#include "GLUtilsPrecompiled.h"
#include "GLTexture.h"

#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <iostream>
#include <vector>
#include <map>
#include "Helper.h"


namespace GLUtils {

  CGLTexture::CGLTexture(bool useMipMaps, bool cacheData)
    : mFile()
    , mTextureID(0)
    , mWidth(0)
    , mHeight(0)
    , mHasAlpha(false)
    , mCacheData(cacheData)
    , mUseMipMaps(useMipMaps)
  {
  }



  //used to load a single image as texture
  CGLTexture::CGLTexture(std::string const & fileName, bool useMipMaps, bool cacheData)
    : mTextureID(0)
    , mWidth(0)
    , mHeight(0)
    , mHasAlpha(false)
    , mCacheData(cacheData)
    , mUseMipMaps(useMipMaps)
  {
    mFile.push_back(std::string(fileName));
  }



  //used to load multiple images as mip map structure as texture
  CGLTexture::CGLTexture(std::vector<std::string> const & fileNames, bool useMipMaps, bool cacheData)
    : mTextureID(0)
    , mWidth(0)
    , mHeight(0)
    , mHasAlpha(false)
    , mCacheData(cacheData)
    , mUseMipMaps(useMipMaps)
    , mFile(fileNames)
  {
  }



  CGLTexture::~CGLTexture(void)
  {
    UnloadFromGPU();
  }



  void CGLTexture::LoadToGPU()
  {
#ifdef USETHREADING
    if (!mLoadToCPUMutex.try_lock()) return;
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif

    if (mData.empty()) throw std::logic_error("Data was not loaded to RAM beforehand!");

    if (mTextureID == 0) {
      glGenTextures(1, &mTextureID);
    }

    if (mTextureID == 0) {
      std::cout << "Texture could be initialized (no empty ID was found)" << std::endl;
      return;
    }

    if (glActiveTexture == nullptr)  {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        std::cout << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }

    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum internFormat, dataFormat, dataType;
    unsigned int numberOfComponents;
    GetGLTextureType(mFormat, internFormat, dataFormat, dataType, numberOfComponents);

    //load multiple images as mipmap
    //load main images and generate storage for mipmaps
    GLint mipmapLevel = 0;
    //std::cout << "Loading Textures " << mTextureID << " to GPU: " << mWidth << " x " << mHeight << " (" << mData[mWidth]->mData.size() << ")" <<  std::endl;
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, mData.size() == 1 ? mUseMipMaps : true);
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, internFormat, mWidth, mHeight, 0, dataFormat, dataType, mData[mWidth]->mData.data());
    CHelper::CheckForError();

    
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, false);

    //load mipmaps
    if (mFile.size() > 1) {
      int width = mWidth;
      int height = mHeight;
      while(width > 1 && height > 0) {
        width /= 2;
        height /= 2;
        ++mipmapLevel;
        if (mData.find(width) != mData.end()) {
          glTexImage2D(GL_TEXTURE_2D, mipmapLevel, internFormat, width, height, 0, dataFormat, dataType, mData[width]->mData.data());
          CHelper::CheckForError();
        }
      }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (mUseMipMaps) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }



  void CGLTexture::UnloadFromGPU()
  {
    if (mTextureID != 0) {
      glDeleteTextures(1, &mTextureID);
    }

    mTextureID = 0;
  }



  void CGLTexture::UnloadFromCPU(void)
  {
#ifdef USETHREADING
    mLoadToCPUMutex.lock();
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif
    mData.clear();
  }



  void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
      FILE* fp = (FILE*)png_get_io_ptr(png_ptr);
      if (!data)
          png_error( png_ptr, "Attempt to read from null file pointer");
      fread( data, length, 1, fp);
  }


  bool LoadPngImage(std::string const & fileName, int &outWidth, int &outHeight, bool &outHasAlpha, Format& outFormat, std::vector<unsigned char>& outData) 
  {
      png_structp png_ptr;
      png_infop info_ptr;
      unsigned int sig_read = 0;
      int color_type, interlace_type;
      FILE * fp = nullptr;
      const char * name = fileName.c_str();
      errno_t errorCode = fopen_s(&fp, name, "rb");
      if (fp == nullptr) {
          return false;
      }
 
      unsigned char signature[8];
      fread( signature, 8, 1, fp);

      if (!png_check_sig( signature, 8)) {
        std::printf("Image file is not a png.\n");
          return false;
      }

      /* Create and initialize the png_struct
       * with the desired error handler
       * functions.  If you want to use the
       * default stderr and longjump method,
       * you can supply NULL for the last
       * three parameters.  We also supply the
       * the compiler header file version, so
       * that we know if the application
       * was compiled with a compatible version
       * of the library.  REQUIRED
       */
      png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
 
      if (png_ptr == NULL) {
          fclose(fp);
          return false;
      }
 
      /* Allocate/initialize the memory
       * for image information.  REQUIRED. */
      info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL) {
          fclose(fp);
          png_destroy_read_struct(&png_ptr, NULL, NULL);
          return false;
      }
 
      /* Set error handling if you are
       * using the setjmp/longjmp method
       * (this is the normal method of
       * doing things with libpng).
       * REQUIRED unless you  set up
       * your own error handlers in
       * the png_create_read_struct()
       * earlier.
       */
      if (setjmp(png_jmpbuf(png_ptr))) {
          /* Free all of the memory associated
           * with the png_ptr and info_ptr */
          png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
          fclose(fp);
          /* If we get here, we had a
           * problem reading the file */
          return false;
      }
 
      /* Set up the output control if
       * you are using standard C streams */
      //png_init_io(png_ptr, fp);
      png_set_read_fn(png_ptr, fp, pngReadCallback);

      /* If we have already
       * read some of the signature */
      png_set_sig_bytes(png_ptr, 8);
 
      /*
       * If you have enough memory to read
       * in the entire image at once, and
       * you need to specify only
       * transforms that can be controlled
       * with one of the PNG_TRANSFORM_*
       * bits (this presently excludes
       * dithering, filling, setting
       * background, and doing gamma
       * adjustment), then you can read the
       * entire image (including pixels)
       * into the info structure with this
       * call
       *
       * PNG_TRANSFORM_STRIP_16 |
       * PNG_TRANSFORM_PACKING  forces 8 bit
       * PNG_TRANSFORM_EXPAND forces to
       *  expand a palette into RGB
       */
      png_read_info(png_ptr, info_ptr);
 
      png_uint_32 width, height;
      int bit_depth;
      png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
      outWidth = width;
      outHeight = height;
 
      bool use16bit = bit_depth == 16;
      if (use16bit) {
        png_set_swap(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
      }

      //detect format
      switch(color_type) {
      case PNG_COLOR_TYPE_GRAY:
        outFormat = use16bit ? Format::Gray16 : Format::Gray8;
        break;
      case PNG_COLOR_TYPE_GRAY_ALPHA:
        outFormat = use16bit ? Format::GrayAlpha16 : Format::GrayAlpha8;
        break;
      case PNG_COLOR_TYPE_RGB:
        outFormat = use16bit ? Format::RGB16 : Format::RGB8;
        break;
    case PNG_COLOR_TYPE_RGBA:
        outFormat = use16bit ? Format::RGBA16 : Format::RGBA8;
        break;
    default:
      throw std::logic_error("invalid pixel format");
      }

    
      //alloc mem
      size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
      outData.resize(row_bytes * outHeight);

      //construct rows
      std::vector<unsigned char*> rowPtrs(outHeight);
      for (int i=0; i < outHeight; ++i) {
        rowPtrs[i] = outData.data() + row_bytes * i;
      }

      png_read_image(png_ptr, rowPtrs.data());

      /* Clean up after the read,
       * and free any memory allocated */
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
 
      /* Close the file */
      fclose(fp);
 
      /* That's it */
      return true;
  }

  void GetGLTextureType(Format format, GLenum & glInternalFormat, GLenum & glDataFormat, GLenum&  glDataType, unsigned int & numberOfComponents) {
    switch(format) {
      case Format::Gray8:
        glInternalFormat = GL_LUMINANCE8;
        glDataFormat = GL_LUMINANCE;
        glDataType = GL_UNSIGNED_BYTE;
        numberOfComponents = 3;
        break;
      case Format::Gray16:
        glInternalFormat = GL_LUMINANCE16;
        glDataFormat = GL_LUMINANCE;
        glDataType = GL_UNSIGNED_SHORT;
        numberOfComponents = 3;
        break;
      case Format::GrayAlpha8:
        glInternalFormat = GL_LUMINANCE8;
        glDataFormat = GL_LUMINANCE;
        glDataType = GL_UNSIGNED_BYTE;
        numberOfComponents = 4;
        break;
      case Format::GrayAlpha16:
        glInternalFormat = GL_LUMINANCE16;
        glDataFormat = GL_LUMINANCE;
        glDataType = GL_UNSIGNED_SHORT;
        numberOfComponents = 4;
        break;
      case Format::RGB8:
        glInternalFormat = GL_RGB8;
        glDataFormat = GL_RGB;
        glDataType = GL_UNSIGNED_BYTE;
        numberOfComponents = 3;
        break;
      case Format::RGB16:
        glInternalFormat = GL_RGB16;
        glDataFormat = GL_RGB;
        glDataType = GL_UNSIGNED_SHORT;
        numberOfComponents = 3;
        break;
      case Format::RGBA8:
        glInternalFormat = GL_RGBA8;
        glDataFormat = GL_RGBA;
        glDataType = GL_UNSIGNED_BYTE;
        numberOfComponents = 4;
        break;
      case Format::RGBA16:
        glInternalFormat = GL_RGBA16;
        glDataFormat = GL_RGBA;
        glDataType = GL_UNSIGNED_SHORT;
        numberOfComponents = 4;
        break;
      }

  }



  bool CGLTexture::LoadFromFile(bool loadToGPU)
  {
    {
#ifdef USETHREADING
      mLoadToCPUMutex.lock();
      CMutexReleaser releaser(mLoadToCPUMutex);
#endif

      std::cout << "Loading texture " << mFile[0] << std::endl;

      for (size_t i = 0; i < mFile.size(); ++i) {
        if (mFile[i].find(".png") == std::string::npos) return false; //currently only png images can be loaded
      }

      if (mData.empty()) { //data was not already loaded

        mWidth = mHeight = 0; //maximum dimension
        for (size_t i = 0; i < mFile.size(); ++i) {

          std::shared_ptr<ImageData> imageData(new ImageData);
          bool success = LoadPngImage(mFile[i].c_str(), imageData->mWidth, imageData->mHeight, mHasAlpha, mFormat, imageData->mData);
          if (!success) {
            mData.clear();
            std::cout << "failed." << std::endl;
            return false;
          }

          mWidth = max(mWidth, imageData->mWidth);
          mHeight = max(mHeight, imageData->mHeight);

          mData[imageData->mWidth] = imageData;
        }
      }

      //std::cout << "Image loaded " << width << " " << height << " alpha " << hasAlpha << std::endl;

    }
    if (loadToGPU) {
      LoadToGPU();
    }

    if (!mCacheData) mData.clear();
    return true;
  }




  bool CGLTexture::LoadFromFile(std::string const & fileName, bool loadToGPU)
  {
    mFile.clear();
    mFile.push_back(fileName);
    return LoadFromFile(loadToGPU);
  }



  bool CGLTexture::LoadFromFile(std::vector<std::string> const & fileNames, bool loadToGPU)
  {
    mFile = fileNames;
    return LoadFromFile(loadToGPU);
  }


  void CGLTexture::SetFileName(std::string const & fileName)
  {
#ifdef USETHREADING
    mLoadToCPUMutex.lock();
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif
    mFile.clear();
    mFile.push_back(fileName);
  }



  void CGLTexture::SetFileName(std::vector<std::string> const & fileNames)
  {
#ifdef USETHREADING
    mLoadToCPUMutex.lock();
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif
    mFile = fileNames;
  }



  bool CGLTexture::IsLoadedToCPU(void) 
  {
#ifdef USETHREADING
    if (!mLoadToCPUMutex.try_lock()) return false;
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif
    return !mData.empty();
  }



//--------------------------------------------------------------------------------------------


  CGLMultiTexture::CGLMultiTexture(bool useMipmaps, bool cacheData)
    : mCacheData(cacheData)
    , mUseMipMaps(useMipmaps)
    , mTextureID(0)
  {
  }



  CGLMultiTexture::CGLMultiTexture(std::vector<std::string> const & fileName, bool useMipmaps, bool cacheData)
    : mUseMipMaps(useMipmaps)
    , mCacheData(cacheData)
    , mTextureID(0)
  {
    std::for_each(fileName.begin(), fileName.end(), [&](std::string const & path) {
      std::vector<std::string> paths;
      paths.push_back(path);
      mFiles.push_back(paths);
    });
  }



  CGLMultiTexture::CGLMultiTexture(std::vector<std::vector<std::string>> const & fileNames, bool cacheData)
    : mUseMipMaps(true)
    , mCacheData(cacheData)
    , mTextureID(0)
    , mFiles(fileNames)
  {
  }



  CGLMultiTexture::~CGLMultiTexture(void)
  {
    UnloadFromGPU();
  }



  //loads the texture from hdd, using previously specified filename (also loads data to VRAM)
  bool CGLMultiTexture::LoadFromFile(bool loadToGPU)
  {
#ifdef USETHREADING
    mLoadToCPUMutex.lock();
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif

    if (mFiles.empty()) return false;

    for (size_t i = 0; i < mFiles.size(); ++i) {
      for (size_t j = 0; j < mFiles[i].size(); ++j) {
        if (mFiles[i][j].find(".png") == std::string::npos) return false; //currently only png images can be loaded
      }
    }
    

    int width = 0;
    for (size_t imageIndex = 0; imageIndex < mFiles.size(); ++imageIndex) {

      //create texture object that only stores data (no mipmapping, not on GPU)
      CGLTexture texture(mFiles[imageIndex], false);

      //load data to RAM (not to GPU)
      bool success = texture.LoadFromFile(false);
      if (!success) {
        mData.clear();
        std::cout << "Unable to load png file" << std::endl;
        return false;
      }

      //set attributes
      if (imageIndex == 0 ) {
        //format should be equal for all textures!
        mWidth = texture.GetWidth();
        mHeight = texture.GetHeight();
        mFormat = texture.GetFormat();
        mHasAlpha = texture.GetHasAlpha();
        
      }

      //save data
      width = 2 * mWidth;

      while(width > 1) {
        width /= 2;

        //note the data must be symmetric, meaning each texture of the array must have the same number of mip map levels
        
         //construct element if not already there
        if (texture.GetData().find(width) != texture.GetData().end()) {
          if (mData[width].get() == nullptr) {
            mData[width].reset(new std::vector<unsigned char>);
          }

          try 
          {
            mData[width]->insert(mData[width]->end(), texture.GetData().at(width)->mData.begin(),texture.GetData().at(width)->mData.end());
          }
          catch(std::bad_alloc& ba)
          {
            std::cout << "Not enough memory available - Using 64bit might solve this problem. Error: " << ba.what() << std::endl;
            //assert(false);
            mData.clear();
            return false;
          }
          catch(...)
          {
            mData.clear();
            return false;
          }
        }
      }

    }
    
    if (loadToGPU) {
      LoadToGPU();
    }

    if (!mCacheData) mData.clear(); //this makes no sense, in case loadToGPU is false, of course
    return true;
  }



    //deletes texture from RAM
  void CGLMultiTexture::UnloadFromCPU(void)
  {
#ifdef USETHREADING
    mLoadToCPUMutex.lock();
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif
    mData.clear();
  }



  //loads the texture from hdd, using specified filename (also loads data to VRAM)
  bool CGLMultiTexture::LoadFromFile(std::vector<std::string> const & fileName, bool loadToGPU)
  {
    std::for_each(fileName.begin(), fileName.end(), [&](std::string const & path) {
      std::vector<std::string> paths;
      paths.push_back(path);
      mFiles.push_back(paths);
    });
    return LoadFromFile(loadToGPU);
  }



  //loads the texture from hdd, using specified filename (also loads data to VRAM)
  //filenames consists of a vector (number of images) of vector (number of mip map levels) of strings (filename)
  bool CGLMultiTexture::LoadFromFile(std::vector<std::vector<std::string>> const & fileNames, bool loadToGPU)
  {
    mFiles = fileNames;
    return LoadFromFile(loadToGPU);
  }



  void CGLMultiTexture::SetFileName(std::vector<std::string> const & fileName)
  {
#ifdef USETHREADING
    mLoadToCPUMutex.lock();
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif

    std::for_each(fileName.begin(), fileName.end(), [&](std::string const & path) {
      std::vector<std::string> paths;
      paths.push_back(path);
      mFiles.push_back(paths);
    });
  }



  void CGLMultiTexture::SetFileName(std::vector<std::vector<std::string>> const & fileNames)
  {
#ifdef USETHREADING
    mLoadToCPUMutex.lock();
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif
    mFiles = fileNames;
  }



  void CGLMultiTexture::LoadToGPU()
  {
#ifdef USETHREADING
    if (!mLoadToCPUMutex.try_lock()) return;
    CMutexReleaser releaser(mLoadToCPUMutex);
#endif
    if (mTextureID == 0) {
      glGenTextures(1, &mTextureID);
    }

    if (mTextureID == 0) {
      std::cout << "Texture could be initialized (no empty ID was found)" << std::endl;
      return;
    }

    if (glActiveTexture == nullptr)  {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        //std::cerr << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum internFormat, dataFormat, dataType;
    unsigned int numberOfComponents;
    GetGLTextureType(mFormat, internFormat, dataFormat, dataType, numberOfComponents);

    GLint mipmapLevel = 0;
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, mFiles.size() == 1 ? mUseMipMaps : true);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, mipmapLevel, internFormat, mWidth, mHeight, static_cast<GLsizei>(mFiles.size()), 0, dataFormat, dataType, mData[mWidth]->data());
    CHelper::CheckForError();

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, false);

    //load mipmaps
    if (mFiles[0].size() > 1) {
      int width = mWidth;
      int height = mHeight;
      while(width > 1 && height > 1) {
        width /= 2;
        height /= 2;
        ++mipmapLevel;
        if (mData.find(width) != mData.end()) {
          glTexImage3D(GL_TEXTURE_2D_ARRAY, mipmapLevel, internFormat, width, height, static_cast<GLsizei>(mFiles.size()), 0, dataFormat, dataType, mData[width]->data());
          CHelper::CheckForError();
        }
      }
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, (mUseMipMaps) ? GL_LINEAR_MIPMAP_LINEAR: GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }



  void CGLMultiTexture::UnloadFromGPU()
  {
    if (mTextureID != 0) {
      glDeleteTextures(1, &mTextureID);
    }

    mTextureID = 0;
  }



  bool CGLMultiTexture::IsLoadedToCPU(void) 
  {
    //if (!mLoadToCPUMutex.try_lock()) return false;
    //CMutexReleaser releaser(mLoadToCPUMutex);

    return !mData.empty();
  }

} //namespace Utils