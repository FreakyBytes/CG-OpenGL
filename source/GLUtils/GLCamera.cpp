#include "GLUtilsPrecompiled.h"
#include "GLCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/constants.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_query.hpp>

static const float sKeyboardMovementScale = 1240.0f;
static const float sMouseMovementScale = 400.0f;

static void MakeIdentity(glm::quat & quat) 
{
  quat = glm::quat(0.0,0.0,0.0,1.0);
}


static void MakeIdentity(glm::mat4 & matrix) 
{
  matrix = glm::mat4(1.0);
}



static dvec3 Vec3TransformCoord(dvec3 const & vector, dmat4 const & matrix)
{
  const double x = vector.x * column(matrix, 0).x + vector.y * column(matrix, 1).x + vector.z * column(matrix, 2).x + column(matrix, 3).x;
  const double y = vector.x * column(matrix, 0).y + vector.y * column(matrix, 1).y + vector.z * column(matrix, 2).y + column(matrix, 3).y;
  const double z = vector.x * column(matrix, 0).z + vector.y * column(matrix, 1).z + vector.z * column(matrix, 2).z + column(matrix, 3).z;
  const double w = vector.x * column(matrix, 0).w + vector.y * column(matrix, 1).w + vector.z * column(matrix, 2).w + column(matrix, 3).w;

  return dvec3(x/w, y/w, z/w);
}


namespace GLUtils {

  //--------------------------------------------------------------------------------------


  //--------------------------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------------------------
  CBaseCamera::CBaseCamera()
  {
      mKeysDown = 0;
      mZAxisUp = false;
      ZeroMemory( mKeys, sizeof( BYTE ) * CAM_MAX_KEYS );

      // Set attributes for the view matrix
      dvec3 vEyePt = dvec3( 0.0, 0.0, 0.0 );
      dvec3 vLookatPt = dvec3( 0.0, 0.0, 1.0 );

      // Setup the view matrix
      SetViewParams( vEyePt, vLookatPt );

      // Setup the projection matrix
      SetProjParams( pi<double>() / 4, 1.0, 1.0, 1000.0 );

      //GetCursorPos( &m_ptLastMousePosition );
      mMouseLButtonDown = false;
      mMouseMButtonDown = false;
      mMouseRButtonDown = false;
      mCurrentButtonMask = 0;
      mMouseWheelDelta = 0;

      mCameraYawAngle = 0.0;
      mCameraPitchAngle = 0.0;

      //SetRect( &m_rcDrag, LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX );
      mVelocity = dvec3( 0, 0, 0 );
      mMovementDrag = false;
      mVelocityDrag = dvec3( 0, 0, 0 );
      mDragTimer = 0.0f;
      mTotalDragTimeToZero = 0.25;
      mRotVelocity = dvec2( 0, 0 );

      mRotationScaler = 0.01;
      mKeyboardMoveScaler = sKeyboardMovementScale;
      mMouseMoveScaler = sMouseMovementScale;

      mInvertPitch = false;
      mEnableYAxisMovement = true;
      mEnablePositionMovement = true;

      mMouseDelta = dvec2( 0, 0 );
      mFramesToSmoothMouseData = 10.0f;

      mClipToBoundary = false;
      mMinBoundary = dvec3( -1, -1, -1 );
      mMaxBoundary = dvec3( 1, 1, 1 );

      mResetCursorAfterMove = false;

      mMouseRotates = true;
  }


