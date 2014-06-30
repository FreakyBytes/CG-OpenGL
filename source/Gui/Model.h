#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "GLDisplayList.h"
#include "GLTexture.h"
#include <memory>

namespace Gui {

  /**
   * 3D scene model (triangle meshes with pos/normal/texcoord only)
   * Note: The model use the gl_MultiTexCoord[1] for the tangent
   */
  class CModel {
  public:
    //represents a face in the triangle mesh 
    struct face_type {
      glm::uint	p[3];	///< indices of the three vertex positions
      glm::uint	n[3];	///< indices of the three vertex normals
      glm::uint	t[3];	///< indices of the three vertex texcoords
    };

    //default constructor
    CModel();
    //default destructor
    ~CModel();

    //Gets the ambient occlusion texture
    GLUtils::CGLTexture const & GetDiffuseTexture() const { return *mDiffuseTexture.get(); }
    //Gets the normal map texture
    GLUtils::CGLTexture const &  GetNormalTexture() const  { return *mNormalTexture.get(); }
    //Gets the size of consumed memory
    size_t GetMeshMemsize() const;
    //loads the rock model and the associated textures
    bool Create(std::string const & objfile, std::string const & diffusetexfile, std::string const & normaltexfile);
    //release all allocated resources
    void Release();
    //render the mesh (binds aotex to texture unit 0 and binds nmtex to texture unit 1 if glsl is enabled)
    void RenderWithTextures(bool glsl = false);
    //render the mesh only 
    void Render();
  private:
    bool LoadObjfile(std::string const & objfile);
    void RenderMesh();
    void ComputeTagentBasis();

    std::vector<glm::vec3>	mPositions;
    std::vector<glm::vec2>	mTexcoords;
    std::vector<glm::vec3>	mNormals;
    std::vector<glm::vec4>	mTangents;
    std::vector<face_type>	mFaces;
    std::shared_ptr<GLUtils::CGLDisplayList> mDisplayList;
    std::shared_ptr<GLUtils::CGLTexture> mDiffuseTexture;
    std::shared_ptr<GLUtils::CGLTexture> mNormalTexture;
    GLuint					mFlags;
  };

}