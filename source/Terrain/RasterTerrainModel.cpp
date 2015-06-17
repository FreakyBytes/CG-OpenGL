#include "TerrainPrecompiled.h"
#include "RasterTerrainModel.h"
#include <cstdio>
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/half_float.hpp>


namespace Terrain {

  //neighbor direction
  enum {
    SOUTH,
    EAST,
    NORTH,
    WEST,
  };

  static const std::pair<glm::uint, int> FSM_TABLE[4][4] = {
      { std::pair<glm::uint, int>(2, SOUTH), std::pair<glm::uint, int>(1,   -1), std::pair<glm::uint, int>(2,    -1),  std::pair<glm::uint, int>(1,  WEST) },
      { std::pair<glm::uint, int>(3, SOUTH), std::pair<glm::uint, int>(0, EAST), std::pair<glm::uint, int>(3,    -1),  std::pair<glm::uint, int>(0,    -1) },
      { std::pair<glm::uint, int>(0,    -1), std::pair<glm::uint, int>(3,   -1), std::pair<glm::uint, int>(0, NORTH),  std::pair<glm::uint, int>(3,  WEST) },
      { std::pair<glm::uint, int>(1,    -1), std::pair<glm::uint, int>(2, EAST), std::pair<glm::uint, int>(1, NORTH),  std::pair<glm::uint, int>(2,    -1) } };


  
  CRasterTerrainModel::Patch* NavigateFSM(CRasterTerrainModel::Patch* node, int c) {
    if (c == -1 || node == nullptr) {
      return node;
    }
    std::pair<glm::uint, int> cmd = FSM_TABLE[node->GetCIndex()][c];
    node = NavigateFSM(node->parent, cmd.second);
    if (node) 
      node = node->childs[cmd.first];

    return node;
  }

  CRasterTerrainModel::Patch::Patch(Patch* p) 
    : parent(p) 
  {
    child_mask  = 0;
    childs[0] = childs[1] = childs[2] = childs[3] = 0;
    glbuf = 0;
    tessLevel = 0;
  }



  CRasterTerrainModel::Patch::~Patch() 
  {
    Release();
    for (int i=0; i < 4; ++i)
      if (childs[i]) {
        delete childs[i];
      }
  }


  void CRasterTerrainModel::Patch::Commit() 
  {
    if (!IsCommited()) {
      glGenBuffers(1, &glbuf);

      //upload data
      glBindBuffer(GL_ARRAY_BUFFER, glbuf);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vbuf.size(), vbuf.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }

  void CRasterTerrainModel::Patch::Release() 
  {
    if (IsCommited()) {
      glDeleteBuffers(1, &glbuf);
      glbuf = 0;
    }
  }



  void CRasterTerrainModel::Patch::ReleaseChilds() 
  {
    for (glm::uint i=0; i < 4; ++i) {
      if (childs[i] && childs[i]->IsCommited()) {
        childs[i]->Release();
        childs[i]->ReleaseChilds();
      }
    }
  }



  float CRasterTerrainModel::Patch::DistanceTo(const glm::vec3& p) const 
  {
    glm::vec3 d= glm::vec3(
      (p.x < bbmin.x) ? (bbmin.x - p.x) : (p.x - bbmax.x),
      (p.y < bbmin.y) ? (bbmin.y - p.y) : (p.y - bbmax.y),
      (p.z < bbmin.z) ? (bbmin.z - p.z) : (p.z - bbmax.z));

    return glm::dot(d, d);
  }



  CRasterTerrainModel::CRasterTerrainModel()
    :mRoot(nullptr)
    , mPatchSize(0)
    , mTessLevels(0)
    , mOutlineIBO(0)
  {
    mTerrainMax = vec3(FLT_MIN);
    mTerrainMin = vec3(FLT_MAX);
    mModelType = ModelType::RasterModel;
  }



  CRasterTerrainModel::~CRasterTerrainModel() 
  {
    Clear();
  }


  void CRasterTerrainModel::UpdateBoundingBox(glm::vec3 const & point) 
  {
    if (point.x < mTerrainMin.x) mTerrainMin.x = point.x;
    if (point.y < mTerrainMin.y) mTerrainMin.y = point.y;
    if (point.z < mTerrainMin.z) mTerrainMin.z = point.z;

    if (point.x > mTerrainMax.x) mTerrainMax.x = point.x;
    if (point.y > mTerrainMax.y) mTerrainMax.y = point.y;
    if (point.z > mTerrainMax.z) mTerrainMax.z = point.z;
  }

  void CRasterTerrainModel::InitGLResources() {

    if (glGenBuffers == nullptr) {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        std::cout << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }
     
    if (mTessellationIBOs.empty()) {
      mTessellationIBOs.resize(mTessellationIBufs.size());
      glGenBuffers(static_cast<GLsizei>(mTessellationIBOs.size()), mTessellationIBOs.data());
      for (glm::uint i=0; i < mTessellationIBOs.size(); i++) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTessellationIBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::uint)*mTessellationIBufs[i].size(), mTessellationIBufs[i].data(), GL_STATIC_DRAW);
      }
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    if (mOutlineIBO == 0) {
      std::vector<glm::uint>	ib;
      int size = (mPatchSize+1);
      for (glm::uint i=0; i < mPatchSize; ++i) { ib.push_back(i);	}
      for (glm::uint i=0; i < mPatchSize; ++i) { ib.push_back(mPatchSize+i*size);	}
      for (glm::uint i=0; i < mPatchSize; ++i) { ib.push_back(mPatchSize-i+mPatchSize*size);	}
      for (glm::uint i=0; i < mPatchSize; ++i) { ib.push_back((mPatchSize-i)*size);	}

      glGenBuffers(1, &mOutlineIBO);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mOutlineIBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, ib.size()*sizeof(glm::uint), ib.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
  }

