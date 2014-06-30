#include "TerrainPrecompiled.h"
#include "TinyViewer.h"

#include "ErrorMetric.h"
#include <cstdlib>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <memory>
#include <algorithm> 


#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/constants.hpp>
#include <iostream>
#include <string>

#include "ViewFrustum.h"
#include "GLCamera.h"

#include "ChunkedTerrainModel.h"
#include "RasterTerrainModel.h"
#include "TerrainModel.h"
#define USESHADER

/************************************************************************/
/* statics                                                              */
/************************************************************************/

static const char *     sWindowTitle        = "Tiny Viewer";
static const double     sFieldOfView        = 45.00;
static const float      sNearPlane          = 1.0f; //0.01
static const float      sFarPlane           = 10000.0f;//2000.0f; //100

static const glm::dvec3 sEyePostion          = glm::dvec3(0.0, 1.0, 255.0);
static const glm::dvec3 sLookAt              = glm::dvec3(0.0, 0.0, 1.0);

static int sScreenWidth                     = 1280;
static int sScreenHeight                    = 720;

static double sLastUpdate                   = 0.0;  // time of the last update call
static int sFrame                           = 0;    //frame counter
static double sFPSTimeBase                  = 0.0;  //part of the fps counter

//switches
static bool sWireframe                      = false;
static bool sShowBounds                     = false;
static bool sDoUpdate                       = true;
static bool sFirstFrustum                   = true;
static bool sUseShader                      = true;
static bool sShowTrajectory                 = true;
static double sTolerance                    = 2.0;

static float sLightAngle = 0.25f * glm::pi<float>();

static glm::vec3 sBoundsMin;
static glm::vec3 sBoundsMax;

//static Terrain::CChunkedTerrainModel sTerrainModel;
static Terrain::CRasterTerrainModel sTerrainModel;
static GLUtils::CViewFrustum sFrustum;

static GLUtils::CFirstPersonCamera sGLCamera;
static const std::string sTerrainModelPath("../../../media/example.rlod");


/************************************************************************/
/* Initialize                                                           */
/************************************************************************/


//setup the default opengl states
void SetupGLStats() {

  glClearColor(0.3f, 0.3f, 0.3f, 1.f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_NORMALIZE);	//normalize normals when scaling objects

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  
  MoveLightPosition(true);
}



//initialize the application -> load/create all required resources
bool InitializeApplication() 
{

  std::cout << "loading terrain data-set..." << std::endl;
  if (!sTerrainModel.Init(sTerrainModelPath.c_str())) {
    return false;
  }

  sTerrainModel.GetBoundings(sBoundsMin, sBoundsMax);
  SetupGLStats();

  sGLCamera.SetZAxisUp(true);
  sGLCamera.SetViewParams(sEyePostion, sLookAt);

  return true;
}



//release all allocated resources
void FinalizeApplication(void) 
{
}



//move light
void MoveLightPosition(bool forward)
{
  if (sLightAngle > 2.0f*pi<float>() || sLightAngle < -2.0f*pi<float>()) {
    sLightAngle = 0.0f;
  }

  sLightAngle = (forward ? 1.0f : -1.0f) * 0.1f + sLightAngle;
  vec3 circle = vec3(sin(sLightAngle), 0.0f, cos(sLightAngle));
  circle = normalize(rotate(circle, 0.25f * pi<float>(), vec3(0.0,0.0,1.0)));

  vec4 lightPosition = vec4(circle.x, circle.y, circle.z, 0.0f);
  glLightfv(GL_LIGHT0, GL_POSITION, &lightPosition.x);
}



/************************************************************************/
/* scene updating and rendering                                         */
/************************************************************************/

void UpdateScene(double time) {

  //compute time diff to the last update
  double timediff = time - sLastUpdate;

  //frame counter
  ++sFrame;
    
  if (time - sFPSTimeBase > 1.0) {
    char fpsString[50];
    sprintf_s(fpsString,"Tiny Viewer - FPS: %4.2f", sFrame/(time-sFPSTimeBase));
    sFPSTimeBase = time;
    sFrame = 0;
    glutSetWindowTitle(fpsString);
  }

  sGLCamera.FrameMove(static_cast<float>(timediff));

  //save time for next frame
  sLastUpdate = time;
}



void RenderTerrainWrapper(void)
{
  
}



