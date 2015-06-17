#pragma once

#include "TerrainDefines.h"
#include "ViewFrustum.h"
#include "ErrorMetric.h"

#include <string>

namespace Terrain {
  enum ModelType  
  {ChunkedLOD,
   RasterModel
  };

  class CTerrainModel
  {
  public:
    CTerrainModel(void) : mNumberOfRenderedTriangles(0), mNumberOfTriangles(0) {}
    virtual ~CTerrainModel(void) {}

    //initialize the terrain model?    
    TERRAIN_API virtual bool Init(const char* hfcfile) = 0;
    //free all allocated resources?    
    TERRAIN_API virtual void Clear(void) = 0;
    //update the terrain (find cut through hierarchy)
    TERRAIN_API virtual void Update(CErrorMetric const & metric, GLUtils::CViewFrustum const & frustum) = 0;
    //render all active patches?    
    TERRAIN_API virtual void Render() const = 0;
    //render all bounds?    
    TERRAIN_API virtual void RenderBounds() const = 0;
    TERRAIN_API void GetBoundings(glm::vec3 & min, glm::vec3 & max) const {min = mTerrainMin; max = mTerrainMax;}
    TERRAIN_API ModelType GetModelType(void) {return mModelType;}
    //render all bounds?   
    TERRAIN_API virtual void RenderOutline() const {}
    TERRAIN_API std::string const & GetModelPath(void) {return mModelPath;}

    TERRAIN_API unsigned int GetNumberOfRenderedTriangles(void) const { return mNumberOfRenderedTriangles; }

  protected:
    CTerrainModel(CTerrainModel const & rhs);             //forbidden
    CTerrainModel & operator=(CTerrainModel const & rhs); //forbidden
    glm::vec3 mTerrainMin;
    glm::vec3 mTerrainMax;
    ModelType mModelType;
    std::string mModelPath;

    //counts the number of rendered triangles
    mutable unsigned int mNumberOfRenderedTriangles;
    mutable unsigned int mNumberOfTriangles;
  };
}