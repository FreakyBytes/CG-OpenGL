#pragma once

#include "GLUtilsDefines.h"
#include <string>
#include <GL/glew.h>
#include <vector>
#include <string>
#include <memory>
#include <map>
#ifdef USETHREADING
#include <mutex>
#endif


namespace GLUtils {


static enum Format {
    Gray8,
    Gray16,
    GrayAlpha8,
    GrayAlpha16,
    RGB8,
    RGB16,
    RGBA8,
    RGBA16,
};



class CGLTextureInterface
{
  public:
    GLUTILS_API CGLTextureInterface(void) {}
    GLUTILS_API virtual ~CGLTextureInterface(void) {}

    GLUTILS_API virtual bool LoadFromFile(bool loadToGPU = true) = 0;
    GLUTILS_API virtual bool IsLoadedToCPU(void) = 0;

  protected:
#ifdef USETHREADING
    std::mutex mLoadToCPUMutex;
#endif
};



class CGLTexture : public CGLTextureInterface
{
public:
  struct ImageData {
    std::vector<unsigned char> mData;
    int mWidth;
    int mHeight;
  };
  typedef std::map<int, std::shared_ptr<ImageData>> ImageMap;

  //creates an empty texture (e.g. for statics)
  GLUTILS_API CGLTexture(bool useMipmaps = true, bool cacheData = true);

  //use this constructor to create a texture, with or without automatically created mip maps
  GLUTILS_API CGLTexture(std::string const & fileName, bool useMipmaps = true, bool cacheData = true);

  //use this constructor to create a texture, with individually specified mip map levels (each entry of the vector represents a mip map slice of the 2D texture)
  GLUTILS_API CGLTexture(std::vector<std::string> const & fileNames, bool useMipmaps = true, bool cacheData = true);

  GLUTILS_API virtual ~CGLTexture(void);

  //loads the texture from hdd, using previously specified filename (also loads data to VRAM)
  GLUTILS_API virtual bool LoadFromFile(bool loadToGPU = true) override;

  //loads the texture from hdd, using specified filename (also loads data to VRAM)
  GLUTILS_API virtual bool LoadFromFile(std::string const & fileName, bool loadToGPU = true);
  GLUTILS_API virtual bool LoadFromFile(std::vector<std::string> const & fileNames, bool loadToGPU = true);

  //sets the filename
  GLUTILS_API virtual void SetFileName(std::string const & fileName);
  GLUTILS_API virtual void SetFileName(std::vector<std::string> const & fileNames);

  //loads data to VRAM
  GLUTILS_API virtual void LoadToGPU(void);

  //deletes the texture from VRAM, but not the data in RAM, if cacheData is true
  GLUTILS_API virtual void UnloadFromGPU(void);

  //deletes texture from RAM
  GLUTILS_API virtual void UnloadFromCPU(void);

  //bind this texture to the current texture unit slot
  GLUTILS_API virtual void BindTexture(void) const  {glBindTexture(GL_TEXTURE_2D, mTextureID);}
  GLUTILS_API virtual void UnbindTexture(void) const  { glBindTexture(GL_TEXTURE_2D, 0); }

  GLUTILS_API virtual GLuint GetTextureID(void) const {return mTextureID;}

  GLUTILS_API virtual bool IsLoadedToCPU(void) override;
  GLUTILS_API virtual bool IsLoadedToGPU(void) {return mTextureID != 0;}

  Format GetFormat(void) const {return mFormat;}
  int GetWidth(void) const {return mWidth;}
  int GetHeight(void) const {return mHeight;}
  bool GetHasAlpha(void) const {return mHasAlpha;}
  ImageMap const & GetData(void) const {return mData;}

private:
  //members
  std::vector<std::string> mFile;
  int mWidth;
  int mHeight;
  bool mHasAlpha;
  Format mFormat;
  GLuint mTextureID;
  ImageMap mData; //map of width as key and image data
  bool mCacheData;
  bool mUseMipMaps;
};



// multi texture is a texture array, that consists of multiple slices of 2D textures, that must have the same size
class CGLMultiTexture : public CGLTextureInterface
{
public:
  //creates an empty texture array (e.g. for statics)
  GLUTILS_API CGLMultiTexture(bool useMipmaps = true, bool cacheData = true);

  //use this constructor to create a texture array, with or without automatically created mip maps
  GLUTILS_API CGLMultiTexture(std::vector<std::string> const & fileNames, bool useMipmaps = true, bool cacheData = true);

  //use this constructor to create a texture array, with individually specified mip map levels (each entry of the vector represents a slice of the mipmapped 2D texture array)
  GLUTILS_API CGLMultiTexture(std::vector<std::vector<std::string>> const & fileNames, bool cacheData = true);

  GLUTILS_API virtual ~CGLMultiTexture(void);

    //loads the texture from hdd, using previously specified filename (also loads data to VRAM)
  GLUTILS_API virtual bool LoadFromFile(bool loadToGPU = true) override;

  //loads the texture from hdd, using specified filename (also might load data to VRAM)
  GLUTILS_API virtual bool LoadFromFile(std::vector<std::string> const & fileName, bool loadToGPU = true);
  GLUTILS_API virtual bool LoadFromFile(std::vector<std::vector<std::string>> const & fileName, bool loadToGPU = true);

  //sets the filename
  GLUTILS_API virtual void SetFileName(std::vector<std::string> const & fileName);
  GLUTILS_API virtual void SetFileName(std::vector<std::vector<std::string>> const & fileName);

  //loads data to VRAM
  GLUTILS_API virtual void LoadToGPU(void);

  //deletes the texture from VRAM, but not the data in RAM, if cacheData is true
  GLUTILS_API virtual void UnloadFromGPU(void);

    //deletes texture from RAM
  GLUTILS_API virtual void UnloadFromCPU(void);

  //bind this texture to the current texture unit slot
  GLUTILS_API virtual void BindTexture(void) const  {glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureID);}
  GLUTILS_API virtual void UnbindTexture(void) const  { glBindTexture(GL_TEXTURE_2D_ARRAY, 0); }
  GLUTILS_API virtual GLuint GetTextureID(void) const {return mTextureID;}

  GLUTILS_API virtual bool IsLoadedToCPU(void) override;
  GLUTILS_API virtual bool IsLoadedToGPU(void) {return mTextureID != 0;}

private:
  GLuint mTextureID;
  std::vector<std::vector<std::string>> mFiles;
  bool mCacheData;
  bool mUseMipMaps;
  int mWidth;
  int mHeight;
  bool mHasAlpha;
  Format mFormat;

  std::map<int, std::shared_ptr<std::vector<unsigned char>>> mData;
};


static void GetGLTextureType(Format format, GLenum & glInternalFormat, GLenum & glDataFormat, GLenum & glDataType, unsigned int & numberOfComponents);
static bool LoadPngImage(std::string const & fileName, int &outWidth, int &outHeight, bool &outHasAlpha, Format& outFormat, std::vector<unsigned char>& outData);

} //namespace Utils