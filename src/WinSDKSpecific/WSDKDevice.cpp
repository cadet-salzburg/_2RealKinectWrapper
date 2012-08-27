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

WSDKDevice::WSDKDevice( INuiSensor* devicePtr, const std::string& name )
	//respect member initialization order -> order as declarations in header
	: m_WidthImageDepthAndUser( m_Configuration.m_ImageResDepth.width ),
	  m_WidthImageColor( m_Configuration.m_ImageResColor.width ),
	  m_HeightImageDepthAndUser( m_Configuration.m_ImageResDepth.height ),
	  m_HeightImageColor( m_Configuration.m_ImageResColor.height ),
	  m_NuiSensor( devicePtr ),
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
	  m_IsDeletingDevice( false ),
	  m_DeviceFlags( WSDKDeviceFlag( DFLAG_MIRROR_COLOR | DFLAG_MIRROR_DEPTH | DFLAG_MIRROR_USER ) ),
	  m_ImageColor_8bit( NULL ),
	  m_ImageDepth_8bit( NULL ),
	  m_ImageUser_8bit( NULL ),
	  m_ImageColoredUser_8bit( NULL ),
	  m_ImageDepth_16bit( NULL ),
	  m_ColorCoords( nullptr ),
	  m_ColorCoordsSize( 0 )
{

}


