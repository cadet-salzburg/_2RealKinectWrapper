#ifdef TARGET_MSKINECTSDK
#include "_2RealConfig.h"
#include "WinSDKSpecific/WSDKDevice.h"
#include <sstream>
#include "_2RealTypes.h"
#include "_2RealTrackedUser.h"
#include "_2RealImplementationWinSDK.hpp"

namespace _2RealKinectWrapper
{

unsigned char Colors[][3] =
{		
	{0,255,255},
	{0,0,255},
	{0,255,0},
	{255,255,0},
	{255,0,0},
	{255,127,0},
	{127,255,0},
	{0,127,255},
	{127,0,255},
	{255,255,127},
	{255,255,255}
};
	
WSDKDevice::~WSDKDevice(void)
{
	shutdown();
}


WSDKDevice::WSDKDevice( INuiSensor* devicePtr, const std::string& name )
	//respect member initialization order -> order as declarations in header
	: m_WidthImageDepthAndUser( m_Configuration.m_ImageResDepth.width ),
	  m_WidthImageColor( m_Configuration.m_ImageResColor.width ),
	  m_HeightImageDepthAndUser( m_Configuration.m_ImageResDepth.height ),
	  m_HeightImageColor( m_Configuration.m_ImageResColor.height ),
	  m_pNuiSensor( devicePtr ),
	  m_Name( name ),
	  m_DeviceID( devicePtr->NuiInstanceIndex() ),
	  m_EventColorImage( CreateEvent( NULL, TRUE, FALSE, NULL ) ), //creating event handles
	  m_EventDepthImage( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	  m_EventSkeletonData( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	  m_EventStopThread( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	  m_HandleColorStream( NULL ),
	  m_HandleDepthStream( NULL ),
	  m_HandleThread( NULL ),
	  m_isDeviceStarted( false ),
	  m_isDeviceShutDown( true ),
	  // needed?!? ---->
	  m_bIsDepthOnly( true ), 
	  m_bIsMirroringColor( false ),
	  m_bIsMirroringDepth( false ),
	  m_bIsMirroringUser( false ),
	  m_bIsDeletingDevice( false ),
	  m_bIsNewColorData( false),
	  m_bIsNewDepthData( false ),
	  m_bIsNewUserData( false ),
	  m_bIsNewSkeletonData( false ),
	  m_bIsNewInfraredData( false ),
	  //<----
	  m_ImageColor_8bit( NULL ),
	  m_ImageDepth_8bit( NULL ),
	  m_ImageUser_8bit( NULL ),
	  m_ImageColoredUser_8bit( NULL ),
	  m_ImageDepth_16bit( NULL )
{

}


void WSDKDevice::initColorStream()
{
	_2REAL_LOG( info ) << "_2Real: Initializing COLOR-STREAM on device: " << m_Name
					   << " ; setting RGB resolution to "
					   << "width: " << m_WidthImageColor
					   << " / height: " << m_HeightImageColor << " ...";

	//allocating image buffer
	m_ImageColor_8bit = boost::shared_array<uchar>( new uchar[m_WidthImageColor*m_HeightImageColor*3] );

	//starting stream
	HRESULT status = m_pNuiSensor->NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR,
													   m_Configuration.m_ImageResColor.WSDKResType,
													   0,
													   2,
													   m_EventColorImage,
													   &m_HandleColorStream );

	if( FAILED( status ) )
	{
		std::stringstream ss( "" );
		ss	<< "_2Real: Error when trying to open COLOR-IMAGE-STREAM on device: " << m_Name
			<< "; Try another resolution for sensor!!" << std::endl;
		throwError( ss.str().c_str() );
	}
	_2REAL_LOG( info ) << "OK" << std::endl;
}

void WSDKDevice::initDepthStream()
{
	_2REAL_LOG( info )	<< "_2Real: Initializing DEPTH(ONLY)-STREAM on device: " << m_Name
						<< " ; setting RGB resolution to "
						<< "width: " << m_WidthImageDepthAndUser
						<< " / height: " << m_HeightImageDepthAndUser << " ...";

	m_bIsDepthOnly = true;
	const uint32_t size = m_WidthImageDepthAndUser * m_HeightImageDepthAndUser;
	m_ImageDepth_8bit = boost::shared_array<uchar>( new uchar[size] );
	m_ImageDepth_16bit = boost::shared_array<uint16_t>( new uint16_t[size] );

	HRESULT status = m_pNuiSensor->NuiImageStreamOpen(	NUI_IMAGE_TYPE_DEPTH,
														m_Configuration.m_ImageResDepth.WSDKResType,
														0,
														2,
														m_EventDepthImage,
														&m_HandleDepthStream );

	if( FAILED( status ) )
	{
		std::stringstream ss( "" );
		ss	<< "_2Real: Error when trying to open DEPTH(ONLY)-STREAM on device: " << m_Name
			<< "; Try another resolution for sensor!!" << std::endl;
		throwError( ss.str().c_str() );
	}
	_2REAL_LOG( info ) << "OK" << std::endl;
}

void WSDKDevice::initUserDepthStream()
{
	_2REAL_LOG( info )	<< "_2Real: Initializing USER_DEPTH-STREAM on device: " << m_Name
						<< " ; setting RGB resolution to "
						<< "width: " << m_WidthImageDepthAndUser
						<< " / height: " << m_HeightImageDepthAndUser << " ...";

	m_bIsDepthOnly = false;
	const uint32_t size = m_WidthImageDepthAndUser * m_HeightImageDepthAndUser;

	m_ImageDepth_8bit =  boost::shared_array<uchar>( new uchar[size] );
	m_ImageDepth_16bit =  boost::shared_array<uint16_t>( new uint16_t[size] );
	m_ImageUser_8bit = boost::shared_array<uchar>( new uchar[size] );
	m_ImageColoredUser_8bit = boost::shared_array<uchar>( new uchar[size*3] );

	//starting depth stream
	HRESULT status = m_pNuiSensor->NuiImageStreamOpen(	NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
														m_Configuration.m_ImageResUser.WSDKResType,
														0,
														2,
														m_EventDepthImage,
														&m_HandleDepthStream );

	if( FAILED( status ) )
	{
		std::stringstream ss( "" );
		ss	<< "_2Real: Error when trying to open USER_DEPTH-STREAM on device: " << m_Name
			<< "; Try another resolution for sensor!!" << std::endl;
		throwError( ss.str().c_str() );
	}
	_2REAL_LOG( info ) << "OK" << std::endl;

	//skeleton tracking
	_2REAL_LOG( info ) << "_2Real: Starting skeleton tracking on device: " << m_Name << " ..." << std::endl;

	status = m_pNuiSensor->NuiSkeletonTrackingEnable( m_EventSkeletonData, 0 );
	if( FAILED( status ) )
		throwError( std::string( "_2Real: Error when trying to enable skeleton tracking on device: " ).append( m_Name ) );

	_2REAL_LOG( info ) << "OK" << std::endl;
}

const bool WSDKDevice::isNewData(_2RealGenerator type) const
{
	//not implemented yet
	return true;
}

DWORD WINAPI WSDKDevice::threadEventsFetcher( LPVOID pParam )
{
	WSDKDevice* pThis = static_cast<WSDKDevice*>( pParam );
	_2REAL_LOG(info) << "\nStarted thread id: " << int( pThis->m_HandleThread ) << std::endl;

	int eventIndex = 0;

	while( true )
	{
		try
		{
			//waiting for events
			eventIndex = WaitForMultipleObjects( 4, pThis->m_WTEvents, false, INFINITE );

			switch( eventIndex )
			{
			case WT_STOP_THREAD:
				return 0;
			case WT_EVENT_COLOR:
				pThis->processColorImageEvent();
				break;
			case WT_EVENT_DEPTH:
				pThis->processDepthImageEvent();
				break;
			case WT_EVENT_SKELETON:
				pThis->processSkeletonEvent();
				break;
			}
		}
		catch( std::exception& e )
		{
			std::cout << e.what() << std::endl;
		}
		catch( ... )
		{

		}
	}
	return 0;
}

void WSDKDevice::processColorImageEvent()
{	
	NUI_IMAGE_FRAME imageFrame;

	if( m_bIsDeletingDevice )
		return;

	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_HandleColorStream, 0, &imageFrame );

	if ( FAILED( hr ) )
	{
		_2REAL_LOG(error) << "_2Real: Device " << m_Name << " skipping frame. Error while trying to fetch color data..." << std::endl;
		return;
	}

	boost::mutex::scoped_lock lock(m_MutexImage);
	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	if ( LockedRect.Pitch != 0 )
	{
		//copy over pixels from BGRX to RGB
		int size = m_WidthImageColor * m_HeightImageColor; //get number of pixels
		uint32_t width = m_WidthImageColor;
		//setting pointers; structures designed for iteration over 24b and 32b image (WSDKDevice.h)
		BGRX* pSource = reinterpret_cast<BGRX*>( LockedRect.pBits );
		RGB* pDestination = reinterpret_cast<RGB*>( m_ImageColor_8bit.get() );

		for( int i = 0;	 i < size; ++i )//loop enrolled: 10 iterations in 1 step
		{
			int index = 0;
			if( !m_bIsMirroringColor )
				index = (( ( i / width + 1 ) *  width ) - ( i % width )) - 1;
			else
				index = i;

			pDestination[index].r = pSource[i].r;
			pDestination[index].g = pSource[i].g;
			pDestination[index].b = pSource[i].b;
		}
	}
	else
	{
		OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
	}

	pTexture->UnlockRect( 0 );

	m_pNuiSensor->NuiImageStreamReleaseFrame( m_HandleColorStream, &imageFrame );
	m_NotificationNewColorImageData.notify_all();
	m_bIsNewColorData = true;
}

void WSDKDevice::processDepthImageEvent()
{
	if( m_bIsDeletingDevice )
		return;

	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_HandleDepthStream, 0, &imageFrame );
	if(FAILED(hr))
	{
		_2REAL_LOG(error) << "_2Real: Device " << m_Name << " skipping frame. Error while trying to fetch depth data..." << std::endl;
		return;
	}
	boost::mutex::scoped_lock lock(m_MutexDepth);
	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	
	int size = m_WidthImageDepthAndUser * m_HeightImageDepthAndUser; //get number of pixels
	uint32_t width = m_WidthImageDepthAndUser;

	uint16_t* source = reinterpret_cast<uint16_t*>( LockedRect.pBits );
	int colors = 8;	// max users for MS SDK
	for( int i = 0;	 i < size; ++i ) //loop enrolled: 10 iterations in 1 step
	{
		int index = 0;

		//mirror capability
		if( !m_bIsMirroringDepth )
			index = (( ( i / width + 1 ) *  width ) - ( i % width )) - 1;
		else
			index = i;

		//writing depth only or user and depth image
		if( m_bIsDepthOnly )
		{
			//12 low bits used of 16bit buffer
			m_ImageDepth_16bit[index] = source[i];
			m_ImageDepth_8bit[index] = uchar( ( (float)source[i] / _2REAL_WSDK_DEPTH_NORMALIZATION_16_TO_8 ) * 255 );
		}
		else
		{
			//using 12 high bits out of 16bit buffer
			m_ImageDepth_16bit[index] = source[i] >> 3;
			m_ImageDepth_8bit[index] = uchar( ( (float)( source[i] >> 3 ) / _2REAL_WSDK_DEPTH_NORMALIZATION_16_TO_8 ) * 255 ); 

			//mirror capability
			if( !m_bIsMirroringUser )
				index = (( ( i / width + 1 ) *  width ) - ( i % width )) - 1;
			else
				index = i;

			//getting first 3 low bits out of 16bit buffer
			m_ImageUser_8bit[index] = source[i] & 0x0007;
			m_ImageColoredUser_8bit[index*3] = 0;
			m_ImageColoredUser_8bit[index*3+1] = 0;
			m_ImageColoredUser_8bit[index*3+2] = 0;

			if(m_ImageUser_8bit[index]%colors != 0)
			{			
				m_ImageColoredUser_8bit[index*3] = Colors[m_ImageUser_8bit[index]%colors][0];
				m_ImageColoredUser_8bit[index*3+1] = Colors[m_ImageUser_8bit[index]%colors][1];
				m_ImageColoredUser_8bit[index*3+2] = Colors[m_ImageUser_8bit[index]%colors][2];
			}
		}		
		
	}
	//remove lock -> 2 imageframes buffer
	//locking both framebuffer will cause an exception
	pTexture->UnlockRect( 0 );
	m_pNuiSensor->NuiImageStreamReleaseFrame( m_HandleDepthStream, &imageFrame );
	m_NotificationNewDepthImageData.notify_all();
	m_bIsNewDepthData = true;
}

void WSDKDevice::processSkeletonEvent()
{
	if( m_bIsDeletingDevice )
		return;

	boost::mutex::scoped_lock lock(m_MutexUser);
	
	NUI_SKELETON_FRAME nuiFrame;
	m_Users.clear();
	
	if( FAILED( m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &nuiFrame ) ) )
	{
		_2REAL_LOG(error) << "_2Real: Device " << m_Name << " skipping frame. Error while trying to fetch skeleton data..." << std::endl;
		return;
	}

