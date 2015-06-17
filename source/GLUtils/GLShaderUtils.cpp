#include "GLUtilsPrecompiled.h"
#include "GLShaderUtils.h"

//#define VCG_USE_GL2
//#define VCG_PRINT_SHADERLOG


namespace GLUtils {


  static void StaticInitialize() {
    if (glCreateShader == nullptr) {
      GLenum glew_err = glewInit();
      if (glew_err != GLEW_NO_ERROR) {
        throw std::runtime_error("failed to initialize opengl extension wrapper!");
      }
    }
  }

  const std::string& CGLShaderUtils::QueryShaderSource(GLuint shader, std::string& sourceString) {
 
    int len;
    glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &len);

    sourceString.resize(len);
    glGetShaderSource(shader, static_cast<GLsizei>(sourceString.size()), &len, &sourceString[0]);
    return sourceString;	
  }


  GLuint CGLShaderUtils::CompileSources(GLenum type, const std::string sources[], size_t sourceCount) {

    StaticInitialize();

    std::vector<int>			sourceLengths(sourceCount);
    std::vector<const char*>	sourceStrings(sourceCount);

    for (size_t i=0; i < sourceCount; ++i) {
      sourceLengths[i] = static_cast<int>(sources[i].size());
      sourceStrings[i] = sources[i].c_str();
    }

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, static_cast<GLsizei>(sourceCount), sourceStrings.data(), sourceLengths.data());	
    glCompileShader(shader);

    //query status
    int query;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &query);
    if (query == GL_FALSE) {
      std::string infolog = ShaderInfolog(shader);
      glDeleteShader(shader);
      throw CGLShaderUtils::CFailure(infolog);
    }

    return shader;
  }

  GLuint CGLShaderUtils::CompileSource(GLenum type, const char* source) 
  {
  
    StaticInitialize();	

    GLuint shader = glCreateShader(type);

    //setup default glsl version
  #if defined(VCG_USE_GL2)
    std::string preamble = "#version 150 compatibility\n";		
  #else
    std::string preamble = "#version 420 compatibility\n";
  #endif

    //enable debugging
  #if defined(DEBUG_GLSHADER)
    preamble += "#define DEBUG (1)\n";
  #endif

    //assign sources
    const char* sources[]		= { preamble.c_str(), source };
    int		  source_lens[]	= { -1, -1 }; //our source strings are null terminated (see gl docs)
    glShaderSource(shader, 2, sources, source_lens);

    //compile shader
    glCompileShader(shader);

    //query status
    int query;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &query);
    if (query == GL_FALSE) {
      std::string infolog = ShaderInfolog(shader);
      glDeleteShader(shader);
      std::cout << "Shader failed to compile:" << infolog << std::endl;
      throw CGLShaderUtils::CFailure(infolog);
    }

    #if defined(VCG_PRINT_SHADERLOG)
      std::cout << "shader compiler: " << ShaderInfolog(shader) << std::endl;
    #endif

    return shader;
  }


  GLuint CGLShaderUtils::CompileSourceFile(GLenum type, const char* file) 
  {
    std::fstream stream(file);
    if (!stream.is_open() || stream.bad()) {
      std::stringstream ss;
      ss << "failed to open shader source file: " << file << "!" << std::endl;
      throw CGLShaderUtils::CFailure(ss.str());
    }

    //read content
    std::string	 source;
    std::string	 line;
    while(!stream.eof()) {

      std::getline(stream, line);
      source += line;
      source += '\n';

      if (stream.bad()) {
        std::stringstream ss;
        ss << "failed to read shader source file: " << file << "!" << std::endl;
        throw CGLShaderUtils::CFailure(ss.str());
      }
    }
    stream.close();

    //create shader from source
    return CompileSource(type, source.c_str());
  }



  GLuint CGLShaderUtils::LinkShaders(GLuint shader[], int num) 
  {
    
    GLuint program = glCreateProgram();

    //assign shaders
    for (int i=0; i < num; i++)
      glAttachShader(program, shader[i]);

    //link program
    glLinkProgram(program);

    //query status
    int query;
    glGetProgramiv(program, GL_LINK_STATUS, &query);
    if (query == GL_FALSE) {
      std::string infolog = ProgramInfolog(program);
      glDeleteProgram(program);
      throw CGLShaderUtils::CFailure(infolog);
    }

    #if defined(VCG_PRINT_SHADERLOG)
      std::cout << "shader linker: " << ProgramInfolog(program) << std::endl;
    #endif

    return program;
  }

  GLuint CGLShaderUtils::LinkShaders(GLuint vs, GLuint fs) 
  {

    GLuint shaders[] = {vs, fs};
    return LinkShaders(shaders, 2);
  }



  GLuint CGLShaderUtils::LinkShaderSources(const char* vertssrc, const char* ctrlssrc, const char* evalssrc, const char* geomssrc, const char* fragssrc) {
    std::vector<GLuint> shaders;
    shaders.reserve(5);

    try {

      if (vertssrc)
        shaders.push_back(
        CompileSource(GL_VERTEX_SHADER, vertssrc));
      if (ctrlssrc) 
        shaders.push_back(
        CompileSource(GL_TESS_CONTROL_SHADER, ctrlssrc));
      if (evalssrc)
        shaders.push_back(
        CompileSource(GL_TESS_EVALUATION_SHADER, evalssrc));
      if (geomssrc)
        shaders.push_back(
        CompileSource(GL_GEOMETRY_SHADER, geomssrc));
      if (fragssrc)
        shaders.push_back(
        CompileSource(GL_FRAGMENT_SHADER, fragssrc));


      GLuint program = LinkShaders(shaders.data(), static_cast<int>(shaders.size()));
      glSafeDeleteShaders(shaders.data(), shaders.size());
      return program;
    }
    catch(...) {
      glSafeDeleteShaders(shaders.data(), shaders.size());
      throw;
    }
  }

  GLuint CGLShaderUtils::LinkShaderFiles(const char* vertsfile, const char* ctrlsfile, const char* evalsfile, const char* geomsfile, const char* fragsfile) 
  {
    std::vector<GLuint> shaders;
    shaders.reserve(5);

    try {

      if (vertsfile) {
        shaders.push_back(CompileSourceFile(GL_VERTEX_SHADER, vertsfile));
      }
      if (ctrlsfile) {
        shaders.push_back(CompileSourceFile(GL_TESS_CONTROL_SHADER, ctrlsfile));
      }
      if (evalsfile) {
        shaders.push_back(CompileSourceFile(GL_TESS_EVALUATION_SHADER, evalsfile));
      }
      if (geomsfile) {
        shaders.push_back(CompileSourceFile(GL_GEOMETRY_SHADER, geomsfile));
      }
      if (fragsfile)  {
        shaders.push_back(CompileSourceFile(GL_FRAGMENT_SHADER, fragsfile));
      }

      GLuint program = LinkShaders(shaders.data(), static_cast<int>(shaders.size()));
      glSafeDeleteShaders(shaders.data(), shaders.size());
      return program;
    }
    catch(...) {
      glSafeDeleteShaders(shaders.data(), shaders.size());
      throw;
    }
  }


  std::string CGLShaderUtils::ShaderInfolog(GLuint shader) {

    int query;
    std::string infolog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &query);
    if (query > 0) {
      infolog.resize(query);
      glGetShaderInfoLog(shader, static_cast<GLsizei>(infolog.size()), &query, &infolog[0]);
    }
    return infolog;
  }

  std::string CGLShaderUtils::ProgramInfolog(GLuint program) {
    int query;
    std::string infolog;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &query);
    if (query > 0) {
      infolog.resize(query);
      glGetProgramInfoLog(program, static_cast<GLsizei>(infolog.size()), &query, &infolog[0]);
    }
    return infolog;
  }


} //namespace Utils