void WSDKDevice::initColorCoords( uint32_t totalPixels )
{
	m_ColorCoordsSize = totalPixels;
	m_ColorCoords = boost::shared_array<LONG>( new LONG[m_ColorCoordsSize] );
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
	HRESULT status = m_NuiSensor->NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR,
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

	// set depth only flag to enabled
	setFlag( DFLAG_DEPTH_ONLY, true );

	const uint32_t size = m_WidthImageDepthAndUser * m_HeightImageDepthAndUser;

	m_ImageDepth_8bit = boost::shared_array<uchar>( new uchar[size] );
	m_ImageDepth_16bit = boost::shared_array<uint16_t>( new uint16_t[size] );
	initColorCoords( size );

	HRESULT status = m_NuiSensor->NuiImageStreamOpen(	NUI_IMAGE_TYPE_DEPTH,
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

	// set depth only flag to disabled
	setFlag( DFLAG_DEPTH_ONLY, false );

	const uint32_t size = m_WidthImageDepthAndUser * m_HeightImageDepthAndUser;

	m_ImageDepth_8bit =  boost::shared_array<uchar>( new uchar[size] );
	m_ImageDepth_16bit =  boost::shared_array<uint16_t>( new uint16_t[size] );
	m_ImageUser_8bit = boost::shared_array<uchar>( new uchar[size] );
	m_ImageColoredUser_8bit = boost::shared_array<uchar>( new uchar[size*3] );
	initColorCoords( size );

	//starting depth stream
	HRESULT status = m_NuiSensor->NuiImageStreamOpen(	NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
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

	status = m_NuiSensor->NuiSkeletonTrackingEnable( m_EventSkeletonData, 0 );
	if( FAILED( status ) )
		throwError( std::string( "_2Real: Error when trying to enable skeleton tracking on device: " ).append( m_Name ) );

	_2REAL_LOG( info ) << "OK" << std::endl;
}

bool WSDKDevice::isNewData(_2RealGenerator type) const
{
	//not implemented yet
	// this function is ridiculous!!
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
	if( m_IsDeletingDevice )
		return;
	
	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = m_NuiSensor->NuiImageStreamGetNextFrame( m_HandleColorStream, 0, &imageFrame );

	if ( FAILED( hr ) )
	{
		_2REAL_LOG( error ) << "_2Real: Device " << m_Name << " skipping frame. Error"
							<< "while trying to fetch color data in processColorImageEvent()..." << std::endl;
		return;
	}

	boost::mutex::scoped_lock lock(m_MutexImage);
	INuiFrameTexture* pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );

	// ensure no change of flags during processing buffer-data
	bool isMirrorColor = isFlagEnabled( DFLAG_MIRROR_COLOR );
	bool isAlignDepthColor = isFlagEnabled( DFLAG_ALIGN_COLOR_DEPTH );

	// copy over pixels from BGRX to RGB
	if ( LockedRect.Pitch != 0 )
	{
		int size = m_WidthImageColor * m_HeightImageColor; //get number of pixels
		//setting pointers; structures designed for iteration over 24b and 32b image (WSDKDevice.h)
		BGRX* pSource = reinterpret_cast<BGRX*>( LockedRect.pBits );
		RGB* pDestination = reinterpret_cast<RGB*>( m_ImageColor_8bit.get() );

		for( int i = 0;	 i < size; ++i )
		{
			int index = i;
			if( !isMirrorColor )
				index = mirrorIndex( index, m_WidthImageColor );

			if( isAlignDepthColor )
			{
				// ratio between image width difference to translate index between image spaces
				int ratio = m_WidthImageColor / m_WidthImageDepthAndUser; 
				int x = ( index % m_WidthImageColor ) / ratio;
				int y = ( index / m_WidthImageColor ) / ratio;

				// calculated index mapped from colorIndexSpace to depthIndexSpace
				int mappedIndex = m_ColorCoords[x + y * m_WidthImageDepthAndUser];

				if( !isMirrorColor )
					mappedIndex = mirrorIndex( mappedIndex, m_WidthImageColor );

				if( mappedIndex < size && mappedIndex >= 0 )
				{
					pDestination[index].b = pSource[mappedIndex].b;
					pDestination[index].g = pSource[mappedIndex].g;
					pDestination[index].r = pSource[mappedIndex].r;
				}
			}
			else
			{
				pDestination[index].b = pSource[i].b;
				pDestination[index].g = pSource[i].g;
				pDestination[index].r = pSource[i].r;
			}
		}
	}
	else
	{
		_2REAL_LOG( error ) << "_2Real: Device " << m_Name << " skipping frame. No Image data"
							<< "in processColorImageEvent()::LockedRect" << std::endl;
	}
	pTexture->UnlockRect( 0 );

	m_NuiSensor->NuiImageStreamReleaseFrame( m_HandleColorStream, &imageFrame );
	m_NotificationNewColorImageData.notify_all();
}

void WSDKDevice::processDepthImageEvent()
{
	if( m_IsDeletingDevice )
		return;

	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = m_NuiSensor->NuiImageStreamGetNextFrame( m_HandleDepthStream, 0, &imageFrame );
	if( FAILED( hr ) )
	{
		_2REAL_LOG( error ) << "_2Real: Device " << m_Name << " skipping frame. Error while"
							<< "trying to fetch depth data..." << std::endl;
		return;
	}

	boost::mutex::scoped_lock lock( m_MutexDepth );
	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	
	int size = m_WidthImageDepthAndUser * m_HeightImageDepthAndUser; //get number of pixels
	uint16_t* source = reinterpret_cast<uint16_t*>( LockedRect.pBits );
	int colors = 8;	// max users for MS SDK
	float norm16to8 = NUI_IMAGE_DEPTH_MAXIMUM / 255;

	bool isMirrorDepth = isFlagEnabled( DFLAG_MIRROR_DEPTH );
	bool isAlignDepthColor = isFlagEnabled( DFLAG_ALIGN_COLOR_DEPTH );
	bool isDepthOnly = isFlagEnabled( DFLAG_DEPTH_ONLY );
	bool isMirrorUser = isFlagEnabled( DFLAG_MIRROR_USER );

	// per pixel processing of depth buffer
	for( int i = 0;	 i < size; ++i )
	{
		int index = i;
		if( !isMirrorDepth )
			index = mirrorIndex( index, m_WidthImageDepthAndUser );
		
		// fetch depth information
		m_ImageDepth_16bit[index] = NuiDepthPixelToDepth( source[i] );
		m_ImageDepth_8bit[index] = uchar( m_ImageDepth_16bit[index] / norm16to8 );
		
		// write mapped color coords to original index i
		if( isAlignDepthColor )
			setMappedColorCoords( i, source, &imageFrame.ViewArea );

		// processing user image
		if( !isDepthOnly )
		{
			//mirror capability of user image
			if( !isMirrorUser )
				index = mirrorIndex( i, m_WidthImageDepthAndUser );
			else
				index = i;

			//getting first 3 low bits out of 16bit buffer for user id
			m_ImageUser_8bit[index] = (uchar)NuiDepthPixelToPlayerIndex( (USHORT)source[i] );
			
			// resetting values before
			m_ImageColoredUser_8bit[index*3] = 0;
			m_ImageColoredUser_8bit[index*3+1] = 0;
			m_ImageColoredUser_8bit[index*3+2] = 0;

			// skip indices with no user information
			if( m_ImageUser_8bit[index]%colors != 0 )
			{			
				m_ImageColoredUser_8bit[index*3] = Colors[m_ImageUser_8bit[index]%colors][0];
				m_ImageColoredUser_8bit[index*3+1] = Colors[m_ImageUser_8bit[index]%colors][1];
				m_ImageColoredUser_8bit[index*3+2] = Colors[m_ImageUser_8bit[index]%colors][2];
			}
		}		
	}
	//remove lock -> 2 imageframes buffer
	//locking both frame buffer will cause an exception
	pTexture->UnlockRect( 0 );
	m_NuiSensor->NuiImageStreamReleaseFrame( m_HandleDepthStream, &imageFrame );
	m_NotificationNewDepthImageData.notify_all();
}

void WSDKDevice::setMappedColorCoords( const uint32_t index, uint16_t* depthSource, const NUI_IMAGE_VIEW_AREA* vArea )
{
	// split index up in column-index (xDepth) and row-index (yDepth)
	uint32_t xDepth = index % m_WidthImageDepthAndUser;
	uint32_t yDepth = index / m_WidthImageDepthAndUser;
	LONG xMappedColor = 0;
	LONG yMappedColor = 0;

	// specifies color-source-index for a color index x translated to depthSpace
	// xDepth/yDepth can be used to change index of source-imagebuffer -> mapping
	// Note: this does not return position where to write pixel information but rather where to find!!!
	HRESULT hr = NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
		m_Configuration.m_ImageResColor.WSDKResType,
		m_Configuration.m_ImageResDepth.WSDKResType,
		vArea,
		xDepth,
		yDepth,
		depthSource[index],
		&xMappedColor,
		&yMappedColor );

	if( hr != E_INVALIDARG && hr != E_POINTER )
	{
		if( xMappedColor >= 0 && xMappedColor < m_WidthImageColor &&
			yMappedColor >= 0 && yMappedColor < m_HeightImageColor )
			m_ColorCoords[index] = xMappedColor + yMappedColor * m_WidthImageColor;
	}
	else
	{
		_2REAL_LOG( error ) << "_2Real: setMappedColorCoords() Error, NUIGETCOLORPIXELCOORDINATESFROMDEPTHPIXELATRESOLULTION could not be called!" << std::endl;
		m_ColorCoords[index] = 0;
	}
}

