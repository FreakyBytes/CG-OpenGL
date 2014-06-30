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


  class CChunkedTerrainModel : public CTerrainModel {
  public:
    struct Vertex {
      glm::vec3 p;
      glm::vec3 n;
    };

    struct Patch {
      //patch properties
      glm::vec3				bbmin;
      glm::vec3				bbmax;
      float 					error;
      std::vector<Vertex>		vbuf;
      std::vector<glm::uint>	ibuf;
      Patch* 					parent;
      glm::uint 				child_count;
      Patch*					childs[4];
      glm::uint				glbufs[2];

      Patch(Patch* p);
      ~Patch();

      //gets if the patch is commited to GPU
      bool IsCommited() const {return glbufs[0] != 0;}
      //commit data to gpu
      void Commit();
      //release gpu data of patch and there childs
      void Release();
      //release gpu data of child patches (recursive)
      void ReleaseChilds();

      //computes distance of p to the patch
      float DistanceTo(const glm::vec3& p) const;
      //gets if patch is leaf
      bool  IsLeaf() const {return child_count == 0;}
    };


    TERRAIN_API CChunkedTerrainModel();
    TERRAIN_API virtual ~CChunkedTerrainModel();

    //get root patch
    size_t RootCount() const { return mRoots.size(); }
    Patch * Root(size_t i) {return mRoots[i];	}
    const Patch * Root(size_t i) const {	return mRoots[i];	}

    //initialize the terrain model
    TERRAIN_API virtual bool Init(const char* hfcfile) override;
    //free all allocated resources
    TERRAIN_API virtual void Clear() override;

    //update the terrain (find cut through hierarchy)
    TERRAIN_API virtual void Update(CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum) override;
    //render all active patches
    TERRAIN_API virtual void Render() const override;
    //render all bounds
    TERRAIN_API virtual void RenderBounds() const override;
  private:
    CChunkedTerrainModel(CChunkedTerrainModel const & rhs);             //forbidden
    CChunkedTerrainModel & operator=(CChunkedTerrainModel const & rhs); //forbidden

    void UpdateBoundingBox(glm::vec3 const & point);

    bool LoadTerrainProperties(FILE* fp);
    bool LoadHierarchy(Patch* node, FILE* fp);
    void RecursiveUpdate(Patch* p, CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum);


    std::vector<Patch*> mRoots;
    std::vector<Patch*> mActivePatches;
  };


} //namespace hfc