void RenderTerrain(Terrain::CErrorMetric const & metric) 
{
  if (sDoUpdate) {
    sTerrainModel.Update(metric, sFrustum);
  }

  //draw frustum for debugging purpose
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  if (!sFirstFrustum) {
    sFrustum.DrawPlanes();
  }

  glEnable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);
  

  sTerrainModel.Render();

  if (sShowBounds) {
    sTerrainModel.RenderBounds();
  }
}



void RenderScene() 
{

  glPolygonMode(GL_FRONT_AND_BACK, sWireframe ? GL_LINE : GL_FILL);

  //rotate eye around y-axis

  const glm::dvec3 eye = sGLCamera.GetEyePt();
  const glm::dvec3 lookAt = sGLCamera.GetLookAtPt();
  const glm::dvec3 upVec = dvec3(0.0f, 0.0f, 1.0f);

  //setup camera
  gluLookAt(eye.x, eye.y, eye.z,          ///< eye position in world space
            lookAt.x, lookAt.y, lookAt.z, ///< point of interest in world space
            upVec.x, upVec.y, upVec.z);   ///< up vector

  if (sFirstFrustum) {
    sFrustum.SetCamDef(eye,lookAt,upVec);
  }
 
  //z is up vector here
  glPushMatrix();
  Terrain::CErrorMetric emetric;
  emetric.SetViewPosition(eye);
  emetric.SetViewparams(static_cast<float>(glm::radians(sFieldOfView)), static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT)), static_cast<float>(sTolerance));

  //display terrain
  RenderTerrain(emetric);
  glPopMatrix();

}



/************************************************************************/
/* glut callbacks														*/
/************************************************************************/

void Display() {

  double time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //update scene
  UpdateScene(time);

  //render scene
  RenderScene();

  //show rendered content
  glutSwapBuffers();
  //force a redisplay for continues animation
  glutPostRedisplay();
}