  //--------------------------------------------------------------------------------------
  // Client can call this to change the position and direction of camera
  //--------------------------------------------------------------------------------------
  void CBaseCamera::SetViewParams( dvec3 const & pvEyePt, dvec3 const & pvLookatPt )
  {
    const dvec3 localEye = (mZAxisUp) ? dvec3(pvEyePt.x, pvEyePt.z, -pvEyePt.y) : pvEyePt;
    mDefaultEye = mEye = localEye;

    const dvec3 localLookAt = (mZAxisUp) ? dvec3(pvLookatPt.x, pvLookatPt.z, -pvLookatPt.y) : pvLookatPt;
    mDefaultLookAt = mLookAt = localLookAt;

    // Calc the view matrix
    const dvec3 vUp = dvec3( 0.0f ,1.0f,0.0f );
    mViewMatrix = lookAt(localEye, localLookAt, vUp);

    dmat4 mInvView = inverse(mViewMatrix);
    
    // The axis basis vectors and camera position are stored inside the 
    // position matrix in the 4 rows of the camera's world matrix.
    // To figure out the yaw/pitch of the camera, we just need the Z basis vector
    dvec3 pZBasis = MakeVec3(column(mInvView, 2));

    mCameraYawAngle = atan2( pZBasis.x, pZBasis.z );
    double fLen = sqrt(pZBasis.z * pZBasis.z + pZBasis.x * pZBasis.x);
    mCameraPitchAngle = -atan2( pZBasis.y, fLen );
  }




  //--------------------------------------------------------------------------------------
  // Calculates the projection matrix based on input params
  //--------------------------------------------------------------------------------------
  void CBaseCamera::SetProjParams(double fFOV, double fAspect, double fNearPlane, double fFarPlane)
  {
      // Set attributes for the projection matrix
      mFOV = fFOV;
      mAspect = fAspect;
      mNearPlane = fNearPlane;
      mFarPlane = fFarPlane;

      mProjectionMatrix = perspective(fFOV, fAspect, fNearPlane, fFarPlane);
  }



  //--------------------------------------------------------------------------------------
  // Call this from your message proc so this class can handle window messages
  //--------------------------------------------------------------------------------------
  void CBaseCamera::HandleKeys(CameraKeys mappedKey, bool keyDown)
  {
    if (keyDown) {
      // Map this key to a D3DUtil_CameraKeys enum and update the
      // state of m_aKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
      // only if the key is not down
      if( mappedKey != CAM_UNKNOWN ) {
        if( false == IsKeyDown( mKeys[mappedKey] ) ) {
          mKeys[ mappedKey ] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
          ++mKeysDown;
        }
      }
    }
    else {
      // Map this key to a D3DUtil_CameraKeys enum and update the
      // state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
      if( mappedKey != CAM_UNKNOWN && ( DWORD )mappedKey < CAM_MAX_KEYS ) {
          mKeys[ mappedKey ] &= ~KEY_IS_DOWN_MASK;
          --mKeysDown;
      }
    }
  }



  void CBaseCamera::HandleMouse(CameraMouseKeys key, bool keyDown, int x, int y)
  {
    if (keyDown) {
      // Compute the drag rectangle in screen coord.
      ivec2 ptCursor = ivec2(x, y);

      // Update member var state
      if( ( key == MouseLeft) && true /*PtInRect( &m_rcDrag, ptCursor ) */)
      {
          mMouseLButtonDown = true; mCurrentButtonMask |= MOUSE_LEFT_BUTTON;
      }
      if( ( key == MouseMiddle ) && true /*PtInRect( &m_rcDrag, ptCursor ) */ )
      {
          mMouseMButtonDown = true; mCurrentButtonMask |= MOUSE_MIDDLE_BUTTON;
      }
      if( ( key == MouseRight ) && true /*PtInRect( &m_rcDrag, ptCursor ) */ )
      {
          mMouseRButtonDown = true; mCurrentButtonMask |= MOUSE_RIGHT_BUTTON;
      }

      // Capture the mouse, so if the mouse button is 
      // released outside the window, we'll get the WM_LBUTTONUP message
      //SetCapture( hWnd );
      mLastMousePosition.x = x;
      mLastMousePosition.y = y;
      //GetCursorPos( &m_ptLastMousePosition );
    }
    else {
      // Update member var state
      if( key == MouseLeft ) {
          mMouseLButtonDown = false; mCurrentButtonMask &= ~MOUSE_LEFT_BUTTON;
      }
      if( key == MouseMiddle )
      {
          mMouseMButtonDown = false; mCurrentButtonMask &= ~MOUSE_MIDDLE_BUTTON;
      }
      if( key == MouseRight )
      {
          mMouseRButtonDown = false; mCurrentButtonMask &= ~MOUSE_RIGHT_BUTTON;
      }

      // Release the capture if no mouse buttons down
      /*if( !m_bMouseLButtonDown &&
          !m_bMouseRButtonDown &&
          !m_bMouseMButtonDown )
      {
          ReleaseCapture();
      }*/
    }

    mCurrentMousePosition.x = x;
    mCurrentMousePosition.y = y;
  }

