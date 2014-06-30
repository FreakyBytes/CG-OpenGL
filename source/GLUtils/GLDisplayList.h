#pragma once

#include "GLUtilsDefines.h"


namespace GLUtils {

  class CGLDisplayList {
    public:
      GLUTILS_API CGLDisplayList(void);
      GLUTILS_API ~CGLDisplayList(void);

      GLUTILS_API bool IsValid(void);     //returns true, if this display list could be registered to the context
      GLUTILS_API void BeginDefine(void); //start the defining code
      GLUTILS_API void EndDefine(void);   //end defining code;
      GLUTILS_API void Render(void);      //renders this display list

    private:
      GLuint mID;
  };
}