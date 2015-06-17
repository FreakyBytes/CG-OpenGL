#include "GuiPrecompiled.h"
#include "Model.h"
#include <iostream>

namespace Gui {

  CModel::CModel() 
    : mPositions()
    , mNormals()
    , mTexcoords()
    , mTangents()
  {
    mDiffuseTexture.reset(new GLUtils::CGLTexture());
    mNormalTexture.reset(new GLUtils::CGLTexture());
  }

  CModel::~CModel() {
    Release();
  }

  size_t CModel::GetMeshMemsize() const {
    return mPositions.size() * sizeof(glm::vec3) + 
         mNormals.size() * sizeof(glm::vec3) + 
         mTexcoords.size() * sizeof(glm::vec2) + 
         mFaces.size() * sizeof(face_type);
  }

  bool CModel::Create(std::string const & objfile, std::string const & aotexfile, std::string const & nmtexfile) {

    //release old stuff
    Release();

    std::cout << "Parse model file" << std::endl;
    if (!LoadObjfile(objfile)) {
      Release();
      return false;
    }
    std::cout << "  - verts: " << mPositions.size() << " | norms: " << mNormals.size() << 
      " | texcoords: " << mTexcoords.size() << " | faces: " << mFaces.size() << std::endl;


    std::cout << "  - compute tangent basis" << std::endl;
    ComputeTagentBasis();


    std::cout << "  - load diffuse texture map" << std::endl;
    mDiffuseTexture->LoadFromFile(aotexfile);
    if (mDiffuseTexture->GetTextureID() == 0) {
      Release();
      return false;
    }

    std::cout << "  - load tangent space normal map" << std::endl;
    mNormalTexture->LoadFromFile(nmtexfile);
    if (mNormalTexture->GetTextureID() == 0) {
      Release();
      return false;
    }
    
    return true;
  }

  void CModel::Release() {
    //delete gl resources
    mDisplayList.reset();
    mDiffuseTexture.reset(new GLUtils::CGLTexture());
    mNormalTexture.reset(new GLUtils::CGLTexture());
    
    //free model data
    mPositions.clear();
    mNormals.clear();
    mTexcoords.clear();
    mTangents.clear();
    mFaces.clear();
  }


  void CModel::Render() {

    //create display list, if not already there
    if (mDisplayList.get() == nullptr) {
      mDisplayList.reset(new GLUtils::CGLDisplayList());
      if (mDisplayList->IsValid()) {
        mDisplayList->BeginDefine();
        RenderMesh();
        mDisplayList->EndDefine();
      }
    }

    mDisplayList->Render();
  }