  //--------------------------------------------------------------------------------------
  // Figure out the velocity based on keyboard input & drag if any
  //--------------------------------------------------------------------------------------
  void CBaseCamera::GetInput( bool bGetKeyboardInput, bool bGetMouseInput, bool bGetGamepadInput,
                              bool bResetCursorAfterMove )
  {
      mKeyboardDirection = vec3( 0, 0, 0 );
      if( bGetKeyboardInput )
      {
          // Update acceleration vector based on keyboard state
          if( IsKeyDown( mKeys[CAM_MOVE_FORWARD] ) )
              mKeyboardDirection.z -= 1.0f;
          if( IsKeyDown( mKeys[CAM_MOVE_BACKWARD] ) )
              mKeyboardDirection.z += 1.0f;
          if( mEnableYAxisMovement )
          {
              if( IsKeyDown( mKeys[CAM_MOVE_UP] ) )
                  mKeyboardDirection.y += 1.0f;
              if( IsKeyDown( mKeys[CAM_MOVE_DOWN] ) )
                  mKeyboardDirection.y -= 1.0f;
          }
          if( IsKeyDown( mKeys[CAM_STRAFE_RIGHT] ) )
              mKeyboardDirection.x += 1.0f;
          if( IsKeyDown( mKeys[CAM_STRAFE_LEFT] ) )
              mKeyboardDirection.x -= 1.0f;
      }

      if( bGetMouseInput )
      {
          UpdateMouseDelta();
      }
      else {
        mMouseDelta = vec2(0.0);
      }
  }


  //--------------------------------------------------------------------------------------
  // Figure out the mouse delta based on mouse movement
  //--------------------------------------------------------------------------------------
  void CBaseCamera::UpdateMouseDelta()
  {
      ivec2 ptCurMouseDelta;
      ivec2 ptCurMousePos = mCurrentMousePosition;

      // Get current position of mouse
      //GetCursorPos( &ptCurMousePos );

      // Calc how far it's moved since last frame
      ptCurMouseDelta.x = ptCurMousePos.x - mLastMousePosition.x;
      ptCurMouseDelta.y = ptCurMousePos.y - mLastMousePosition.y;

      // Record current position for next time
      mLastMousePosition = ptCurMousePos;

      /*
      if( m_bResetCursorAfterMove)
      {
          // Set position of camera to center of desktop, 
          // so it always has room to move.  This is very useful
          // if the cursor is hidden.  If this isn't done and cursor is hidden, 
          // then invisible cursor will hit the edge of the screen 
          // and the user can't tell what happened
          POINT ptCenter;

          // Get the center of the current monitor
          MONITORINFO mi;
          mi.cbSize = sizeof( MONITORINFO );
          DXUTGetMonitorInfo( DXUTMonitorFromWindow( DXUTGetHWND(), MONITOR_DEFAULTTONEAREST ), &mi );
          ptCenter.x = ( mi.rcMonitor.left + mi.rcMonitor.right ) / 2;
          ptCenter.y = ( mi.rcMonitor.top + mi.rcMonitor.bottom ) / 2;
          SetCursorPos( ptCenter.x, ptCenter.y );
          m_ptLastMousePosition = ptCenter;
      }*/

      if (mMouseRotates) {

        // Smooth the relative mouse data over a few frames so it isn't 
        // jerky when moving slowly at low frame rates.
        double fPercentOfNew = 1.0 / mFramesToSmoothMouseData;
        double fPercentOfOld = 1.0 - fPercentOfNew;
        mMouseDelta.x = mMouseDelta.x * fPercentOfOld + static_cast<float>(ptCurMouseDelta.x) * fPercentOfNew;
        mMouseDelta.y = mMouseDelta.y * fPercentOfOld + static_cast<float>(ptCurMouseDelta.y) * fPercentOfNew;
      }
      else {
        mMouseDelta = vec2(static_cast<float>(ptCurMouseDelta.x), static_cast<float>(ptCurMouseDelta.y));
      }
      //if (mMouseRotates) {
      //  mRotVelocity = mMouseDelta * mRotationScaler;
      //}
      //else {
      //  mKeyboardDirection += vec3(mMouseDelta.x, mMouseDelta.y, 0.0f);
      //}
  }




