#pragma once

#include "GLUtilsDefines.h"
#include <stdexcept>
#include <cassert>
#include <glm/glm.hpp>
#include <GL/glew.h>

namespace GLUtils {


class CGLShaderProgram {
public:
  GLUTILS_API CGLShaderProgram();
  GLUTILS_API CGLShaderProgram(GLuint vsshader);
  GLUTILS_API CGLShaderProgram(GLuint vsshader, GLuint fsshader);
  GLUTILS_API CGLShaderProgram(GLuint shaders[], size_t shaderCount);
  GLUTILS_API ~CGLShaderProgram();

  //gets the opengl program handle
  GLUTILS_API GLuint GetHandle() const {return mHandle;}

  //gets if program is linked
  GLUTILS_API bool IsLinked() const;

  //gets if the program is valid for the current opengl state
  GLUTILS_API bool IsValid() const;

  //gets if program is in use
  GLUTILS_API bool IsUsed() const;

  //link provided shaders to a a shader program
  template <size_t N> bool Link(const GLuint (shaders)[N]) {
    Link(shaders, N);
  }

  //link provided shader to a a shader program
  GLUTILS_API bool Link(GLuint shader);

  //link provided vertex and fragment shader to a a shader program
  GLUTILS_API bool Link(GLuint vsshader, GLuint fsshader);

  //link provided shaders to a a shader program
  GLUTILS_API bool Link(GLuint shaders[], size_t shaderCount);

  //destroy the shader program
  GLUTILS_API void Destroy();

  //use this shader program
  GLUTILS_API void Use();

  //Get uniform location of the provided uniform
  GLUTILS_API GLint GetUniformLocation(const char* name) const {assert(IsLinked()); return glGetUniformLocation(mHandle, name);}

  //query uniform locations
  template <size_t N> void GetUniformLocations(const char* (&uniformNames)[N], GLint (&uniformLocations)[N]) const {
    GetUniformLocations(uniformNames, uniformLocations, N);
  }

  GLUTILS_API void GetUniformLocations(const char* uniformNames[], GLint uniformLocations[], size_t uniformCount) const;

  //query the info log 
  GLUTILS_API const std::string& QueryInfoLog(std::string& logString) const;

  //set uniform prototypes
  template <typename T> void SetUniform(GLint location, const T& x) {
    //nop
  }
  template <typename T> void SetUniform(const char* uniformName, const T& x) {
    SetUniform(glGetUniformLocation(mHandle, uniformName), x);
  }
  
  //set uniform single value specializations
  template <> void SetUniform<bool>(GLint location, const bool& x) {
    assert(IsUsed());
    glUniform1i(location, x ? 1 : 0);
  }
  template <> void SetUniform<unsigned int>(GLint location, const unsigned int& x) {
    assert(IsUsed());
    glUniform1ui(location, x);
  }
  template <> void SetUniform<int>(GLint location, const int& x) {
    assert(IsUsed());
    glUniform1i(location, x);
  }
  template <> void SetUniform<float>(GLint location, const float& x) {
    assert(IsUsed());
    glUniform1f(location, x);
  }

  //set uniform vec2 specializations
  template <> void SetUniform<glm::ivec2>(GLint location, const glm::ivec2& x) {
    assert(IsUsed());
    glUniform2i(location, x.x, x.y);
  }
  template <> void SetUniform<glm::uvec2>(GLint location, const glm::uvec2& x) {
    assert(IsUsed());
    glUniform2ui(location, x.x, x.y);
  }
  template <> void SetUniform<glm::vec2>(GLint location, const glm::vec2& x) {
    assert(IsUsed());
    glUniform2f(location, x.x, x.y);
  }

  //set uniform vec3 specializations
  template <> void SetUniform<glm::ivec3>(GLint location, const glm::ivec3& x) {
    assert(IsUsed());
    glUniform3i(location, x.x, x.y, x.z);
  }
  template <> void SetUniform<glm::uvec3>(GLint location, const glm::uvec3& x) {
    assert(IsUsed());
    glUniform3ui(location, x.x, x.y, x.z);
  }
  template <> void SetUniform<glm::vec3>(GLint location, const glm::vec3& x) {
    assert(IsUsed());
    glUniform3f(location, x.x, x.y, x.z);
  }

  //set uniform vec4 specializations
  template <> void SetUniform<glm::ivec4>(GLint location, const glm::ivec4& x) {
    assert(IsUsed());
    glUniform4i(location, x.x, x.y, x.z, x.w);
  }
  template <> void SetUniform<glm::uvec4>(GLint location, const glm::uvec4& x) {
    assert(IsUsed());
    glUniform4ui(location, x.x, x.y, x.z, x.w);
  }
  template <> void SetUniform<glm::vec4>(GLint location, const glm::vec4& x) {
    assert(IsUsed());
    glUniform4f(location, x.x, x.y, x.z, x.w);
  }

  //matrix uniforms
  template <> void SetUniform<glm::mat2>(GLint location, const glm::mat2& x) {
    assert(IsUsed());
    glUniformMatrix2fv(location, 1, false, &x[0][0]);
  }
  template <> void SetUniform<glm::mat3>(GLint location, const glm::mat3& x) {
    assert(IsUsed());
    glUniformMatrix3fv(location, 1, false, &x[0][0]);
  }
  template <> void SetUniform<glm::mat4>(GLint location, const glm::mat4& x) {
    assert(IsUsed());
    glUniformMatrix4fv(location, 1, false, &x[0][0]);
  }

private:
  GLuint	mHandle;
};

} //namespace Utils