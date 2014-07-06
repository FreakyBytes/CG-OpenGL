#include "OpenGLWindow.h"

#include <iostream>
#include "Helper.h"
#include "GLUtilities.h"
#include "RasterTerrainModel.h"
#include <string.h>

using namespace Gui;

namespace Application {

	COpenGLWindow::COpenGLWindow(Terrain::CTerrainModel & terrain)
		: CPrimaryView(terrain)
	{
		mWidth = 1024;
		mHeight = 768;
	}


	const GLfloat light_pos[] = { -400.0, -350.0, 1300.0, 0.0 };
	const GLfloat light_direction[] = { 0.0, 0.0, -10.0 };
	const GLfloat light_ambient[] = { 1.0, 1.0, 0.7, 0.1 };
	const GLfloat light_diffuse[] = { 0.812, 0.812, 0.333, 0.6 };
	const GLfloat light_specular[] = { 1.0, 1.0, 0.5, 0.3 };

	const GLfloat terrain_ambient[] = { 0.2, 0.2, 0.2, 0.5 };
	const GLfloat terrain_diffuse[] = { 0.55, 0.8, 0.55, 0.6 };
	const GLfloat terrain_specular[] = { 0.02, 0.02, 0.02, 0.01 }; // Terrain has nearly no specular "reflection"

	GLint active_camera = 0;
	GLboolean alternate_camera = false;

	//start up window
	bool COpenGLWindow::Initialize(int argc, char **argv)
	{
		//configure glut
		CPrimaryView::Initialize(argc, argv);

		//set clear color
		glm::vec3 const & clearColor = GetViewStates().GetClearColor();
		glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_NORMALIZE);	//normalize normals when scaling objects