  //--------------------------------------------------------------------------------------
  // Figure out the velocity based on keyboard input & drag if any
  //--------------------------------------------------------------------------------------
  void CBaseCamera::UpdateVelocity( double fElapsedTime )
  {
      dmat4 mRotDelta;
      dvec3 vAccelMouse(0);

      if (mMouseRotates) {
        mRotVelocity = mMouseDelta * mRotationScaler;
      }
      else {
        vAccelMouse += dvec3(-mMouseDelta.x, mMouseDelta.y, 0.0f);
      }

      dvec3 vAccelKeyboard = mKeyboardDirection;

      // Normalize vector so if moving 2 dirs (left & forward), 
      // the camera doesn't move faster than if moving in 1 dir
      if (!isNull(vAccelKeyboard, 0.0001)) vAccelKeyboard = normalize(vAccelKeyboard);
      if (!isNull(vAccelMouse, 0.0001)) vAccelMouse = normalize(vAccelMouse);

      // Scale the acceleration vector
      vAccelKeyboard *= mKeyboardMoveScaler;
      vAccelMouse *= mMouseMoveScaler;

      dvec3 vAccel = vAccelKeyboard + vAccelMouse;
      if( mMovementDrag )
      {
          // Is there any acceleration this frame?
        if( length(vAccel) > 0.0 )
          {
              // If so, then this means the user has pressed a movement key\
              // so change the velocity immediately to acceleration 
              // upon keyboard input.  This isn't normal physics
              // but it will give a quick response to keyboard input
              mVelocity = vAccel;
              mDragTimer = mTotalDragTimeToZero;
              mVelocityDrag = vAccel / mDragTimer;
          }
          else
          {
              // If no key being pressed, then slowly decrease velocity to 0
              if( mDragTimer > 0 )
              {
                  // Drag until timer is <= 0
                  mVelocity -= mVelocityDrag * fElapsedTime;
                  mDragTimer -= fElapsedTime;
              }
              else
              {
                  // Zero velocity
                  mVelocity = vec3( 0, 0, 0 );
              }
          }
      }
      else
      {
          // No drag, so immediately change the velocity
          mVelocity = vAccel;
      }
  }




  //--------------------------------------------------------------------------------------
  // Clamps pV to lie inside m_vMinBoundary & m_vMaxBoundary
  //--------------------------------------------------------------------------------------
  void CBaseCamera::ConstrainToBoundary( dvec3 * pV )
  {
      // Constrain vector to a bounding box 
      pV->x = std::max( pV->x, mMinBoundary.x );
      pV->y = std::max( pV->y, mMinBoundary.y );
      pV->z = std::max( pV->z, mMinBoundary.z );

      pV->x = std::min( pV->x, mMaxBoundary.x );
      pV->y = std::min( pV->y, mMaxBoundary.y );
      pV->z = std::min( pV->z, mMaxBoundary.z );
  }




