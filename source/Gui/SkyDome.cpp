#include "GuiPrecompiled.h"
#include "SkyDome.h"

#include "GLTexture.h"
#include "GLUtilities.h"
#include "Helper.h"


namespace Gui {

  static const float	TERRAIN_SCALE = 10000.f;


  void CSkyDome::Render(void) {
    if (mDisplayList.get() == nullptr) {
      mDisplayList.reset(new GLUtils::CGLDisplayList());

      if (mDisplayList->IsValid()) {
        mDisplayList->BeginDefine();

        const int numSlices = 50;
        const int numSegments = 50;

        //compute parametric sphere
        for (int j = 0; j < numSlices; ++j) {
          float v0 = j / static_cast<float>(numSlices);
          float v1 = (j + 1) / static_cast<float>(numSlices);
          glBegin(GL_TRIANGLE_STRIP);
          for (int i = 0; i < numSegments; ++i) {
            float u = i / static_cast<float>(numSegments - 1);

            glm::vec3 p1 = EvalParametricSphere(1.f, u, v0);
            glm::vec3 p2 = EvalParametricSphere(1.f, u, v1);

            glTexCoord2f(u, v0);
            glVertex3f(p1.x, -p1.z, p1.y);
            glTexCoord2f(u, v1);
            glVertex3f(p2.x, -p2.z, p2.y);
          }
          glEnd();
        }
        mDisplayList->EndDefine();

        //create texture
        mTexture.reset(new GLUtils::CGLTexture());
        mTexture->LoadFromFile(mTexturePath);
      }
    }
    else {

      //disable writing to z-buffer
      glDepthMask(false);
      //disable lighting
      glDisable(GL_LIGHTING);

      //scale dome over terrain
      GLUtils::CGLPushMatrix scopedMatrix(GL_MODELVIEW);
      float scale = glm::sqrt(TERRAIN_SCALE*TERRAIN_SCALE);
      glScalef(scale, scale, scale);

      //setup texture
      glActiveTexture(GL_TEXTURE0);
      glEnable(GL_TEXTURE_2D);
      mTexture->BindTexture();
      GLUtils::CHelper::CheckForError();
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	//only use texture color
      mDisplayList->Render();
      mTexture->UnbindTexture();
      glDisable(GL_TEXTURE_2D);

      //restore states
      glDepthMask(true);
      glEnable(GL_LIGHTING);
      GLUtils::CHelper::CheckForError();
    }
  }



  glm::vec3 CSkyDome::EvalParametricSphere(float r, float u, float v) 
  {
    float phi = glm::radians(u*360.f);
    float theta = glm::radians(v*180.f);

    return glm::vec3(
      r*glm::sin(theta)*glm::cos(phi),
      r*glm::cos(theta),
      r*glm::sin(theta)*glm::sin(phi));

  }
}