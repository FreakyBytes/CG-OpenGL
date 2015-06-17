#include "GLUtilsPrecompiled.h"
#include "GLTesselator.h"


#include "GLUtilities.h"
#include "Helper.h"


namespace GLUtils {

  std::vector<std::vector<GLdouble>> CGLTesselator::mTempVertexData;

  CGLTesselator::CGLTesselator(void)
  {
  }



  CGLTesselator::~CGLTesselator(void)
  {
    mTempVertexData.clear();
  }



  bool CGLTesselator::TesselateSimplePolygon(std::vector<glm::vec3> const & simplePolygon)
  {
    GLUtesselator *tess = gluNewTess(); // create a tessellator
    if(!tess) return false;  // failed to create tessellation object, return 0

    // register callback functions
    gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK *)())CGLTesselator::TesselationBeginCB);
    gluTessCallback(tess, GLU_TESS_END, (void (CALLBACK *)())CGLTesselator::TesselationEndCB);
    gluTessCallback(tess, GLU_TESS_ERROR, (void (CALLBACK *)())CGLTesselator::TesselationErrorCB);
    gluTessCallback(tess, GLU_TESS_VERTEX, (void (CALLBACK *)())CGLTesselator::TesselationVertexCB);

    // tessellate a concave quad
    // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
    // and pointer to vertex data to be passed to vertex callback.
    // The second param is used only to perform tessellation, and the third
    // param is the actual vertex data to draw. It is usually same as the second
    // param, but It can be more than vertex coord, for example, color, normal
    // and UV coords which are needed for actual drawing.
    // Here, we are looking at only vertex coods, so the 2nd and 3rd params are
    // pointing same address.

    std::vector<std::vector<GLdouble>> tempVertices;
    tempVertices.resize(simplePolygon.size());
    size_t i = 0;
    for (auto iter = simplePolygon.begin(); iter != simplePolygon.end(); ++iter) {
      tempVertices[i].push_back(static_cast<GLdouble>(iter->x));
      tempVertices[i].push_back(static_cast<GLdouble>(iter->y));
      tempVertices[i].push_back(static_cast<GLdouble>(iter->z));
      ++i;
    }

    gluTessBeginPolygon(tess, 0);                   // with NULL data
        gluTessBeginContour(tess);
		for (auto iter = tempVertices.begin(); iter != tempVertices.end(); ++iter) {
            gluTessVertex(tess, iter->data(), iter->data());
          }
        gluTessEndContour(tess);
    gluTessEndPolygon(tess);

    gluDeleteTess(tess);        // delete after tessellation
    return !CHelper::CheckForError();
  }



  bool CGLTesselator::TesselatePolygonWithHoles(std::vector<glm::vec3> const & exterior, std::vector<std::vector<glm::vec3>> const & interior)
  {

    GLUtesselator *tess = gluNewTess(); // create a tessellator
    if(!tess) return false;         // failed to create tessellation object, return 0

    // register callback functions
    gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK *)())CGLTesselator::TesselationBeginCB);
    gluTessCallback(tess, GLU_TESS_END, (void (CALLBACK *)())CGLTesselator::TesselationEndCB);
    gluTessCallback(tess, GLU_TESS_ERROR, (void (CALLBACK *)())CGLTesselator::TesselationErrorCB);
    gluTessCallback(tess, GLU_TESS_VERTEX, (void (CALLBACK *)())CGLTesselator::TesselationVertexCB);

    //exterior
    std::vector<std::vector<GLdouble>> exteriorVertices;
    exteriorVertices.resize(exterior.size());
    size_t i = 0;
    for (auto iter = exterior.begin(); iter != exterior.end(); ++iter) {
      exteriorVertices[i].push_back(static_cast<GLdouble>(iter->x));
      exteriorVertices[i].push_back(static_cast<GLdouble>(iter->y));
      exteriorVertices[i].push_back(static_cast<GLdouble>(iter->z));
      ++i;
    }

    //interior (gaps)
    std::vector<std::vector<std::vector<GLdouble>>> interiorPolygons;
    for (auto polygonIter = interior.begin(); polygonIter != interior.end(); ++polygonIter) {

      std::vector<std::vector<GLdouble>> interiorVertices;
      interiorVertices.resize(polygonIter->size());
      i = 0;

      //gaps are defined counterclockwise
      for (auto vertexIter = polygonIter->crbegin(); vertexIter != polygonIter->crend(); ++vertexIter) {
        interiorVertices[i].push_back(static_cast<GLdouble>(vertexIter->x));
        interiorVertices[i].push_back(static_cast<GLdouble>(vertexIter->y));
        interiorVertices[i].push_back(static_cast<GLdouble>(vertexIter->z));
        ++i;
      }

      interiorPolygons.push_back(interiorVertices);
    }

    // tessellate and compile a concave quad into display list
    gluTessBeginPolygon(tess, 0);                       // with NULL data
        gluTessBeginContour(tess);                      // outer polygon
		for (auto iter = exteriorVertices.begin(); iter != exteriorVertices.end(); ++iter) {
          gluTessVertex(tess, iter->data(), iter->data());
        }
        gluTessEndContour(tess);

        for (auto polygonIter = interiorPolygons.begin(); polygonIter != interiorPolygons.end(); ++polygonIter) {
          gluTessBeginContour(tess);                      // inner polygon (hole)
		  for (auto vertexIter = polygonIter->begin(); vertexIter != polygonIter->end(); ++vertexIter) {
              gluTessVertex(tess, vertexIter->data(), vertexIter->data());
            }
          gluTessEndContour(tess);
        }
    gluTessEndPolygon(tess);

    gluDeleteTess(tess);        // delete after tessellation

    return !CHelper::CheckForError();
  }




  // convert enum of OpenGL primitive type to a string(char*)
  // OpenGL supports only 10 primitive types.
  const char* CGLTesselator::GetPrimitiveType(GLenum type)
  {
    switch(type)
    {
    case 0x0000:
        return "GL_POINTS";
    case 0x0001:
        return "GL_LINES";
    case 0x0002:
        return "GL_LINE_LOOP";
    case 0x0003:
        return "GL_LINE_STRIP";
    case 0x0004:
        return "GL_TRIANGLES";
    case 0x0005:
        return "GL_TRIANGLE_STRIP";
    case 0x0006:
        return "GL_TRIANGLE_FAN";
    case 0x0007:
        return "GL_QUADS";
    case 0x0008:
        return "GL_QUAD_STRIP";
    case 0x0009:
        return "GL_POLYGON";
    }

    return "";
  }



  void CALLBACK CGLTesselator::TesselationBeginCB(GLenum which)
  {
    glBegin(which);
  }



  void CALLBACK CGLTesselator::TesselationEndCB()
  {
    glEnd();
  }



  void CALLBACK CGLTesselator::TesselationVertexCB(const GLvoid *data)
  {
    // cast back to double type
    const GLdouble *ptr = (const GLdouble*)data;
    glVertex3dv(ptr);
  }



  void CALLBACK CGLTesselator::TesselationColoredVertexCB(const GLvoid *data)
  {
    // cast back to double type
    const GLdouble *ptr = (const GLdouble*)data;
    glColor3dv(ptr+3);
    glVertex3dv(ptr);
  }



  
  // Combine callback is used to create a new vertex where edges intersect.
  // In this function, copy the vertex data into local array and compute the
  // color of the vertex. And send it back to tessellator, so tessellator pass it
  // to vertex callback function.
  //
  // newVertex: the intersect point which tessellator creates for us
  // neighborVertex[4]: 4 neighbor vertices to cause intersection (given from 3rd param of gluTessVertex()
  // neighborWeight[4]: 4 interpolation weights of 4 neighbor vertices
  // outData: the vertex data to return to tessellator
  void CALLBACK CGLTesselator::TesselationCombinedCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4], const GLfloat neighborWeight[4], GLdouble **outData)
  {
    // copy new intersect vertex to local array
    // Because newVertex is temporal and cannot be hold by tessellator until next
    // vertex callback called, it must be copied to the safe place in the app.
    std::vector<GLdouble> tempVertices;
    tempVertices.resize(6);
    mTempVertexData.push_back(tempVertices);
    std::vector<GLdouble> & vertices = mTempVertexData.at(mTempVertexData.size() - 1);
    vertices[0] = newVertex[0];
    vertices[1] = newVertex[1];
    vertices[2] = newVertex[2];


    // compute vertex color with given weights and colors of 4 neighbors
    // the neighborVertex[4] must hold required info, in this case, color.
    // neighborVertex was actually the third param of gluTessVertex() and is
    // passed into here to compute the color of the intersect vertex.
    vertices[3] = neighborWeight[0] * neighborVertex[0][3] +   // red
                  neighborWeight[1] * neighborVertex[1][3] +
                  neighborWeight[2] * neighborVertex[2][3] +
                  neighborWeight[3] * neighborVertex[3][3];
    vertices[4] = neighborWeight[0] * neighborVertex[0][4] +   // green
                  neighborWeight[1] * neighborVertex[1][4] +
                  neighborWeight[2] * neighborVertex[2][4] +
                  neighborWeight[3] * neighborVertex[3][4];
    vertices[5] = neighborWeight[0] * neighborVertex[0][5] +   // blue
                  neighborWeight[1] * neighborVertex[1][5] +
                  neighborWeight[2] * neighborVertex[2][5] +
                  neighborWeight[3] * neighborVertex[3][5];


    // return output data (vertex coords and others)
    *outData = vertices.data();   // assign the address of new intersect vertex
  }
  


  void CALLBACK CGLTesselator::TesselationErrorCB(GLenum errorCode)
  {
      const GLubyte *errorStr;

      errorStr = gluErrorString(errorCode);
      std::cout << "Error: " << errorStr << std::endl;
  }

}