		// apply settings for light 0
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		return GLUtils::CHelper::CheckForError();
	}


	//render loop - will be called every rendered frame
	void COpenGLWindow::Display(void)
	{
		//if window size has changed, recalculate projection matrix
		if (mProjectionMatrixDirty) {
			UpdateProjectionMatrix();
		}

		//get elapsed time since last frame
		double time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

		//clear screen
		glm::vec3 const & clearColor = GetViewStates().GetClearColor();
		glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		//clear model view matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//update scene
		UpdateScene(time);
		GLUtils::CHelper::CheckForError();

		//render scene
		RenderScene();
		GLUtils::CHelper::CheckForError();

		//show rendered content 
		ShowRenderedFrame();
	}



	void COpenGLWindow::UpdateScene(double time)
	{
		//compute time diff to the last update
		double timediff = time - GetViewStates().GetLastUpdate();

		//frame counter
		GetViewStates().IncrementFrame();

		//put out fps
		if (time - GetViewStates().GetFPSTimeBase() > 1.0) {
			char fpsString[100];
			sprintf_s(fpsString, "OpenGL Example - FPS: %4.2f - Triangles: %d", GetViewStates().GetFrame() / (time - GetViewStates().GetFPSTimeBase()), GetTerrain().GetNumberOfRenderedTriangles());
			GetViewStates().SetFPSTimeBase(time);
			GetViewStates().ResetFrame();

			GetViewStates().SetTitelString(fpsString);
		}

		//update camera
		GetFirstPersonCamera().FrameMove(static_cast<float>(timediff));

		//save time for next frame
		GetViewStates().SetLastUpdate(time);
	}



	void COpenGLWindow::RenderScene(void)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GetViewStates().GetWireframeMode() ? GL_LINE : GL_FILL);

		glm::dvec3 eye;
		glm::dvec3 lookAt;
		const glm::dvec3 upVec = dvec3(0.0f, 0.0f, 1.0f);

		//rotate eye around y-axis
		if (alternate_camera == false) {
			eye = GetFirstPersonCamera().GetEyePt();
			lookAt = GetFirstPersonCamera().GetLookAtPt();
		}
		else
		{
			switch (active_camera)
			{
			case 1:
				eye = dvec3(-3600.6f, 34.7f, 1038.9f);
				lookAt = dvec3(-3599.6f, 34.5f, 1038.7f);
				break;

			case 2:
				eye = dvec3(-3398.3, -352.0, 571.2);
				lookAt = dvec3(-3397.5, -352.6, 571.2);
				break;

			case 3:
				eye = dvec3(-949.5, 1261.4, 1681.6);
				lookAt = dvec3(-948.5, 1261.85, 1681.3);
				break;
			default:
				return;
			}
		}

		//setup camera
		gluLookAt(eye.x, eye.y, eye.z,          // eye position in world space
			lookAt.x, lookAt.y, lookAt.z, // point of interest in world space
			upVec.x, upVec.y, upVec.z);   // up vector

		//render sky dome
		RenderSkyDome();

		//render palme
		RenderPalm();

		//render terrain
		RenderTerrain(eye, lookAt, upVec);
	}



	void COpenGLWindow::RenderPalm(void)
	{
		//save current matrix
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		//disable culling and enable blending (required for leafs)
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		//transform palm
		glScalef(100.1f, 100.1f, 70.1f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(2.0f, 0.0f, 20.0f);

		//render the palm
		CPrimaryView::RenderPalm();

		glPopMatrix();
		glPushMatrix();

		//transform palm
		glScalef(80.1f, 80.1f, 60.1f);
		glRotatef(85.7f, 1.0f, 0.0f, 0.1f);
		glTranslatef(-0.6f, 0.0f, 20.0f);

		//render the palm
		CPrimaryView::RenderPalm();

		glPopMatrix();
		glPushMatrix();

		//transform palm
		glScalef(80.1f, 80.1f, 60.1f);
		glRotatef(90.7f, 1.0f, 0.0f, 0.0f);
		glTranslatef(-1.6f, -1.0f, 20.0f);

		//render the palm
		CPrimaryView::RenderPalm();

		//restore states
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		//restore previous matrix
		glPopMatrix();
	}

	void COpenGLWindow::RenderTerrain(glm::dvec3 const & eye, glm::dvec3 const & lookAt, glm::dvec3 const & upVec)
	{
		//save current matrix
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		//save current material (being none material)
		glPushAttrib(GL_LIGHTING_BIT);

		//set material definition here 
		glMaterialfv(GL_FRONT, GL_AMBIENT, terrain_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, terrain_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, terrain_specular);
		
		//render the palm
		CPrimaryView::RenderTerrain(eye, lookAt, upVec);

		//restore current material
		glPopAttrib();

		//restore previous matrix
		glPopMatrix();
	}



	//param key: key pressed (only standard keys)
	//param ctrl: ctrl pressed
	//param alt: alt pressed
	bool COpenGLWindow::EvaluateActionKey(unsigned char key, bool ctrl, bool alt)
	{
		const glm::dvec3 eye = GetFirstPersonCamera().GetEyePt();
		const glm::dvec3 lookAt = GetFirstPersonCamera().GetLookAtPt();
		if (alt) {
			switch (key) {
			case 'w':
				GetViewStates().ToggleWireframeMode();
				return true;
			case 'k':
				if (alternate_camera == true) {
					alternate_camera = false;
					std::cout << "deactivated alt camera!" << std::endl;
				}
				else {
					alternate_camera = true;
					active_camera = 1;
					std::cout << "activated alt camera!" << std::endl;
				}

				return true;
			case 'r':
				GetFirstPersonCamera().Reset();

				return true;

			case 'p':
				

				printf("eye: [%f, %f, %f]\n lookAt: [%f, %f, %f] \n", eye.x, eye.y, eye.z, lookAt.x, lookAt.y, lookAt.z);
				return true;
			default:
				return false;
			}
		}
		else {
			switch (key)
			{
			case '1':
				active_camera = 1;
				std::cout << "set to alt camera 1" << std::endl;
				return true;
			case '2':
				active_camera = 2;
				std::cout << "set to alt camera 2" << std::endl;
				return true;
			case '3':
				active_camera = 3;
				std::cout << "set to alt camera 3" << std::endl;
				return true;
			default:
				return false;;
			}
		}
	}
}