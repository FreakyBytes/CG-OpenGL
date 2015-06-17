#include "GLUtilsPrecompiled.h"
#include "Helper.h"


#include "GLUtilities.h"
#include <GL/glew.h>

namespace GLUtils {




//----------------------------------------------------------------------------


void CHelper::RenderTexturedFullScreenQuad(std::vector<GLuint> const & textures) 
{
  {
    glMatrixMode(GL_PROJECTION);
    CGLPushMatrix scopedProjectionMatrix;
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    CGLPushMatrix scopedModelViewMatrix;
    glLoadIdentity();
    glDisable(GL_LIGHTING);

    for (size_t i = 0; i < textures.size(); ++i) {
      glActiveTexture(static_cast<GLenum>(static_cast<size_t>(GL_TEXTURE0) + i));
      glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    CheckForError();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_2D);
    glBegin( GL_QUADS );
    glTexCoord2f(0.f,0.f); glVertex2f(0.f,0.f);
    glTexCoord2f(1.f,0.f); glVertex2f(1.f,0.f);
    glTexCoord2f(1.f,1.f); glVertex2f(1.f,1.f);
    glTexCoord2f(0.f,1.f); glVertex2f(0.f,1.f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
  }
}


void CHelper::RenderTexturedFullScreenQuad(GLuint texture) 
{
  std::vector<GLuint> textures;
  textures.push_back(texture);
  RenderTexturedFullScreenQuad(textures);
}



void CHelper::RenderTexturedFullScreenQuad(CGLTexture const & texture)
{
  RenderTexturedFullScreenQuad(texture.GetTextureID());
}


bool CHelper::CheckForError(void)
{
#ifdef CHECKOPENGLERROR
  GLenum errCode;
  std::string errString;

  if ((errCode = glGetError()) != GL_NO_ERROR) {
    errString = std::string((const char *)(gluErrorString(errCode)));
    //assert(false);
    std::cout << "OpenGL Error: " <<  errString << std::endl;
    return true;
  }
#endif
  return false;
}




}