void Reshape(int w, int h) {

  sScreenWidth = w;
  sScreenHeight = h;
  glViewport(0, 0, w, h);

  double aratio = static_cast<double>(w)/static_cast<double>(h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(sFieldOfView, aratio, sNearPlane, sFarPlane);
  sFrustum.SetCamInternals(static_cast<float>(sFieldOfView),static_cast<float>(aratio), static_cast<float>(sNearPlane),static_cast<float>(sFarPlane));
}



GLUtils::CameraKeys MapKey(int key) 
{
  switch( key )
  {
    if (GLUT_ACTIVE_ALT != glutGetModifiers()) {
        //case VK_CONTROL:
        //    return CAM_CONTROLDOWN;
        case GLUT_KEY_LEFT:
            return GLUtils::CAM_STRAFE_LEFT;
        case GLUT_KEY_RIGHT:
            return GLUtils::CAM_STRAFE_RIGHT;
        case GLUT_KEY_UP:
            return GLUtils::CAM_MOVE_FORWARD;
        case GLUT_KEY_DOWN:
            return GLUtils::CAM_MOVE_BACKWARD;
        case GLUT_KEY_PAGE_DOWN:
            return GLUtils::CAM_MOVE_UP;        // pgup
        case GLUT_KEY_PAGE_UP:
            return GLUtils::CAM_MOVE_DOWN;      // pgdn
        case GLUT_KEY_HOME:
            return GLUtils::CAM_RESET;
        default:
          return GLUtils::CAM_UNKNOWN;
    } 
  }

  return GLUtils::CAM_UNKNOWN;
}



GLUtils::CameraKeys MapKey(unsigned char key) 
{
  switch( key )
  {
    if (GLUT_ACTIVE_ALT != glutGetModifiers()) {
        case 'a':
            return GLUtils::CAM_STRAFE_LEFT;
        case 'd':
            return GLUtils::CAM_STRAFE_RIGHT;
        case 'w':
            return GLUtils::CAM_MOVE_FORWARD;
        case 's':
            return GLUtils::CAM_MOVE_BACKWARD;
        case 'q':
            return GLUtils::CAM_MOVE_DOWN;
        case 'e':
            return GLUtils::CAM_MOVE_UP;
        case '+':
          return GLUtils::CAM_ACCELERATE;
        case '-':
          return GLUtils::CAM_THROTTLE;
        default:
          return GLUtils::CAM_UNKNOWN;
    }
  }

  return GLUtils::CAM_UNKNOWN;
}



bool EvaluateSpecialKey(unsigned char key) 
{
  if (GLUT_ACTIVE_ALT == glutGetModifiers()) {
    switch(key) {
      case 'w':
        sWireframe = !sWireframe;
        return true;
      case 'b':
        sShowBounds = !sShowBounds;
        return true;
      case 'u':
        sDoUpdate = !sDoUpdate;
        if (sDoUpdate)
          std::cout << "update is enabled!" << std::endl;
        else
          std::cout << "update is disabled!" << std::endl;
        return true;

      case '+':
        sTolerance += 0.1;
        std::cout << "error tolerance is set to: " << sTolerance << std::endl;
        return true;
      case '-':
        sTolerance = glm::max(0.5, sTolerance-0.1);
        std::cout << "error tolerance is set to: " << sTolerance << std::endl;
        return true;
      case 't':
        sShowTrajectory = !sShowTrajectory;
      case 'c':
        sFrustum.ToggleFrustumCulling();
        return true;

      case 'f':
        sFirstFrustum = !sFirstFrustum;
        return true;
    }
  }

  return false;
}



bool EvaluateSpecialKey(int key) 
{
  switch(key) {
    case GLUT_KEY_PAGE_DOWN:
      MoveLightPosition(false);
      return true;
    case GLUT_KEY_PAGE_UP:
      MoveLightPosition(true);
      return true;
    default: 
      return false;
  }
  return false;
}


void KeyPressed(unsigned char key, int x, int y)
{
  if (!EvaluateSpecialKey(key)) {
    GLUtils::CameraKeys keyPressed = MapKey(key);
    sGLCamera.HandleKeys(keyPressed, true);
  }
}



void KeyReleased(unsigned char key, int x, int y)
{
  GLUtils::CameraKeys keyReleased = MapKey(key);
  sGLCamera.HandleKeys(keyReleased, false);
}


void KeyPressed(int key, int x, int y)
{
  if (!EvaluateSpecialKey(key)) {
    GLUtils::CameraKeys keyPressed = MapKey(key);
    sGLCamera.HandleKeys(keyPressed, true);
  }
}



void KeyReleased(int key, int x, int y)
{
  GLUtils::CameraKeys keyReleased = MapKey(key);
  sGLCamera.HandleKeys(keyReleased, false);
}



void MouseKey(int button, int state, int x, int y)
{
  GLUtils::CameraMouseKeys key = GLUtils::MouseUnknown;
  switch (button) {
  case GLUT_LEFT_BUTTON:
    key = GLUtils::MouseLeft;
    break;
  case GLUT_RIGHT_BUTTON:
    key = GLUtils::MouseRight;
    break;
  case GLUT_MIDDLE_BUTTON:
    key = GLUtils::MouseMiddle;
    break;
  default: 
    key = GLUtils::MouseUnknown;
  }

  sGLCamera.HandleMouse(key, state == GLUT_DOWN, x, y);
}



void MouseMove(int x, int y)
{
  sGLCamera.HandleMouse(GLUtils::MouseUnknown, false, x, y);
}


#include <regex>

/************************************************************************/
/* application main                                                     */
/************************************************************************/

int main(int argc, char* argv[]) 
{
  //init glut
  glutInit(&argc, argv);
  glutInitContextVersion(2, 0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH| GLUT_DOUBLE);

  //create window
  glutInitWindowSize(sScreenWidth, sScreenHeight);
  glutCreateWindow(sWindowTitle);

  glutDisplayFunc(Display);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(KeyPressed);
  glutKeyboardUpFunc(KeyReleased);
  glutSpecialFunc(KeyPressed);
  glutSpecialUpFunc(KeyReleased);
  glutMouseFunc(MouseKey);
  glutMotionFunc(MouseMove);
  glutCloseFunc(FinalizeApplication);	// on window close -> term app

  //init glew
  GLenum glew_err = glewInit();
  if (glew_err != GLEW_NO_ERROR) {
    std::cerr << "failed to initialize opengl extension wrapper: " << (const char*)glewGetErrorString(glew_err) << std::endl;
    return EXIT_FAILURE;
  }

  //init the application
  if (!InitializeApplication()) {
    std::cerr << "failed to initialize application!" << std::endl;
    return EXIT_FAILURE;
  }


  //enter mainloop
  glutMainLoop();
  return EXIT_SUCCESS;
}