  static const char MAGIC[] = {'R','L','O','D'};

  bool CRasterTerrainModel::Init(const char* hfcfile) 
  {
    mModelPath = std::string(hfcfile);
    
    FILE* fp = nullptr;
    errno_t error = fopen_s(&fp, hfcfile, "rb");

    if (fp == nullptr || error != 0) {
      std::cerr << "failed to open terrain file " << hfcfile << "!" << std::endl;
      return false;
    }

    //read magic
    char sig[4];
    fread(sig, 1, 4, fp);
    if (memcmp(sig, MAGIC, 4) != 0) {
      fclose(fp);
      std::cerr << "terrain file is not a raster-lod!" << std::endl;
      return false;
    }

    //read compress flag
    uint compressFlag;
    fread(&compressFlag, sizeof(glm::uint), 1, fp);

    //read extent
    float extent[4];
    fread(extent, sizeof(float), 4, fp);
    std::cout << "terrain has an extent of: " << extent[0] << ", " <<extent[2] << "; " << extent[1] << ", " <<extent[3] << "!" << std::endl;
    mTerrainMin.x = extent[0];
    mTerrainMax.x = extent[1];
    mTerrainMin.z = extent[2];
    mTerrainMax.z = extent[3];

    //read patch size
    fread(&mPatchSize, sizeof(glm::uint), 1, fp);
    std::cout << "terrain has patchsize of: " << mPatchSize << "!" << std::endl;

    //read tess levels
    fread(&mTessLevels, sizeof(glm::uint), 1, fp);
    mTessellationIBufs.resize(mTessLevels*mTessLevels*4);	///mTessLevels*mTessLevels configs for the 4 childs
    std::cout << "terrain has a total count of tessellation buffers: " << mTessellationIBufs.size() << "!" << std::endl;

    glm::uint tessBufferSize = 0;
    for (glm::uint i=0; i < mTessellationIBufs.size(); ++i) {
      glm::uint count;
      fread(&count, sizeof(glm::uint), 1, fp);
      mTessellationIBufs[i].resize(count);
      fread(mTessellationIBufs[i].data(), sizeof(glm::uint), count, fp);
      tessBufferSize += count * sizeof(uint);
    }
    std::cout << "tessellation buffers consumes approx: " << tessBufferSize << "bytes!" << std::endl;


    std::cout << "\treading patch hierarchy |";
    mRoot = new Patch(0);
    if (!LoadHierarchy(mRoot, compressFlag == 1, fp)) {
      Clear();
      return false;
    }
    std::cout << "\r\treading patch hierarchy [done]" << std::endl;

    //assign neighbors
    AssignChildNeighbors(mRoot);
    return true;
  }



