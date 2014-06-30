#pragma once

#include "GLUtilsDefines.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>

using namespace glm;

static dvec3 MakeVec3(dvec4 const & vector) {return dvec3(vector.x, vector.y, vector.z);}

namespace GLUtils {
  typedef unsigned char BYTE;

  
  //--------------------------------------------------------------------------------------
  // used by CCamera to map WM_KEYDOWN keys
  //--------------------------------------------------------------------------------------
  enum CameraKeys
  {
      CAM_STRAFE_LEFT = 0,
      CAM_STRAFE_RIGHT,
      CAM_MOVE_FORWARD,
      CAM_MOVE_BACKWARD,
      CAM_MOVE_UP,
      CAM_MOVE_DOWN,
      CAM_RESET,
      CAM_CONTROLDOWN,
      CAM_ACCELERATE,
      CAM_THROTTLE,
      CAM_MAX_KEYS,
      CAM_UNKNOWN     = 0xFF
  };

  enum CameraMouseKeys
  {
    MouseLeft = 0,
    MouseMiddle,
    MouseRight,
    MouseUnknown
  };

  #define KEY_WAS_DOWN_MASK 0x80
  #define KEY_IS_DOWN_MASK  0x01
  #define MOUSE_LEFT_BUTTON   0x01
  #define MOUSE_MIDDLE_BUTTON 0x02
  #define MOUSE_RIGHT_BUTTON  0x04
  #define MOUSE_WHEEL         0x08


  //--------------------------------------------------------------------------------------
  // Simple base camera class that moves and rotates.  The base class
  //       records mouse and keyboard input for use by a derived class, and 
  //       keeps common state.
  //--------------------------------------------------------------------------------------
  class CBaseCamera
  {
  public:
      GLUTILS_API CBaseCamera();
      GLUTILS_API virtual ~CBaseCamera(){}

      // Call these from client and use Get*Matrix() to read new matrices
      //UTILS_API virtual LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
      GLUTILS_API virtual void HandleKeys(CameraKeys key, bool keyDown);
      GLUTILS_API virtual void HandleMouse(CameraMouseKeys key, bool keyDown, int x, int y);
      GLUTILS_API virtual void FrameMove( double fElapsedTime ) = 0;

      // Functions to change camera matrices
      GLUTILS_API virtual void Reset();
      GLUTILS_API virtual void SetViewParams( dvec3 const & pvEyePt, dvec3 const & pvLookatPt );
      GLUTILS_API virtual void SetProjParams( double fFOV, double fAspect, double fNearPlane, double fFarPlane );

      // Functions to change behavior
      //GLUTILS_API virtual void SetDragRect( RECT & rc ) {m_rcDrag = rc;}
      GLUTILS_API void SetInvertPitch( bool bInvertPitch ) {mInvertPitch = bInvertPitch;}
      GLUTILS_API void SetDrag( bool bMovementDrag, double fTotalDragTimeToZero = 0.25 ) {mMovementDrag = bMovementDrag; mTotalDragTimeToZero = fTotalDragTimeToZero;}
      GLUTILS_API void SetEnableYAxisMovement( bool bEnableYAxisMovement ) {mEnableYAxisMovement = bEnableYAxisMovement;}
      GLUTILS_API void etEnablePositionMovement( bool bEnablePositionMovement ) {mEnablePositionMovement = bEnablePositionMovement;}
      GLUTILS_API void SetClipToBoundary( bool bClipToBoundary, dvec3 * pvMinBoundary, dvec3 * pvMaxBoundary ) {mClipToBoundary = bClipToBoundary; if( pvMinBoundary ) mMinBoundary = *pvMinBoundary; if( pvMaxBoundary ) mMaxBoundary = *pvMaxBoundary;}
      GLUTILS_API void SetScalers( double fRotationScaler = 0.01, double fKeyboardMoveScaler = 5.0, double fMouseMoveScaler = 5.0) {mRotationScaler = fRotationScaler; mKeyboardMoveScaler = fKeyboardMoveScaler; mMouseMoveScaler = fMouseMoveScaler;}
      GLUTILS_API void SetNumberOfFramesToSmoothMouseData( int nFrames ) {if( nFrames > 0 ) mFramesToSmoothMouseData = static_cast<double>(nFrames);}
      GLUTILS_API void SetResetCursorAfterMove( bool bResetCursorAfterMove ) {mResetCursorAfterMove = bResetCursorAfterMove;}

      // Functions to get state
      GLUTILS_API dmat4 const & GetViewMatrix() const {return mViewMatrix;}
      GLUTILS_API dmat4 const & GetProjMatrix() const {return mProjectionMatrix;}
      GLUTILS_API virtual dvec3 GetEyePt() const {return (mZAxisUp) ? dvec3(mEye.x, -mEye.z, mEye.y) : mEye;}
      GLUTILS_API dvec3 GetLookAtPt() const {return (mZAxisUp) ? dvec3(mLookAt.x, -mLookAt.z, mLookAt.y) : mLookAt;}
      GLUTILS_API double GetNearClip() const {return mNearPlane;}
      GLUTILS_API double GetFarClip() const {return mFarPlane;}

      GLUTILS_API bool IsBeingDragged() const {return ( mMouseLButtonDown || mMouseMButtonDown || mMouseRButtonDown );}
      GLUTILS_API bool IsMouseLButtonDown() const {return mMouseLButtonDown;}
      GLUTILS_API bool IsMouseMButtonDown() const {return mMouseMButtonDown;}
      GLUTILS_API bool IsMouseRButtonDown() const {return mMouseRButtonDown;}

