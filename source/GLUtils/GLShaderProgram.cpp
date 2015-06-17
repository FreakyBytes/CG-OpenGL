#include "GLUtilsPrecompiled.h"
#include "GLShaderProgram.h"

#include "Helper.h"


namespace GLUtils {


CGLShaderProgram::CGLShaderProgram():mHandle(0) {
}



CGLShaderProgram::CGLShaderProgram(GLuint vsshader):mHandle(0) {
  Link(vsshader);
}



CGLShaderProgram::CGLShaderProgram(GLuint vsshader, GLuint fsshader):mHandle(0) {
  Link(vsshader, fsshader);

}



CGLShaderProgram::CGLShaderProgram(GLuint shaders[], size_t shaderCount):mHandle(0) {
  Link(shaders, shaderCount);
}



CGLShaderProgram::~CGLShaderProgram() {
  Destroy();
}



bool CGLShaderProgram::IsLinked() const {
  if (mHandle != 0) {

    GLint state;
    glGetProgramiv(mHandle, GL_LINK_STATUS, &state);
    return state == GL_TRUE;
  }
  return false;
}



bool CGLShaderProgram::IsValid() const {
  if (mHandle != 0) {

    GLint state;
    glValidateProgram(mHandle);
    glGetProgramiv(mHandle, GL_VALIDATE_STATUS, &state);
    return state == GL_TRUE;
  }
  return false;
}



bool CGLShaderProgram::IsUsed() const 
{
  
  if (mHandle != 0) {
    GLint curProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &curProgram);
    return GLuint(curProgram) == mHandle;
  }
  return false;
}



bool CGLShaderProgram::Link(GLuint shader) {
  return Link(&shader, 1);
}



bool CGLShaderProgram::Link(GLuint vsshader, GLuint fsshader) {
  GLuint shaders[] = { vsshader, fsshader };
  return Link(shaders, 2);
}



bool CGLShaderProgram::Link(GLuint shaders[], size_t shaderCount) {
  
  if (shaders == nullptr || shaderCount == 0) 
    throw CFailure("invalid arguments!");

  Destroy();
  GLuint program = glCreateProgram();
  CHelper::CheckForError();

  //assign shaders
  for (size_t i=0; i < shaderCount; i++) {
    if (shaders[i] != 0) {
      glAttachShader(program, shaders[i]);
    }
    CHelper::CheckForError();
  }

  //link program
  glLinkProgram(program);
  CHelper::CheckForError();

  //query status
  int query;
  glGetProgramiv(program, GL_LINK_STATUS, &query);
  if (query == GL_FALSE) {
    mHandle = program;
    std::string log;
    QueryInfoLog(log);
    mHandle = 0;
    glDeleteProgram(program);
    throw CFailure(log);
  }

  mHandle = program;
  return true;
}

void CGLShaderProgram::Destroy() {
  if (mHandle != 0) {
    glDeleteProgram(mHandle);
    mHandle = 0;
    CHelper::CheckForError();
  }
}

void CGLShaderProgram::Use() {
  if (mHandle != 0) {
    glUseProgram(mHandle);
#if defined(_DEBUG) 
    if (!IsValid()) {
      std::string error;
      QueryInfoLog(error);
      std::cerr << "program (" << mHandle << ") is invalid for current state: " << error << std::endl;
    }
#endif
  }
}

  
void CGLShaderProgram::GetUniformLocations(const char* uniformNames[], GLint uniformLocations[], size_t uniformCount) const {
  
  if (!IsLinked()) 
    throw std::logic_error("shader program is not linked!");

  for (size_t i=0; i < uniformCount; ++i) {
    uniformLocations[i] = glGetUniformLocation(mHandle, uniformNames[i]);
    #if defined(_DEBUG) 
      if (uniformLocations[i] < 0) {
        std::cerr << "unknown uniform: " << uniformNames[i] << std::endl;
      }
    #endif
  }
}

const std::string& CGLShaderProgram::QueryInfoLog(std::string& logString) const {

  logString.clear();
  if (mHandle != 0) {
    int query;
    glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &query);
    if (query > 0) {
      logString.resize(query);
      glGetProgramInfoLog(mHandle, static_cast<GLsizei>(logString.size()), &query, &logString[0]);
    }
  }
  return logString;
}


} //namespace Utils