  //--------------------------------------------------------------------------------------
  // Maps a windows virtual key to an enum
  //--------------------------------------------------------------------------------------
  CameraKeys CBaseCamera::MapKey( unsigned int nKey )
  {
      // This could be upgraded to a method that's user-definable but for 
      // simplicity, we'll use a hardcoded mapping.
    
    /* should be handled by client!
    switch( nKey )
      {
      
          case VK_CONTROL:
              return CAM_CONTROLDOWN;
          case VK_LEFT:
              return CAM_STRAFE_LEFT;
          case VK_RIGHT:
              return CAM_STRAFE_RIGHT;
          case VK_UP:
              return CAM_MOVE_FORWARD;
          case VK_DOWN:
              return CAM_MOVE_BACKWARD;
          case VK_PRIOR:
              return CAM_MOVE_UP;        // pgup
          case VK_NEXT:
              return CAM_MOVE_DOWN;      // pgdn

          case 'A':
              return CAM_STRAFE_LEFT;
          case 'D':
              return CAM_STRAFE_RIGHT;
          case 'W':
              return CAM_MOVE_FORWARD;
          case 'S':
              return CAM_MOVE_BACKWARD;
          case 'Q':
              return CAM_MOVE_DOWN;
          case 'E':
              return CAM_MOVE_UP;

          case VK_NUMPAD4:
              return CAM_STRAFE_LEFT;
          case VK_NUMPAD6:
              return CAM_STRAFE_RIGHT;
          case VK_NUMPAD8:
              return CAM_MOVE_FORWARD;
          case VK_NUMPAD2:
              return CAM_MOVE_BACKWARD;
          case VK_NUMPAD9:
              return CAM_MOVE_UP;
          case VK_NUMPAD3:
              return CAM_MOVE_DOWN;

          case VK_HOME:
              return CAM_RESET;
      }
  */
      return CAM_UNKNOWN;
  }




  //--------------------------------------------------------------------------------------
  // Reset the camera's position back to the default
  //--------------------------------------------------------------------------------------
  void CBaseCamera::Reset()
  {
      SetViewParams( mDefaultEye, mDefaultLookAt );
  }




  //--------------------------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------------------------
  CFirstPersonCamera::CFirstPersonCamera() 
    : mActiveButtonMask( 0x07 )
    , mCameraWorld(mat4(1.0))
  {
      mRotateWithoutButtonDown = false;
  }