  void CModel::RenderWithTextures(bool glsl) {

    if (glsl) {
      //we only need to bind texture when using glsl
      glActiveTexture(GL_TEXTURE0);
      mDiffuseTexture->BindTexture();
      glActiveTexture(GL_TEXTURE1);
      mNormalTexture->BindTexture();
    }
    else {
      //if we use fixed function we need to enable texturing and the mode how color information is mixed (texture env)
      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_2D);
      mDiffuseTexture->BindTexture();
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    Render();

    //reset states
    if (glsl) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
      glDisable(GL_TEXTURE_2D);
    }
  }



  void CModel::RenderMesh() {

    //for each face send the four vertices to gpu (very slow)
    //however, its not important if compiled into display list
    glBegin(GL_TRIANGLES);
    for (glm::uint i = 0; i < mFaces.size(); ++i) {
      face_type& face = mFaces[i];

      glNormal3fv(&mNormals[face.n[0]].x);
      glTexCoord2fv(&mTexcoords[face.t[0]].x);
      glMultiTexCoord4fv(GL_TEXTURE1, &mTangents[face.n[0]].x);
      glVertex3fv(&mPositions[face.p[0]].x);

      glNormal3fv(&mNormals[face.n[1]].x);
      glTexCoord2fv(&mTexcoords[face.t[1]].x);
      glMultiTexCoord4fv(GL_TEXTURE1, &mTangents[face.n[1]].x);
      glVertex3fv(&mPositions[face.p[1]].x);

      glNormal3fv(&mNormals[face.n[2]].x);
      glTexCoord2fv(&mTexcoords[face.t[2]].x);
      glMultiTexCoord4fv(GL_TEXTURE1, &mTangents[face.n[2]].x);
      glVertex3fv(&mPositions[face.p[2]].x);
    }
    glEnd();
  }



  bool CModel::LoadObjfile(std::string const & objfile) {

    static const int BUFSZ = 512;

    FILE* fp;

    fopen_s(&fp, objfile.c_str(), "r");
    if (!fp) {
      std::cerr << "Failed to open model file: " << objfile << std::endl;
      return false;
    }

    char parse_buf[BUFSZ];
    float		val[4];
    face_type   face;
    int			fi[3];
    int			match;

    while (fscanf(fp, "%s", parse_buf) != EOF) {

      switch (parse_buf[0]) {
      case '#':
        //ignore comment line
        fgets( parse_buf, BUFSZ, fp);
        break;

        //parse vertex data
      case 'v':
        switch (parse_buf[1]) {

        case '\0':
          //vertex, 3 or 4 components (w is ignored)
          match = fscanf( fp, "%f %f %f %f", &val[0], &val[1], &val[2], &val[4]);
          if (match < 3 || match > 4) {
            std::cerr << "failed to parse model file: bad position format!" << std::endl;
            fclose(fp);
            return false;
          }
          mPositions.push_back(glm::vec3(val[0], val[1], val[2]));
          break;

        case 'n':
          //normal, 3 components
          match = fscanf( fp, "%f %f %f", &val[0], &val[1], &val[2]);
          if (match != 3) {
            std::cerr << "failed to parse model file: bad normal format!" << std::endl;
            fclose(fp);
            return false;
          }
          mNormals.push_back(glm::vec3(val[0], val[1], val[2]));
          break;

        case 't':
          //texcoord, 2 or 3 components (only 2 components are considered)
          match = fscanf( fp, "%f %f %f %f", &val[0], &val[1], &val[2]);
          if (match < 2 || match > 3) {
            std::cerr << "failed to parse model file: bad texture format!" << std::endl;
            fclose(fp);
            return false;
          }
          mTexcoords.push_back(glm::vec2(val[0], val[1]));
          break;
        }
        break;

      case 'f':
        //scan faces (all entries have the same format)
        for (int i=0; i < 3; i++) {
          //assume format (position/texcoord/normals)
          memset(fi, 0, sizeof(fi));
          match = fscanf(fp, "%d/%d/%d", &fi[0], &fi[1], &fi[2]);
          if (match != 3) {
            std::cerr << "failed to parse model file: bad face format!" << std::endl;
            fclose(fp);
            return false;
          }
          //remap to right position
          face.p[i] = (fi[0] > 0) ? (fi[0] - 1) : ((int)mPositions.size() - fi[0]);
          face.t[i] = (fi[1] > 0) ? (fi[1] - 1) : ((int)mTexcoords.size() - fi[1]);
          face.n[i] = (fi[2] > 0) ? (fi[2] - 1) : ((int)mNormals.size() - fi[2]);
        }
        mFaces.push_back(face);
        break;

        //ignored elements
      case 'u':
      case 's':
      case 'g':
      default:
        fgets( parse_buf, BUFSZ, fp);
      };
    }
    fclose(fp);
    return true;
  }

  
  void CModel::ComputeTagentBasis() {

    mTangents.resize(mNormals.size());

    //compute tangent space for all faces and accumulate per vertex
    std::vector<glm::vec3> tangents, bitangents;
    tangents.resize(mNormals.size());
    bitangents.resize(mNormals.size());
    
    for (glm::uint i=0; i < mFaces.size(); i++) {

      face_type& f = mFaces[i];
      glm::vec3 e1 = mPositions[f.p[1]] - mPositions[f.p[0]];
      glm::vec3 e2 = mPositions[f.p[2]] - mPositions[f.p[0]];

      glm::vec2 uve1 = mTexcoords[f.t[1]] - mTexcoords[f.t[0]];
      glm::vec2 uve2 = mTexcoords[f.t[2]] - mTexcoords[f.t[0]];

      float cp = uve1.y * uve2.x - uve1.x * uve2.y;
      if ( cp != 0.0f ) {
        float mul = 1.0f / cp;
        glm::vec3 tangent   = glm::normalize((e1 * -uve2.y + e2 * uve1.y) * mul);
        glm::vec3 bitangent = glm::normalize((e1 * -uve2.x + e2 * uve1.x) * mul);

        //add to per vertex tangent space
        for (int i=0; i < 3; i++) {
          tangents[f.n[i]] += tangent;
          bitangents[f.n[i]] += bitangent;
        }
      }
    }
    
    //bitangent can be computed by cross(N, T)*w (w = handedness) -> so compute w and encode T and w in 4D vector
    for (glm::uint i=0; i < mNormals.size(); i++) {
      const glm::vec3& N = mNormals[i];
      const glm::vec3& T = tangents[i];
      const glm::vec3& B = bitangents[i];

      //compute orthogonal basis by Gram-Schmidt
      glm::vec3 Tortho = glm::normalize(T - N * glm::dot(N, T));
      // Calculate handedness
      float	  w		 = (glm::dot(glm::cross(N, T), B) < 0.0f) ? -1.f : 1.f;

      //store
      mTangents[i] = glm::vec4(Tortho, w);
    }
  }

} //namespace vccg