  void CRasterTerrainModel::Clear() 
  {

    mTessellationIBufs.clear();
    if (!mTessellationIBOs.empty()) {
      glDeleteBuffers(static_cast<GLsizei>(mTessellationIBOs.size()), mTessellationIBOs.data());
      mTessellationIBOs.clear();
    }
    if (mOutlineIBO) {
      glDeleteBuffers(1, &mOutlineIBO);
      mOutlineIBO = 0;
    }
    
    if (mRoot) {
      delete mRoot;
      mRoot = nullptr;
    }
  }



  void CRasterTerrainModel::Update(CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum) 
  {
    InitGLResources();

    mActivePatches.clear();
    RecursiveUpdate(mRoot, metric, frustum);
  }


  void CRasterTerrainModel::RecursiveUpdate(Patch * p, CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum) 
  {
    p->tessLevel = 0;
    if (frustum.Intersects(p->bbmin, p->bbmax)) {
      if (metric.Evaluate(p->bbmin, p->bbmax, p->error) && !p->IsLeaf()) {

        for (glm::uint i=0; i < 4; ++i)
          if (p->childs[i])
            RecursiveUpdate(p->childs[i], metric, frustum);
      }
      else {
        p->Commit();
        p->ReleaseChilds();
        p->propagateTessLevel(0);
        mActivePatches.push_back(p);
      }
    }
    else {
      p->Release();
      p->ReleaseChilds();
    }
  }


  void DrawBBox(Terrain::CRasterTerrainModel::Patch* patch) 
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



  void CRasterTerrainModel::RenderBounds() const 
  {

    glDisable(GL_LIGHTING);
  
    glColor3f(1.f, 1.f, 0.f);
    std::vector<Patch*>::const_iterator itr, itre = mActivePatches.end();
    for (itr = mActivePatches.begin(); itr != itre; ++itr) {
      DrawBBox(*itr);
    }
    glEnable(GL_LIGHTING);
  }