  //--------------------------------------------------------------------------------------
  // Update the view matrix based on user input & elapsed time
  //--------------------------------------------------------------------------------------
  void CFirstPersonCamera::FrameMove( double fElapsedTime )
  {
      if( IsKeyDown( mKeys[CAM_RESET] ) ) {
          Reset();
      }

      if (IsKeyDown(mKeys[CAM_ACCELERATE])) {
        if (mKeyboardMoveScaler < 10000.0) {
          mKeyboardMoveScaler *= 1.2;
        }

        if (mMouseMoveScaler < 10000.0) {
          mMouseMoveScaler *= 1.2;
        }
        //since accelerating shouldn't be done continously, force key up here
        HandleKeys(CAM_ACCELERATE, false);
      }
      if (IsKeyDown(mKeys[CAM_THROTTLE])) {
        if (mKeyboardMoveScaler > 0.1) {
          mKeyboardMoveScaler /= 1.2;
        }

        if (mMouseMoveScaler > 0.1) {
          mMouseMoveScaler /= 1.2;
        }

        HandleKeys(CAM_THROTTLE, false);
      }

      // Get keyboard/mouse/gamepad input
      GetInput( mEnablePositionMovement, ( mActiveButtonMask & mCurrentButtonMask ) || mRotateWithoutButtonDown,
                true, mResetCursorAfterMove );

      // Get amount of velocity based on the keyboard input and drag (if any)
      UpdateVelocity( fElapsedTime );

      // Simple euler method to calculate position delta
      dvec3 vPosDelta = mVelocity * fElapsedTime;

      // If rotating the camera 
      if (mMouseRotates) {
        if( ( mActiveButtonMask & mCurrentButtonMask ) || mRotateWithoutButtonDown) {

            // Update the pitch & yaw angle based on mouse movement
          double fYawDelta = mRotVelocity.x;
          double fPitchDelta = mRotVelocity.y;

            // Invert pitch if requested
            if( mInvertPitch )
                fPitchDelta = -fPitchDelta;

            mCameraPitchAngle -= fPitchDelta;
            mCameraYawAngle -= fYawDelta;

            // Limit pitch to straight up or straight down
            mCameraPitchAngle = std::max( -pi<double>() * 0.499, mCameraPitchAngle );
            mCameraPitchAngle = std::min( +pi<double>() * 0.499, mCameraPitchAngle );
        }
      }

      // Make a rotation matrix based on the camera's yaw & pitch
      dmat4 mCameraRot = yawPitchRoll(mCameraYawAngle, mCameraPitchAngle, 0.0);


      // Transform vectors based on camera's rotation matrix
      dvec3 vWorldUp, vWorldAhead;
      const dvec3 vLocalUp = dvec3( 0, 1, 0 );
      const dvec3 vLocalAhead = dvec3( 0, 0, -1 );

      vWorldUp = Vec3TransformCoord(vLocalUp, mCameraRot);
      vWorldAhead = Vec3TransformCoord(vLocalAhead, mCameraRot);

      // Transform the position delta by the camera's rotation 
      dvec3 vPosDeltaWorld;
      if( !mEnableYAxisMovement )
      {
          // If restricting Y movement, do not include pitch
          // when transforming position delta vector.
          mCameraRot = yawPitchRoll(mCameraYawAngle, 0.0, 0.0 );
      }

      vPosDeltaWorld = Vec3TransformCoord(vPosDelta, mCameraRot );

      // Move the eye position 
      mEye += vPosDeltaWorld;
      if( mClipToBoundary )
          ConstrainToBoundary( &mEye );

      // Update the lookAt position based on the eye position 
      mLookAt = mEye + vWorldAhead;

      // Update the view matrix
      mViewMatrix = lookAt(mEye, mLookAt, vWorldUp );

      mCameraWorld = inverse(mViewMatrix );
  }


  //--------------------------------------------------------------------------------------
  // Enable or disable each of the mouse buttons for rotation drag.
  //--------------------------------------------------------------------------------------
  void CFirstPersonCamera::SetMouseButtons( bool bLeft, bool bMiddle, bool bRight, bool bRotateWithoutButtonDown )
  {
      mActiveButtonMask = ( bLeft ? MOUSE_LEFT_BUTTON : 0 ) |
                            ( bMiddle ? MOUSE_MIDDLE_BUTTON : 0 ) |
                            ( bRight ? MOUSE_RIGHT_BUTTON : 0 );
      mRotateWithoutButtonDown = bRotateWithoutButtonDown;
  }



  dvec3 CFirstPersonCamera::GetWorldRight() const 
  { 
    const dvec3 worldRightYUp = MakeVec3(column(mCameraWorld, 0));
    return (mZAxisUp) ? dvec3(worldRightYUp.x, -worldRightYUp.z, worldRightYUp.y) : worldRightYUp;
  }



  dvec3 CFirstPersonCamera::GetWorldUp() const 
  {
    const dvec3 worldUpYUp = MakeVec3(column(mCameraWorld, 1)); 
    return (mZAxisUp) ? dvec3(worldUpYUp.x, -worldUpYUp.z, worldUpYUp.y) : worldUpYUp;
  }



  dvec3 CFirstPersonCamera::GetWorldAhead() const 
  {
    const dvec3 lookAtYUp = MakeVec3(column(mCameraWorld, 2)); 
    return (mZAxisUp) ? dvec3(lookAtYUp.x, -lookAtYUp.z, lookAtYUp.y) : lookAtYUp;
  }



  dvec3 CFirstPersonCamera::GetEyePt() const 
  {
    const dvec3 eyeYUp = MakeVec3(column(mCameraWorld, 3)); 
    return (mZAxisUp) ?  dvec3(eyeYUp.x, -eyeYUp.z, eyeYUp.y) : eyeYUp;
  }



} //namespace Utils