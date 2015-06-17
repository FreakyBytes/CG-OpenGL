#pragma once

#include "GLUtilsDefines.h"
#include <string>
#include <stdexcept>

#include <GL/glew.h>
#include <glm/glm.hpp>


#if defined(_DEBUG) || defined(DEBUG)
  #define VCG_DEBUG_SHADER (1)
#endif


namespace GLUtils {


//helper for compile and link opengl programs
class CGLShaderUtils {
public:
  //exception class for compiler/link errors
  class CFailure : public std::runtime_error {
  public:
    GLUTILS_API CFailure(std::string const & msg):std::runtime_error(msg) {}
  };

  GLUTILS_API static const std::string& QueryShaderSource(GLuint shader, std::string& sourceString);

  //Creates and compile a shader from the provided source codes
  template <size_t N> static GLuint CompileSources(GLenum type, const std::string (&sources)[N]) {
    return CompileSources(type, sources, N);
  }

  //Creates and compile a shader from the provided source codes
  GLUTILS_API static GLuint CompileSources(GLenum type, const std::string sources[], size_t sourceCount);
  //Creates and compile a shader from the provided source code
  GLUTILS_API static GLuint CompileSource(GLenum type, const char* source);
  //Creates and compile a shader from the provided source file
  GLUTILS_API static GLuint CompileSourceFile(GLenum type, const char* file);
  //Creates and links a shader program from the provided shader objects
  GLUTILS_API static GLuint LinkShaders(GLuint vs, GLuint fs);
  //Creates and links a shader program from the provided shader objects
  GLUTILS_API static GLuint LinkShaders(GLuint shader[], int num);
    
  //Creates and links a shader program from the provided shader sources
  GLUTILS_API static GLuint LinkShaderSources(const char* vertssrc, const char* fragssrc) {return LinkShaderSources(vertssrc, 0, 0, 0, fragssrc);}

  //Creates and links a shader program from the provided shader sources
  GLUTILS_API static GLuint LinkShaderSources(const char* vertssrc, const char* geomssrc, const char* fragssrc) {return LinkShaderSources(vertssrc, 0, 0, geomssrc, fragssrc);}

  //Creates and links a shader program from the provided shader sources
  GLUTILS_API static GLuint LinkShaderSources(const char* vertssrc, const char* ctrlssrc, const char* evalssrc, const char* geomssrc, const char* fragssrc);

  //Creates and links a shader program from the provided shader files
  GLUTILS_API static GLuint LinkShaderFiles(const char* vertsfile, const char* fragsfile) {return LinkShaderFiles(vertsfile, 0, 0, 0, fragsfile);}

  //Creates and links a shader program from the provided shader files
  GLUTILS_API static GLuint LinkShaderFiles(const char* vertsfile, const char* geomsfile, const char* fragsfile) {return LinkShaderFiles(vertsfile, 0, 0, geomsfile, fragsfile);}

  //Creates and links a shader program from the provided shader files
  GLUTILS_API static GLuint LinkShaderFiles(const char* vertsfile, const char* ctrlsfile, const char* evalsfile, const char* geomsfile, const char* fragsfile);

  //get the info logs from the compiler
  GLUTILS_API static std::string ShaderInfolog(GLuint shader);
  GLUTILS_API static std::string ProgramInfolog(GLuint program);
private:
  CGLShaderUtils();  //forbidden
  ~CGLShaderUtils(); //static class
};

/************************************************************************/
/* some useful helper for delete shader/program handles                 */
/************************************************************************/
  
//Safe deletes a shader object and zero the handle
static inline void glSafeDeleteShader(GLuint& shader) {
  if (shader) {
    glDeleteShader(shader);
    shader = 0;
  }
}

//Safe deletes a a list of shader object and zero the handles
static inline void glSafeDeleteShaders(GLuint shaders[], size_t n) {
  for (size_t i=0; i < n; ++i) 
    glSafeDeleteShader(shaders[i]);
}

//Safe deletes a program object and zero the handle
static inline void glSafeDeleteProgram(GLuint& program) {
  if (program) {
    glDeleteProgram(program);
    program = 0;
  }
}


} //namespace Utils