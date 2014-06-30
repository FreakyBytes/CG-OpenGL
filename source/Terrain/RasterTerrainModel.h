#pragma once

#include "TerrainDefines.h"

#include <glm/glm.hpp>
#include <vector>
#include "ViewFrustum.h"
#include "ErrorMetric.h"
#include <GL/glew.h>
#include "TerrainModel.h"


class GLUtils::CViewFrustum;

namespace Terrain {


  class CRasterTerrainModel : public CTerrainModel 
  {
  public:
    typedef std::vector<uint>		IndexBuffer;
    struct Vertex {
      glm::vec3 p;
      glm::vec3 n;
    };

    struct Patch {
      //patch properties
      uint					label;
      glm::vec3				bbmin;
      glm::vec3				bbmax;
      float 					error;
      std::vector<Vertex>		vbuf;
      Patch* 					parent;
      uint					child_mask;
      Patch*					childs[4];
      Patch*					neigbor[2];
      glm::uint				glbuf;
      uint					tessLevel;

      Patch(Patch* p);
      ~Patch();

      //gets if the patch is commited to GPU
      bool IsCommited() const {return glbuf != 0;}
      //commit data to gpu
      void Commit();
      //release gpu data of patch and there childs
      void Release();
      //release gpu data of child patches (recursive)
      void ReleaseChilds();
      //Gets label of this node
      uint GetLabel() {
        return label;
      }
      void propagateTessLevel(uint l) {
        tessLevel = l;
        for (int i=0; i < 4; ++i)
          if (childs[i])
            childs[i]->propagateTessLevel(l+1);
      }
      //gets index of child node in the parent
      int GetCIndex() {
        return (label-1) & 0x03;
      }

      //computes distance of p to the patch
      float DistanceTo(const glm::vec3& p) const;
      //gets if patch is leaf
      bool  IsLeaf() const {return child_mask == 0;}
    };


    TERRAIN_API CRasterTerrainModel();
    TERRAIN_API virtual ~CRasterTerrainModel();

    //get root patch
    Patch * GetRoot() {return mRoot;	}
    const Patch * GetRoot() const {	return mRoot;	}

    //initialize the terrain model
    TERRAIN_API virtual bool Init(const char* rlodfile) override;
    //free all allocated resources
    TERRAIN_API virtual void Clear() override;

    //update the terrain (find cut through hierarchy)
    TERRAIN_API virtual void Update(CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum) override;
    //render all active patches
    TERRAIN_API void Render() const;
    //render all bounds
    TERRAIN_API virtual void RenderOutline() const override;
    //render all bounds
    TERRAIN_API virtual void RenderBounds() const override;
  private:
    CRasterTerrainModel(CRasterTerrainModel const & rhs);             //forbidden
    CRasterTerrainModel & operator=(CRasterTerrainModel const & rhs); //forbidden

    void InitGLResources();
    void UpdateBoundingBox(glm::vec3 const & point);

    bool LoadTerrainProperties(FILE* fp);
    bool LoadHierarchy(Patch* node, bool compress, FILE* fp);
    void AssignChildNeighbors(Patch* patch);
    void RecursiveUpdate(Patch* p, CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum);


    Patch* mRoot;
    std::vector<Patch*> mActivePatches;
    
    std::vector<IndexBuffer>	mTessellationIBufs;
    std::vector<GLuint>			mTessellationIBOs;
    glm::uint					mOutlineIBO;
    glm::uint					mPatchSize;
    glm::uint					mTessLevels;
  };


} //namespace hfc