      GLUTILS_API void SetZAxisUp(bool zAxisUp) {mZAxisUp = zAxisUp;}

      GLUTILS_API void SetMouseToRotate(void) {mMouseRotates = true;}
      GLUTILS_API void SetMouseToMove(void) {mMouseRotates = false;}

  protected:
      // Functions to map a WM_KEYDOWN key to a D3DUtil_CameraKeys enum
      virtual CameraKeys  MapKey( unsigned int nKey );
      bool IsKeyDown( BYTE key ) const {return( ( key & KEY_IS_DOWN_MASK ) == KEY_IS_DOWN_MASK );}
      bool WasKeyDown( BYTE key ) const {return( ( key & KEY_WAS_DOWN_MASK ) == KEY_WAS_DOWN_MASK );}

      void ConstrainToBoundary( dvec3 * pV );
      void UpdateMouseDelta();
      void UpdateVelocity( double fElapsedTime );
      void GetInput( bool bGetKeyboardInput, bool bGetMouseInput, bool bGetGamepadInput, bool bResetCursorAfterMove );

      dmat4 mViewMatrix;                                 // View matrix 
      dmat4 mProjectionMatrix;                           // Projection matrix

      int mKeysDown;                                    // Number of camera keys that are down.
      BYTE mKeys[CAM_MAX_KEYS];                         // State of input - KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK
      dvec3 mKeyboardDirection;                          // Direction vector of keyboard input
      ivec2 mLastMousePosition;                         // Last absolute position of mouse cursor
      ivec2 mCurrentMousePosition;                      // Last absolute position of mouse cursor
      bool mMouseLButtonDown;                           // True if left button is down 
      bool mMouseMButtonDown;                           // True if middle button is down 
      bool mMouseRButtonDown;                           // True if right button is down 
      int mCurrentButtonMask;                           // mask of which buttons are down
      int mMouseWheelDelta;                             // Amount of middle wheel scroll (+/-) 
      dvec2 mMouseDelta;                                 // Mouse relative delta smoothed over a few frames
      double mFramesToSmoothMouseData;                   // Number of frames to smooth mouse data over

      dvec3 mDefaultEye;                                 // Default camera eye position
      dvec3 mDefaultLookAt;                              // Default LookAt position
      dvec3 mEye;                                        // Camera eye position
      dvec3 mLookAt;                                     // LookAt position
      double mCameraYawAngle;                            // Yaw angle of camera
      double mCameraPitchAngle;                          // Pitch angle of camera

      //RECT m_rcDrag;                                  // Rectangle within which a drag can be initiated.
      dvec3 mVelocity;                                   // Velocity of camera
      bool mMovementDrag;                               // If true, then camera movement will slow to a stop otherwise movement is instant
      dvec3 mVelocityDrag;                               // Velocity drag force
      double mDragTimer;                                 // Countdown timer to apply drag
      double mTotalDragTimeToZero;                       // Time it takes for velocity to go from full to 0
      dvec2 mRotVelocity;                                // Velocity of camera

      bool mMouseRotates;                               //If true, the mouse rotates the camera, else the mouse moves the camera

      double mFOV;                                       // Field of view
      double mAspect;                                    // Aspect ratio
      double mNearPlane;                                 // Near plane
      double mFarPlane;                                  // Far plane

      double mRotationScaler;                            // Scaler for rotation
      double mKeyboardMoveScaler;                        // Scaler for keyboard movement
      double mMouseMoveScaler;                           // Scaler for Mouse movement

      bool mInvertPitch;                                // Invert the pitch axis
      bool mEnablePositionMovement;                     // If true, then the user can translate the camera/model 
      bool mEnableYAxisMovement;                        // If true, then camera can move in the y-axis

      bool mClipToBoundary;                             // If true, then the camera will be clipped to the boundary
      dvec3 mMinBoundary;                                // Min point in clip boundary
      dvec3 mMaxBoundary;                                // Max point in clip boundary

      bool mResetCursorAfterMove;                       // If true, the class will reset the cursor position so that the cursor always has space to move 

      bool mZAxisUp;                                    //means, in the world coordinate system, the z-axis up (instead of the y-axis)
  };


  //--------------------------------------------------------------------------------------
  // Simple first person camera class that moves and rotates.
  //       It allows yaw and pitch but not roll.  It uses WM_KEYDOWN and 
  //       GetCursorPos() to respond to keyboard and mouse input and updates the 
  //       view matrix based on input.  
  //--------------------------------------------------------------------------------------
  class CFirstPersonCamera : public CBaseCamera
  {
  public:
      GLUTILS_API CFirstPersonCamera();
      GLUTILS_API virtual ~CFirstPersonCamera() {}

      // Call these from client and use Get*Matrix() to read new matrices
      GLUTILS_API virtual void FrameMove( double fElapsedTime ) override;

      // Functions to change behavior
      GLUTILS_API void SetMouseButtons( bool bLeft, bool bMiddle, bool bRight, bool bRotateWithoutButtonDown = false );

      // Functions to get state
      GLUTILS_API dmat4 const & GetWorldMatrix() {return mCameraWorld;}
      GLUTILS_API dvec3 GetWorldRight() const;
      GLUTILS_API dvec3 GetWorldUp() const;
      GLUTILS_API dvec3 GetWorldAhead() const;
      GLUTILS_API virtual dvec3 GetEyePt() const override;

  protected:
      dmat4 mCameraWorld;                    // World matrix of the camera (inverse of the view matrix)

      int mActiveButtonMask;                // Mask to determine which button to enable for rotation
      bool mRotateWithoutButtonDown;
  };


 

} //namespace Utils