void WSDKDevice::processSkeletonEvent()
{
	if( m_IsDeletingDevice )
		return;

	NUI_SKELETON_FRAME nuiFrame;
	
	if( FAILED( m_NuiSensor->NuiSkeletonGetNextFrame( 0, &nuiFrame ) ) )
	{
		_2REAL_LOG( error ) << "_2Real: Device " << m_Name << " skipping frame. Error while trying to fetch skeleton data..." << std::endl;
		return;
	}

	//tracked skeletons available?
	bool detectedSkeletons = false;
	for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
	{
		if( nuiFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
			detectedSkeletons = true;
	}

	boost::mutex::scoped_lock lock(m_MutexUser);
	m_Users.clear();

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
}

_2RealTrackedJoint_sptr WSDKDevice::createJointFromNUI( _2RealJointType type, _NUI_SKELETON_POSITION_INDEX nuiType, const NUI_SKELETON_DATA& data, const NUI_SKELETON_BONE_ORIENTATION* nuiOrientation )
{
	_2RealVector3f screenPos;
	_2RealVector3f worldPos;
	
	// Mirror skeleton joints when user image is mirrored
	Vector4 position = data.SkeletonPositions[nuiType];
	if( !isFlagEnabled( DFLAG_MIRROR_USER ) )
		position.x = -position.x;
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

void WSDKDevice::getUsers( bool waitForNewData, _2RealTrackedUserVector& out ) const
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{	
		boost::mutex::scoped_lock lock( m_MutexFetchUser );
		m_NotificationNewUserdata.wait( lock );
	}

	out.clear(); // ensure writing to "clean" vector
	//prevent changing data fetching thread to change vector while copying
	boost::mutex::scoped_lock lock1( m_MutexUser );
	out = m_Users;
}

unsigned int WSDKDevice::getNumberOfUsers() const
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
		m_NuiSensor->NuiCameraElevationSetAngle( angle );
		return true;
	}
	return false;
}

int WSDKDevice::getMotorAngle() const
{
	LONG tmp;
	m_NuiSensor->NuiCameraElevationGetAngle( &tmp );
	return tmp;
}

void WSDKDevice::shutdown()
{
	if( m_isDeviceShutDown ) return;

	m_IsDeletingDevice = 1;
	m_isDeviceShutDown = true; //needs to be set before stop()
	stop( true );

	if( m_NuiSensor )
	{
		m_NuiSensor->NuiShutdown();
		m_NuiSensor->Release();
		m_NuiSensor = nullptr;
	}
}

void WSDKDevice::start()
{
	if( m_isDeviceStarted ) return;

	// initializing
	_2REAL_LOG( info ) << "_2Real: Initialize device: " << m_DeviceID << " ...";

	HRESULT status = 0;
	if( FAILED( status = m_NuiSensor->NuiInitialize( m_Configuration.m_GeneratorsWSDK ) ) )
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

void WSDKDevice::stop( const bool msgThread )
{
	// i.e called by shutdown() stop thread thingys too
	if( m_isDeviceShutDown == true && msgThread ) 
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

const WSDKDeviceConfiguration& WSDKDevice::getDeviceConfiguration() const
{
	return m_Configuration;
}

WSDKDeviceConfiguration& WSDKDevice::getDeviceConfiguration()
{
	return m_Configuration;
}


bool WSDKDevice::isFlagEnabled( WSDKDeviceFlag flags ) const
{
	return ( m_DeviceFlags & flags ) != 0;
}

void WSDKDevice::setFlag( const WSDKDeviceFlag flags, const bool setEnabled )
{
	if( ( m_DeviceFlags & flags ) != (uint8_t)setEnabled )
		m_DeviceFlags ^= flags;
}

uint32_t WSDKDevice::mirrorIndex( const uint32_t index, const uint32_t imageWidth )
{
	return (( ( index / imageWidth + 1 ) *  imageWidth ) - ( index % imageWidth )) - 1;
}

}
#endif