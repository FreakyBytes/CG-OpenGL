#include "TerrainPrecompiled.h"
#include "ChunkedTerrainModel.h"
#include <cstdio>
#include <iostream>
#include <GL/glew.h>


namespace Terrain {

  CChunkedTerrainModel::Patch::Patch(Patch* p) 
    : parent(p) 
  {
    child_count  = 0;
    childs[0] = childs[1] = childs[2] = childs[3] = 0;
    glbufs[0] = glbufs[1] = 0;
  }



  CChunkedTerrainModel::Patch::~Patch() 
  {
    Release();
    for (int i=0; i < 4; ++i)
      if (childs[i] != 0) {
        delete childs[i];
      }
  }


  void CChunkedTerrainModel::Patch::Commit() 
  {
    if (glGenBuffers == nullptr) {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        std::cout << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }

    if (!IsCommited()) {
      glGenBuffers(2, glbufs);

      //upload data
      glBindBuffer(GL_ARRAY_BUFFER, glbufs[0]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vbuf.size(), vbuf.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glbufs[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::uint)*ibuf.size(), ibuf.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
  }



  void CChunkedTerrainModel::Patch::Release() 
  {
    if (IsCommited()) {
      glDeleteBuffers(2, glbufs);
      glbufs[0] = 0;
      glbufs[1] = 1;
    }
  }
  


  void CChunkedTerrainModel::Patch::ReleaseChilds() 
  {
    for (glm::uint i=0; i < child_count; ++i) {
      if (childs[i]->IsCommited()) {
        childs[i]->Release();
        childs[i]->ReleaseChilds();
      }
    }
  }



  float CChunkedTerrainModel::Patch::DistanceTo(const glm::vec3& p) const 
  {
    glm::vec3 d= glm::vec3(
      (p.x < bbmin.x) ? (bbmin.x - p.x) : (p.x - bbmax.x),
      (p.y < bbmin.y) ? (bbmin.y - p.y) : (p.y - bbmax.y),
      (p.z < bbmin.z) ? (bbmin.z - p.z) : (p.z - bbmax.z));

    return glm::dot(d, d);
  }



  CChunkedTerrainModel::CChunkedTerrainModel()
    :mRoots()
    
  {
    mTerrainMax = vec3(FLT_MIN);
    mTerrainMin = vec3(FLT_MAX);
    mModelType = ModelType::RasterModel;
  }



  CChunkedTerrainModel::~CChunkedTerrainModel() 
  {
    Clear();
  }


  void CChunkedTerrainModel::UpdateBoundingBox(glm::vec3 const & point) 
  {
    if (point.x < mTerrainMin.x) mTerrainMin.x = point.x;
    if (point.y < mTerrainMin.y) mTerrainMin.y = point.y;
    if (point.z < mTerrainMin.z) mTerrainMin.z = point.z;

    if (point.x > mTerrainMax.x) mTerrainMax.x = point.x;
    if (point.y > mTerrainMax.y) mTerrainMax.y = point.y;
    if (point.z > mTerrainMax.z) mTerrainMax.z = point.z;
  }


  bool CChunkedTerrainModel::Init(const char* hfcfile) 
  {
    mModelPath = std::string(hfcfile);
    FILE* fp = nullptr;
    errno_t error = fopen_s(&fp, hfcfile, "rb");

    if (fp == nullptr || error != 0) {
      std::cerr << "failed to open terrain file " << hfcfile << "!" << std::endl;
      return false;
    }

    //read number of root patches
    glm::uint num_roots;
    fread(&num_roots, sizeof(glm::uint), 1, fp);
    std::cout << "terrain has " << num_roots << " root nodes!" << std::endl;

    //recursive read all patch hierarchy for all roots
    mRoots.resize(num_roots);
    for (std::vector<Patch*>::iterator itr = mRoots.begin(); itr != mRoots.end(); ++itr) {
      Patch* root = new Patch(0);

      std::cout << "\treading patch hierarchy";
      if (!LoadHierarchy(root, fp)) {
        Clear();
        return false;
      }
      (*itr) = root;
      std::cout << std::endl;
    }

    return true;
  }



  void CChunkedTerrainModel::Clear() 
  {

    for (std::vector<Patch*>::iterator itr = mRoots.begin(); itr != mRoots.end(); ++itr)
      if ((*itr) != 0)
        delete (*itr);

    mRoots.clear();
  }



  void CChunkedTerrainModel::Update(CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum) 
  {
    mActivePatches.clear();
    for (std::vector<Patch*>::iterator itr = mRoots.begin(); itr != mRoots.end(); ++itr)
      RecursiveUpdate(*itr, metric, frustum);
  }


  void CChunkedTerrainModel::RecursiveUpdate(Patch * p, CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum) 
  {

    if (frustum.Intersects(p->bbmin, p->bbmax)) {
      if (metric.Evaluate(p->bbmin, p->bbmax, p->error) && p->child_count > 0) {

        for (glm::uint i=0; i < p->child_count; ++i)
          RecursiveUpdate(p->childs[i], metric, frustum);
      }
      else {
        p->Commit();
        p->ReleaseChilds();
        mActivePatches.push_back(p);
      }
    }
    else {
      p->Release();
      p->ReleaseChilds();
    }
  }
  
  

  void CChunkedTerrainModel::Render() const 
  {
    if (glPrimitiveRestartIndex == nullptr)  {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        std::cout << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(UINT_MAX);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    
    //resets number of rendered triangles
    mNumberOfRenderedTriangles = 0;

    std::vector<Patch*>::const_iterator itr, itre = mActivePatches.end();
    for (itr = mActivePatches.begin(); itr != itre; ++itr) {
      Patch* p = (*itr);
      glBindBuffer(GL_ARRAY_BUFFER, p->glbufs[0]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p->glbufs[1]);
      glVertexPointer(3, GL_FLOAT, sizeof(CChunkedTerrainModel::Vertex), 0);
      glNormalPointer(GL_FLOAT, sizeof(CChunkedTerrainModel::Vertex), (void*)12);
      glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>((*itr)->ibuf.size()), GL_UNSIGNED_INT, 0);
      mNumberOfRenderedTriangles += static_cast<unsigned int>((*itr)->ibuf.size());
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_PRIMITIVE_RESTART);
  }


  void DrawBBox(Terrain::CChunkedTerrainModel::Patch* patch) 
  {

    glBegin(GL_LINES);
      glVertex3f(patch->bbmin.x, patch->bbmin.y, patch->bbmin.z);
      glVertex3f(patch->bbmax.x, patch->bbmin.y, patch->bbmin.z);

      glVertex3f(patch->bbmax.x, patch->bbmin.y, patch->bbmin.z);
      glVertex3f(patch->bbmax.x, patch->bbmax.y, patch->bbmin.z);

      glVertex3f(patch->bbmax.x, patch->bbmax.y, patch->bbmin.z);
      glVertex3f(patch->bbmin.x, patch->bbmax.y, patch->bbmin.z);

      glVertex3f(patch->bbmin.x, patch->bbmax.y, patch->bbmin.z);
      glVertex3f(patch->bbmin.x, patch->bbmin.y, patch->bbmin.z);

      glVertex3f(patch->bbmin.x, patch->bbmin.y, patch->bbmax.z);
      glVertex3f(patch->bbmax.x, patch->bbmin.y, patch->bbmax.z);

      glVertex3f(patch->bbmax.x, patch->bbmin.y, patch->bbmax.z);
      glVertex3f(patch->bbmax.x, patch->bbmax.y, patch->bbmax.z);

      glVertex3f(patch->bbmax.x, patch->bbmax.y, patch->bbmax.z);
      glVertex3f(patch->bbmin.x, patch->bbmax.y, patch->bbmax.z);

      glVertex3f(patch->bbmin.x, patch->bbmax.y, patch->bbmax.z);
      glVertex3f(patch->bbmin.x, patch->bbmin.y, patch->bbmax.z);


      glVertex3f(patch->bbmin.x, patch->bbmin.y, patch->bbmin.z);
      glVertex3f(patch->bbmin.x, patch->bbmin.y, patch->bbmax.z);

      glVertex3f(patch->bbmax.x, patch->bbmin.y, patch->bbmin.z);
      glVertex3f(patch->bbmax.x, patch->bbmin.y, patch->bbmax.z);

      glVertex3f(patch->bbmax.x, patch->bbmax.y, patch->bbmin.z);
      glVertex3f(patch->bbmax.x, patch->bbmax.y, patch->bbmax.z);

      glVertex3f(patch->bbmin.x, patch->bbmax.y, patch->bbmin.z);
      glVertex3f(patch->bbmin.x, patch->bbmax.y, patch->bbmax.z);
    glEnd();
  }



  void CChunkedTerrainModel::RenderBounds() const 
  {

    glDisable(GL_LIGHTING);
  
    glColor3f(1.f, 1.f, 0.f);
    std::vector<Patch*>::const_iterator itr, itre = mActivePatches.end();
    for (itr = mActivePatches.begin(); itr != itre; ++itr) {
      DrawBBox(*itr);
    }
    glEnable(GL_LIGHTING);
  }



  bool CChunkedTerrainModel::LoadHierarchy(Patch* node, FILE* fp) {

    glm::uint count;

    fread(&node->bbmin.x, sizeof(glm::vec3), 1, fp);
    node->bbmin = vec3(node->bbmin.x, node->bbmin.y, node->bbmin.z);
    UpdateBoundingBox(node->bbmin);
    fread(&node->bbmax.x, sizeof(glm::vec3), 1, fp);
    node->bbmin = vec3(node->bbmax.x, node->bbmax.y, node->bbmax.z);
    UpdateBoundingBox(node->bbmax);
    fread(&node->error, sizeof(float), 1, fp);

    //alloc vbuf & ibuf
    fread(&count, sizeof(glm::uint), 1, fp);
    node->vbuf.resize(count);
    fread(&count, sizeof(glm::uint), 1, fp);
    node->ibuf.resize(count);

    //read data
    fread(node->vbuf.data(), sizeof(Vertex), node->vbuf.size(), fp);
    for (auto iter = node->vbuf.begin(); iter != node->vbuf.end(); ++iter) {

      Vertex vertex = *iter;
      iter->p.y = vertex.p.y;
      iter->p.z = vertex.p.z;
      iter->n.y = vertex.n.y;
      iter->n.z = vertex.n.z;

    }

    fread(node->ibuf.data(), sizeof(glm::uint), node->ibuf.size(), fp);

    //std::cout << ".";

    fread(&count, sizeof(glm::uint), 1, fp);
    node->child_count = count;

    for (glm::uint i=0; i < node->child_count; ++i) {
      node->childs[i] = new Patch(node);
      if (!LoadHierarchy(node->childs[i], fp))
        return false;
    }

    //detect read errors
    if (ferror(fp) != 0) {
      std::cerr << "failed to read patch hierchary!" << std::endl;
      return false;
    }

    return true;
  }


} //namespace hfc
