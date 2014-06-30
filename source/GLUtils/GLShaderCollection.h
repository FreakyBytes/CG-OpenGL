#pragma once

#include "GLUtilsDefines.h"
#include "GLShaderProgram.h"
#include <string>
#include <vector>
#include <map>

namespace GLUtils {


//macro for opengl shaders 
typedef std::pair<std::string, std::string>		GLShaderMacro;

class CGLShaderCollection {
public:
  typedef std::map<std::string, GLuint> ContainerType;
  typedef ContainerType::iterator Iterator;
  typedef ContainerType::const_iterator ConstIterator;

  //Creates a new empty shader collection
  GLUTILS_API CGLShaderCollection();
  //Destroy the shader collection
  GLUTILS_API ~CGLShaderCollection();


  //gets if the collection is empty
  bool IsEmpty() const {return mContainer.empty();}
  
  //Gets begin iterator for iterate through collection
  Iterator GetBeginIterator() {return mContainer.begin();}
  ConstIterator GetBeginIterator() const {return mContainer.begin();}

  //Gets end iterator for marking the end of the collection
  Iterator GetEndIterator() {return mContainer.end();}
  ConstIterator GetEndIterator() const {return mContainer.end();}

  //gets the number of shaders in the collection
  size_t GetShaderCount() const {return mContainer.size();}
  
  //get the shader handle
  GLUTILS_API GLuint GetShader(const std::string& shaderName) const;

  //read from the provided shader collection input and compile the shaders
  template <size_t N> void CreateFromSourceStream(std::istream& stream, const GLShaderMacro (&marcos)[N]) {
    CreateFromSourceStream(source, macros, N);
  }
  //parse the provided shader collection source and compile the shaders
  template <size_t N> void CreateFromSourceString(const std::string& source, const GLShaderMacro (&marcos)[N]) {
    CreateFromSourceString(source, macros, N);
  }
  //parse the provided shader collection source and compile the shaders
  template <size_t N> void CreateFromSourceFile(const std::string& fileaname, const GLShaderMacro (&marcos)[N]) {
    CreateFromSourceFile(fileaname, macros, N);
  }

  //read from the provided shader collection input and compile the shaders
  GLUTILS_API void CreateFromSourceStream(std::istream& stream, const GLShaderMacro macros[] = nullptr, size_t macroCount = 0);
  //parse the provided shader collection source and compile the shaders
  GLUTILS_API void CreateFromSourceString(const std::string& source, const GLShaderMacro macros[] = nullptr, size_t macroCount = 0);
  //parse the provided shader collection source file and compile the shaders
  GLUTILS_API void CreateFromSourceFile(const std::string& filename, const GLShaderMacro macros[] = nullptr, size_t macroCount = 0);

  //delete all shader and clears the collection
  GLUTILS_API void Clear();

  //link all shader to one shader program
  GLUTILS_API bool LinkShaders(CGLShaderProgram& program);

  //link a vertex and a fragment shader to one shader program
  GLUTILS_API bool LinkShaders(CGLShaderProgram& program, const std::string& vertexShader, const std::string& fragmentShader) {return LinkShaders(program, vertexShader, "", "", "", fragmentShader);}

  //link a vertex, geometry and a fragment shader to one shader program
  GLUTILS_API bool LinkShaders(CGLShaderProgram& program, const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader) {return LinkShaders(program, vertexShader, "", "", geometryShader, fragmentShader);}

  //link a vertex, control, evaluation, geometry and a fragment shader to one shader program
  GLUTILS_API bool LinkShaders(CGLShaderProgram& program, const std::string& vertexShader, const std::string& controlShader, const std::string& evaluationShader, const std::string& geometryShader, const std::string& fragmentShader);

private:
  CGLShaderCollection(const CGLShaderCollection&);
  CGLShaderCollection operator=(const CGLShaderCollection&);

  ContainerType mContainer;
};

} //namespace Utils