  void CRasterTerrainModel::RenderOutline() const {
    
    glEnableClientState(GL_VERTEX_ARRAY);
    

    glDisable(GL_LIGHTING);
    glColor3f(134.f/255.f, 198.f/255.f, 231.f/255.f);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.5f);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mOutlineIBO);
    std::vector<Patch*>::const_iterator itr, itre = mActivePatches.end();
    for (itr = mActivePatches.begin(); itr != itre; ++itr) {
      Patch* p = (*itr);

      glBindBuffer(GL_ARRAY_BUFFER, p->glbuf);
      glVertexPointer(3, GL_FLOAT, sizeof(CRasterTerrainModel::Vertex), 0);
      glDrawElements(GL_LINE_LOOP, static_cast<GLsizei>(mPatchSize*4), GL_UNSIGNED_INT, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
  }

  void CRasterTerrainModel::Render() const 
  {
    //glColor3f(1.0f, 0.0f, 0.0f);
    if (glPrimitiveRestartIndex == nullptr)  {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        std::cout << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
        return;
      }
    }
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(1.0, 1.0);
    

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(UINT_MAX);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    //reset number of rendered triangles
    mNumberOfRenderedTriangles = 0;

    std::vector<Patch*>::const_iterator itr, itre = mActivePatches.end();
    for (itr = mActivePatches.begin(); itr != itre; ++itr) {
      Patch* p = (*itr);

      //compute id
      uint hlv = p->neigbor[0] ? p->neigbor[0]->tessLevel : 0;
      uint vlv = p->neigbor[1] ? p->neigbor[1]->tessLevel : 0;
      uint cid = p->GetCIndex();
      uint tessID = vlv + hlv*mTessLevels + cid*(mTessLevels*mTessLevels);

      glBindBuffer(GL_ARRAY_BUFFER, p->glbuf);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTessellationIBOs[tessID]);
      glVertexPointer(3, GL_FLOAT, sizeof(CRasterTerrainModel::Vertex), 0);
      glNormalPointer(GL_FLOAT, sizeof(CRasterTerrainModel::Vertex), (void*)12);
      //glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>((*itr)->vbuf.size()));
      glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(mTessellationIBufs[tessID].size()), GL_UNSIGNED_INT, 0);

      //count number of rendered triangles
      mNumberOfRenderedTriangles += static_cast<unsigned int>(mTessellationIBufs[tessID].size());
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_PRIMITIVE_RESTART);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POLYGON_OFFSET_FILL);
  }



  bool CRasterTerrainModel::LoadHierarchy(Patch* node, bool compress, FILE* fp) {

    static const char LOADING_CHARS[] = {'|', '/', '-', '\\' };
    static int CUR_CHAR = 0;

    static std::vector<glm::half>	readbuf;

    glm::vec3 extent;
    glm::uint count, cmask;

    //read label
    fread(&node->label, sizeof(glm::uint), 1, fp);
    
    //read bounds
    fread(&node->bbmin.x, sizeof(glm::vec3), 1, fp);
    node->bbmin = vec3(node->bbmin.x, node->bbmin.y, node->bbmin.z);
    UpdateBoundingBox(node->bbmin);
    fread(&node->bbmax.x, sizeof(glm::vec3), 1, fp);
    node->bbmax = vec3(node->bbmax.x, node->bbmax.y, node->bbmax.z);
    UpdateBoundingBox(node->bbmax);
    fread(&node->error, sizeof(float), 1, fp);

    //read vertex data
    if (compress) {
      fread(&count, sizeof(glm::uint), 1, fp);
      readbuf.resize(count);
      fread(readbuf.data(), sizeof(glm::half), count, fp);
    
      //decompress
      extent = node->bbmax - node->bbmin;
      node->vbuf.reserve(count/6);
      for (uint i=0; i < readbuf.size(); i+=6) {
        Vertex v;
        v.p.x = node->bbmin.x + extent.x*static_cast<float>(readbuf[i]);
        v.p.y = (node->bbmin.y + extent.y*static_cast<float>(readbuf[i + 1]));
        v.p.z = (node->bbmin.z + extent.z* static_cast<float>(readbuf[i + 2])); 
        v.n.x = (2.f*static_cast<float>(readbuf[i + 3]) - 1.f);
        v.n.y = (2.f*static_cast<float>(readbuf[i + 4]) - 1.f);
        v.n.z = (2.f*static_cast<float>(readbuf[i + 5]) - 1.f);
        node->vbuf.push_back(v);
      }
    }
    else {
      fread(&count, sizeof(glm::uint), 1, fp);
      node->vbuf.resize(count);
      fread(node->vbuf.data(), sizeof(Vertex), count, fp);

      for (auto iter = node->vbuf.begin(); iter != node->vbuf.end(); ++iter) {
        
        Vertex vertex = *iter;
        iter->p.y = vertex.p.y;
        iter->p.z = vertex.p.z;
        iter->n.y = vertex.n.y;
        iter->n.z = vertex.n.z;

      }
    }

    //read child mask
    fread(&cmask, sizeof(glm::uint), 1, fp);
    node->child_mask = cmask;

    //read childs
    for (glm::uint i=0; i < 4; ++i) {
      Patch* p = nullptr;
      if (cmask & (1 << i)) {
        p = new Patch(node);
        if (!LoadHierarchy(p, compress, fp))
          return false;
      }
      node->childs[i] = p;
    }

    //detect read errors
    if (ferror(fp) != 0) {
      std::cerr << "failed to read patch hierchary!" << std::endl;
      return false;
    }

    CUR_CHAR = (CUR_CHAR+1) % 4;
    //std::cout << "\r\treading patch hierarchy " << LOADING_CHARS[CUR_CHAR];

    return true;
  }



  void CRasterTerrainModel::AssignChildNeighbors(Patch* patch) {

    if (patch->childs[0]) {
      patch->childs[0]->neigbor[0] = NavigateFSM(patch->childs[0], SOUTH);
      patch->childs[0]->neigbor[1] = NavigateFSM(patch->childs[0], WEST);
      AssignChildNeighbors(patch->childs[0]);
    }
    if (patch->childs[1]) {
      patch->childs[1]->neigbor[0] = NavigateFSM(patch->childs[1], SOUTH);
      patch->childs[1]->neigbor[1] = NavigateFSM(patch->childs[1], EAST);
      AssignChildNeighbors(patch->childs[1]);
    }
    if (patch->childs[2]) {
      patch->childs[2]->neigbor[0] = NavigateFSM(patch->childs[2], NORTH);
      patch->childs[2]->neigbor[1] = NavigateFSM(patch->childs[2], WEST);
      AssignChildNeighbors(patch->childs[2]);
    }
    if (patch->childs[3]) {
      patch->childs[3]->neigbor[0] = NavigateFSM(patch->childs[3], NORTH);
      patch->childs[3]->neigbor[1] = NavigateFSM(patch->childs[3], EAST);
      AssignChildNeighbors(patch->childs[3]);
    }
  }


} //namespace hfc