	//tracked skeletons available?
	bool detectedSkeletons = false;
	for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
	{
		if( nuiFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
			detectedSkeletons = true;
	}

	//exiting if no tracked skeletons
	if( !detectedSkeletons )
	{
		m_NotificationNewUserdata.notify_all();
		return;
	}

	// smooth out the skeleton data
	NUI_TRANSFORM_SMOOTH_PARAMETERS params;
	params.fCorrection = float( _2REAL_WSDK_CORRECTION );
	params.fJitterRadius = float( _2REAL_WSDK_JITTER_RATIUS );
	params.fMaxDeviationRadius = float( _2REAL_WSDK_MAX_DEVIATION_RADIUS );
	params.fPrediction = float( _2REAL_WSDK_PREDICTION );
	params.fSmoothing = float( _2REAL_WSDK_SMOOTHING );

	NuiTransformSmooth( &nuiFrame, &params );

	_2RealTrackedUser_sptr user;
	//get or create user
	for( int i = 0; i < NUI_SKELETON_COUNT; ++i )
	{
		if( nuiFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
		{
			m_Users.push_back( _2RealTrackedUser_sptr( new _2RealTrackedUser( nuiFrame.SkeletonData[i].dwTrackingID ) ) );
			user = m_Users.back();
		}
		else
			continue;

		//calculate nui-orientations
		NUI_SKELETON_BONE_ORIENTATION nuiOrientations[NUI_SKELETON_POSITION_COUNT];
		NuiSkeletonCalculateBoneOrientations( &nuiFrame.SkeletonData[i], nuiOrientations );

		//setting/updating joints
		user->setJoint( JOINT_HEAD, createJointFromNUI( JOINT_HEAD, NUI_SKELETON_POSITION_HEAD, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_NECK, createJointFromNUI( JOINT_NECK, NUI_SKELETON_POSITION_SHOULDER_CENTER, nuiFrame.SkeletonData[i], nuiOrientations ) ); 
		user->setJoint( JOINT_TORSO, createJointFromNUI( JOINT_TORSO, NUI_SKELETON_POSITION_SPINE, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_WAIST, createJointFromNUI( JOINT_WAIST, NUI_SKELETON_POSITION_HIP_CENTER, nuiFrame.SkeletonData[i], nuiOrientations ) );

		user->setJoint( JOINT_LEFT_COLLAR, _2RealTrackedJoint_sptr( new _2RealTrackedJoint( JOINT_LEFT_COLLAR ) ) ); //empty joint -> not supported
		user->setJoint( JOINT_LEFT_SHOULDER, createJointFromNUI( JOINT_LEFT_SHOULDER, NUI_SKELETON_POSITION_SHOULDER_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_LEFT_ELBOW, createJointFromNUI( JOINT_LEFT_ELBOW, NUI_SKELETON_POSITION_ELBOW_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_LEFT_WRIST, createJointFromNUI( JOINT_LEFT_WRIST, NUI_SKELETON_POSITION_WRIST_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_LEFT_HAND, createJointFromNUI( JOINT_LEFT_HAND, NUI_SKELETON_POSITION_HAND_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_LEFT_FINGERTIP, _2RealTrackedJoint_sptr( new _2RealTrackedJoint( JOINT_LEFT_FINGERTIP ) ) ); //empty joint -> not supported

		user->setJoint( JOINT_RIGHT_COLLAR, _2RealTrackedJoint_sptr( new _2RealTrackedJoint( JOINT_LEFT_COLLAR ) ) ); //empty joint -> not supported
		user->setJoint( JOINT_RIGHT_SHOULDER, createJointFromNUI( JOINT_RIGHT_SHOULDER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_RIGHT_ELBOW, createJointFromNUI( JOINT_RIGHT_ELBOW, NUI_SKELETON_POSITION_ELBOW_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_RIGHT_WRIST, createJointFromNUI( JOINT_RIGHT_WRIST, NUI_SKELETON_POSITION_WRIST_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_RIGHT_HAND, createJointFromNUI( JOINT_RIGHT_HAND, NUI_SKELETON_POSITION_HAND_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_RIGHT_FINGERTIP, _2RealTrackedJoint_sptr( new _2RealTrackedJoint( JOINT_RIGHT_FINGERTIP ) ) ); //empty joint -> not supported
		
		user->setJoint( JOINT_LEFT_HIP, createJointFromNUI( JOINT_LEFT_HIP, NUI_SKELETON_POSITION_HIP_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_LEFT_KNEE, createJointFromNUI( JOINT_LEFT_KNEE, NUI_SKELETON_POSITION_KNEE_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_LEFT_ANKLE, createJointFromNUI( JOINT_LEFT_ANKLE, NUI_SKELETON_POSITION_ANKLE_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_LEFT_FOOT, createJointFromNUI( JOINT_LEFT_FOOT, NUI_SKELETON_POSITION_FOOT_LEFT, nuiFrame.SkeletonData[i], nuiOrientations ) );

		user->setJoint( JOINT_RIGHT_HIP, createJointFromNUI( JOINT_RIGHT_HIP, NUI_SKELETON_POSITION_HIP_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_RIGHT_KNEE, createJointFromNUI( JOINT_RIGHT_KNEE, NUI_SKELETON_POSITION_KNEE_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_RIGHT_ANKLE, createJointFromNUI( JOINT_RIGHT_ANKLE, NUI_SKELETON_POSITION_ANKLE_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
		user->setJoint( JOINT_RIGHT_FOOT, createJointFromNUI( JOINT_RIGHT_FOOT, NUI_SKELETON_POSITION_FOOT_RIGHT, nuiFrame.SkeletonData[i], nuiOrientations ) );
	}
	m_NotificationNewUserdata.notify_all();
	m_bIsNewUserData = m_bIsNewSkeletonData = true;
}

_2RealTrackedJoint_sptr WSDKDevice::createJointFromNUI( _2RealJointType type, _NUI_SKELETON_POSITION_INDEX nuiType, const NUI_SKELETON_DATA& data, const NUI_SKELETON_BONE_ORIENTATION* nuiOrientation )
{
	_2RealVector3f screenPos;
	_2RealVector3f worldPos;
	
	// Mirror skeleton joints when user image is mirrored
	Vector4 position = data.SkeletonPositions[nuiType];
	if(!m_bIsMirroringUser)
	{
		position.x = (FLOAT)1.0 -position.x;
	}
	else
		NuiTransformSkeletonToDepthImage( position, (FLOAT*)&screenPos.x, (FLOAT*)&screenPos.y );
	
	screenPos.z = data.SkeletonPositions[nuiType].z;
	worldPos.x = data.SkeletonPositions[nuiType].x;
	worldPos.y = data.SkeletonPositions[nuiType].y;
	worldPos.z = data.SkeletonPositions[nuiType].z;
	
	// fetch orientations
	
	const NUI_SKELETON_BONE_ORIENTATION& boneOrientation = nuiOrientation[nuiType];
	
	_2RealMatrix3x3 rotMatrix;
	rotMatrix.m11 = boneOrientation.absoluteRotation.rotationMatrix.M11;
	rotMatrix.m12 = boneOrientation.absoluteRotation.rotationMatrix.M12;
	rotMatrix.m13 = boneOrientation.absoluteRotation.rotationMatrix.M13;

	rotMatrix.m21 = boneOrientation.absoluteRotation.rotationMatrix.M21;
	rotMatrix.m22 = boneOrientation.absoluteRotation.rotationMatrix.M22;
	rotMatrix.m23 = boneOrientation.absoluteRotation.rotationMatrix.M23;

	rotMatrix.m31 = boneOrientation.absoluteRotation.rotationMatrix.M31;
	rotMatrix.m32 = boneOrientation.absoluteRotation.rotationMatrix.M32;
	rotMatrix.m33 = boneOrientation.absoluteRotation.rotationMatrix.M33;

	
	return _2RealTrackedJoint_sptr( new _2RealTrackedJoint( type,
															screenPos,
															worldPos,
															rotMatrix,
															_2RealJointConfidence( 1, 1 )));
}

void WSDKDevice::getUsers( bool waitForNewData, _2RealTrackedUserVector& out )
{
	boost::mutex::scoped_lock lock( m_MutexFetchUser );
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		m_NotificationNewUserdata.wait( lock );
	}
	out.clear();
	//prevent changing data fetching thread to change vector while copying
	boost::mutex::scoped_lock lock1( m_MutexUser );
	out = m_Users;
}

unsigned int WSDKDevice::getNumberOfUsers()
{
	//prevent changing data fetching thread to change vector while copying
	boost::mutex::scoped_lock lock( m_MutexUser );
	return m_Users.size();
}


boost::shared_array<uchar> WSDKDevice::getColorImageBuffer( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::mutex::scoped_lock lock( m_MutexFetchColorImage );
		m_NotificationNewColorImageData.wait( lock );
	}
	m_bIsNewColorData = false;
	return m_ImageColor_8bit;
}

boost::shared_array<uchar> WSDKDevice::getDepthImageBuffer( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::mutex::scoped_lock lock( m_MutexFetchDepthImage );
		m_NotificationNewDepthImageData.wait( lock );
	}
	return m_ImageDepth_8bit;
}

boost::shared_array<uchar> WSDKDevice::getUserImageBuffer( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::mutex::scoped_lock lock( m_MutexFetchDepthImage2 );
		m_NotificationNewDepthImageData.wait( lock );
	}
	return m_ImageUser_8bit;
}


boost::shared_array<uint16_t> WSDKDevice::getDepthImageBuffer16Bit( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::mutex::scoped_lock lock( m_MutexFetchDepthImage3 );
		m_NotificationNewDepthImageData.wait( lock );
	}
	return m_ImageDepth_16bit;
}

boost::shared_array<uchar> WSDKDevice::getColoredUserImageBuffer( bool waitForNewData )
{
	if( waitForNewData )
	{
		boost::mutex::scoped_lock lock( m_MutexColoredUser );
		m_NotificationNewDepthImageData.wait( lock );
	}
	return m_ImageColoredUser_8bit;
}

bool WSDKDevice::setMotorAngle(int angle)
{
	if(angle<=27 && angle>=-27)
	{
		m_pNuiSensor->NuiCameraElevationSetAngle( angle );
		return true;
	}
	else
	{
		return false;
	}
}

int WSDKDevice::getMotorAngle()
{
	LONG tmp;
	m_pNuiSensor->NuiCameraElevationGetAngle(&tmp);
	return tmp;
}

void WSDKDevice::shutdown()
{
	if( m_isDeviceShutDown ) return;

	m_bIsDeletingDevice = 1;
	m_isDeviceShutDown = true; //needs to be set before stop()
	stop( true );

	if( m_pNuiSensor )
	{
		m_pNuiSensor->NuiShutdown();
		m_pNuiSensor->Release();
	}
}

void WSDKDevice::start()
{
	if( m_isDeviceStarted ) return;

	// initializing
	_2REAL_LOG( info ) << "_2Real: Initialize device: " << m_DeviceID << " ...";

	HRESULT status = 0;
	if( FAILED( status = m_pNuiSensor->NuiInitialize( m_Configuration.m_GeneratorsWSDK ) ) )
	{ 
		throwError( "_2Real: Error when trying to initialize device");
	}
	_2REAL_LOG( info ) << "OK" << std::endl;


	// opening NUI streams depending on config flags------------------------------->
	if( m_Configuration.m_GeneratorsWSDK & NUI_INITIALIZE_FLAG_USES_COLOR )
		initColorStream();
	if( m_Configuration.m_GeneratorsWSDK & NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX )
		initUserDepthStream();
	else if( m_Configuration.m_GeneratorsWSDK & NUI_INITIALIZE_FLAG_USES_DEPTH )
		initDepthStream();


	// set mirror flags default to true
	m_bIsMirroringColor = m_bIsMirroringDepth = m_bIsMirroringUser = 1;

	// if it was not a complete shutdown 
	if( !m_HandleThread )
	{
		_2REAL_LOG( info ) << "_2Real: Starting own thread for fetching sensor data...";
		m_HandleThread = CreateThread( NULL, 0, threadEventsFetcher, this, NULL, NULL );
		if( m_HandleThread )
		{
			_2REAL_LOG( info ) << "OK" << std::endl;
		}
		else
		{
			_2REAL_LOG( error ) << "Starting thread failed" << std::endl;
		}
	}

	// settings handles for worker thread
	m_WTEvents[WT_EVENT_COLOR] = m_EventColorImage;
	m_WTEvents[WT_EVENT_DEPTH] = m_EventDepthImage;
	m_WTEvents[WT_EVENT_SKELETON] = m_EventSkeletonData;
	m_WTEvents[WT_STOP_THREAD] = m_EventStopThread;
	m_isDeviceStarted = true;
	m_isDeviceShutDown = false;
}

void WSDKDevice::stop( const bool shutdown )
{
	// i.e called by shutdown() stop thread thingys too
	if( m_isDeviceShutDown == true && shutdown ) 
	{
		// wait for WT to finish
		SetEvent( m_EventStopThread );
		WaitForSingleObject( m_HandleThread, INFINITE );

		CloseHandle( m_EventStopThread );
		CloseHandle( m_HandleThread );
		CloseHandle( m_EventColorImage );
		CloseHandle( m_EventDepthImage );
		CloseHandle( m_EventSkeletonData );

		m_WTEvents[WT_STOP_THREAD] = nullptr;
		m_EventStopThread = nullptr;
	}
	if( !m_isDeviceStarted ) return;

	// prevent from WT to fetch events
	m_WTEvents[WT_EVENT_COLOR] = nullptr;
	m_WTEvents[WT_EVENT_DEPTH] = nullptr;
	m_WTEvents[WT_EVENT_SKELETON] = nullptr;
	m_isDeviceStarted = false;
}

bool WSDKDevice::isDeviceStarted() const
{
	return m_isDeviceStarted;
}

void WSDKDevice::startGenerator( uint32_t generators )
{
	if( generators & COLORIMAGE )
		m_WTEvents[WT_EVENT_COLOR] = m_EventColorImage;
	if( generators & USERIMAGE )
		m_WTEvents[WT_EVENT_SKELETON] = m_EventSkeletonData;
	if( generators & DEPTHIMAGE )
		m_WTEvents[WT_EVENT_DEPTH] = m_EventDepthImage;
	if( ( generators & INFRAREDIMAGE ) )
	{
		_2REAL_LOG( info ) << "_2Real: Infrared capability is not supported by Win-SDK!" << std::endl;
	}
}

void WSDKDevice::stopGenerator( uint32_t generators )
{
	if( generators & COLORIMAGE )
		m_WTEvents[WT_EVENT_COLOR] = nullptr;
	if( generators & USERIMAGE )
		m_WTEvents[WT_EVENT_SKELETON] = nullptr;
	if( generators & DEPTHIMAGE )
		m_WTEvents[WT_EVENT_DEPTH] = nullptr;
	if( ( generators & INFRAREDIMAGE ) )
	{
		_2REAL_LOG( info ) << "_2Real: Infrared capability is not supported by Win-SDK!" << std::endl;
	}
}

bool WSDKDevice::isDeviceShutDown() const
{
	return m_isDeviceShutDown;
}

WSDKDeviceConfiguration& WSDKDevice::getDeviceConfiguration() const
{
	return m_Configuration;
}

}
#endif