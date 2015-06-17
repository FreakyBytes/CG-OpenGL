#include "GLUtilsPrecompiled.h"
#include "GLShaderCollection.h"
#include "GLShaderUtils.h"
#include <regex>

#include "Helper.h"


namespace GLUtils {


static const std::regex SECTION_PARSE_PATTERN("\\s*#shader\\s*\\(\\s*(vs|cs|es|gs|fs)\\s*,\\s*([\\w|_|-]+)\\s*\\)\\s*", std::regex_constants::icase);
static const std::regex SECTION_MATCH_PATTERN("\\s*#shader.*", std::regex_constants::icase);

struct GLShaderSectionTokenizer {
  GLenum			mShaderType;
  std::string		mShaderName;

  bool matches(const std::string& line) {

    if (std::regex_match(line, SECTION_MATCH_PATTERN)) {

      std::smatch result;
      if (std::regex_match(line, result, SECTION_PARSE_PATTERN)) {
        
        mShaderType = GetShaderTypeFromString(result[1]);
        mShaderName = result[2];
        return true;
      }
      else {
        std::cerr << result.str() << std::endl;
        throw CFailure("Invalid shader section command!");
      }
    }
    return false;
  }

  static GLenum GetShaderTypeFromString(const std::string& name) {
    if (_stricmp(name.c_str(), "vs") == 0) return GL_VERTEX_SHADER;
    if (_stricmp(name.c_str(), "cs") == 0) return GL_TESS_CONTROL_SHADER;
    if (_stricmp(name.c_str(), "es") == 0) return GL_TESS_EVALUATION_SHADER;
    if (_stricmp(name.c_str(), "gs") == 0) return GL_GEOMETRY_SHADER;
    if (_stricmp(name.c_str(), "fs") == 0) return GL_FRAGMENT_SHADER;

    throw std::logic_error("parsing error: invalid shader type!");
  }
};


//---------------------------------------------------------------------//

CGLShaderCollection::CGLShaderCollection() {
}



CGLShaderCollection::~CGLShaderCollection() {
  Clear();
}


GLuint CGLShaderCollection::GetShader(const std::string& shaderName) const {
  auto itr = mContainer.find(shaderName);
  if (itr != mContainer.end())
    return (*itr).second;
  else
    return 0;
}



void CGLShaderCollection::CreateFromSourceStream(std::istream& stream, const GLShaderMacro macros[], size_t macroCount) {

  GLShaderSectionTokenizer tokenizer;
  std::string shaderSources[2];
  std::stringstream sourceStream;
  size_t lineIndex = 0;

  //assemble common shader source
  sourceStream << "#version 420 compatibility" << std::endl;
  if (macros != nullptr) {
    for (size_t i=0; i < macroCount; ++i) {
      sourceStream << "#define " << macros[i].first << " " << macros[i].second << std::endl;
    }
  }

  //read common part from stream
  std::string line;
  while(std::getline(stream, line) && !tokenizer.matches(line)) {
    sourceStream << line << std::endl;	
    ++lineIndex;
  }

  //end of stream or failure?
  if (!stream)
    throw CFailure("Failed to read shader sources!");

  //common source
  shaderSources[0] = sourceStream.str();

  // read and compile shaders
  std::string shaderName;
  GLenum shaderType;
  while (stream) {

    //next line
    ++lineIndex; 

    //store current tokenizer state
    shaderName = tokenizer.mShaderName;
    shaderType = tokenizer.mShaderType;
    

    //check for duplicate entries
    auto itr = mContainer.find(tokenizer.mShaderName);
    if (itr != mContainer.end()) {
      std::stringstream err;
      err << "(" << lineIndex << "):duplicated shader name detected" << lineIndex << "): " << shaderName << std::endl;
      throw CFailure(err.str());
    }

    //read shader source
    sourceStream.str("");
    while(std::getline(stream, line) && !tokenizer.matches(line)) {
      sourceStream << line << std::endl;	
      ++lineIndex;
    }

    //compile and add to container
    shaderSources[1] = sourceStream.str();
    try {
      mContainer[shaderName] = CGLShaderUtils::CompileSources(shaderType, shaderSources);
    }
    catch(std::exception& ex) {
      std::stringstream err;
      err << "failed to compile shader: " << shaderName << std::endl;
      err << "shader compiler log: " << std::endl << ex.what() << std::endl;
      throw CFailure(err.str());
    }
  }
}



void CGLShaderCollection::CreateFromSourceString(const std::string& source, const GLShaderMacro macros[], size_t macroCount) {
  
  std::istringstream stream(source);
  CreateFromSourceStream(stream, macros, macroCount);
}



void CGLShaderCollection::CreateFromSourceFile(const std::string& filename, const GLShaderMacro macros[], size_t macroCount) {

  std::ifstream stream(filename);
  if (stream.is_open())
    CreateFromSourceStream(stream, macros, macroCount);
  else
    throw CFailure("failed to open shader collection file!");
}


bool CGLShaderCollection::LinkShaders(CGLShaderProgram& program) {

  std::vector<GLuint> shaders;
  shaders.reserve(mContainer.size());

  std::for_each(mContainer.begin(), mContainer.end(), [&shaders](const ContainerType::value_type& value) {
    shaders.push_back(value.second);
  });

  try {
    program.Link(shaders.data(), static_cast<GLsizei>(shaders.size()));
  }
  catch (CFailure & ex)
  {
    throw ex;
  }
  catch (...) 
  {
    return false;
  }

  return true;
}



void CGLShaderCollection::Clear() {

  std::for_each(mContainer.begin(), mContainer.end(), [](ContainerType::value_type& entry) {
    glSafeDeleteShader(entry.second);
  });
  CHelper::CheckForError();
  mContainer.clear();
}



bool CGLShaderCollection::LinkShaders(CGLShaderProgram& program,
    const std::string& vertexShader, 
    const std::string& controlShader, 
    const std::string& evaluationShader, 
    const std::string& geometryShader, 
    const std::string& fragmentShader) {

  std::vector<GLuint> shaders;
  shaders.reserve(5);

  if (vertexShader.empty())
    throw std::logic_error("Vertex shader is mandatory");

  if (!vertexShader.empty())
    shaders.push_back(GetShader(vertexShader));
  if (!vertexShader.empty())
    shaders.push_back(GetShader(controlShader));
  if (!vertexShader.empty())
    shaders.push_back(GetShader(evaluationShader));
  if (!vertexShader.empty())
    shaders.push_back(GetShader(geometryShader));
  if (!vertexShader.empty())
    shaders.push_back(GetShader(fragmentShader));

  program.Link(shaders.data(), static_cast<GLsizei>(shaders.size()));

  return true;

}


} //namespace Utils