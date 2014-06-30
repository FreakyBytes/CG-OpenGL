#pragma once

#include <vector>

namespace Terrain {
  class CErrorMetric;
}

/************************************************************************/
/* methods                                                              */
/************************************************************************/


//start openGL
static void SetupGLStats(void);

//use render targets

//initialize the application -> load/create all required resources
static bool InitializeApplication();

//release all allocated resources
static void FinalizeApplication(void);

//move light
static void MoveLightPosition(bool forward);

// render loop (general)
static void UpdateScene(double time);

//render loop terrain
static void RenderScene();


/************************************************************************/
/* glut callbacks                                                       */
/************************************************************************/

// render loop
static void Display();

// size of window changed
static void Reshape(int w, int h);

/************************************************************************/
/* keyboard and mouse callbacks                                         */
/************************************************************************/

//static CameraKeys MapKey(int key) ;

//static CameraKeys MapKey(unsigned char key);

static bool EvaluateSpecialKey(unsigned char key);

static bool EvaluateSpecialKey(int key);

static void KeyPressed(unsigned char key, int x, int y);

static void KeyReleased(unsigned char key, int x, int y);

static void KeyPressed(int key, int x, int y);

static void KeyReleased(int key, int x, int y);

static void MouseKey(int button, int state, int x, int y);

static void MouseMove(int x, int y);
