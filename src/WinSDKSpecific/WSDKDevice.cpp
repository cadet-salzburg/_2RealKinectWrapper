#ifdef TARGET_MSKINECTSDK
#include "_2RealConfig.h"
#include "WinSDKSpecific/WSDKDevice.h"
#include <sstream>
#include "_2RealTypes.h"
#include "_2RealTrackedUser.h"


namespace _2Real
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
	m_IsDeletingDevice = 1;
	SetEvent( m_EventStopThread );
	WaitForSingleObject( m_HandleThread, INFINITE );

	CloseHandle( m_EventStopThread );
	CloseHandle( m_EventColorImage );
	CloseHandle( m_EventDepthImage );
	CloseHandle( m_EventSkeletonData );
	CloseHandle( m_HandleThread );

	//shutting down + destroy of instance
	if(m_pNuiSensor)
	{
		m_pNuiSensor->NuiShutdown();
	}
	if ( m_pNuiSensor )
    {
        m_pNuiSensor->Release();
        m_pNuiSensor = NULL;
    }

	if( m_ImageDepth_16bit )
		delete [] m_ImageDepth_16bit;
	if( m_ImageColor_8bit )
		delete [] m_ImageColor_8bit;
	if( m_ImageUser_8bit )
		delete [] m_ImageUser_8bit;
	if( m_ImageDepth_8bit )
		delete [] m_ImageDepth_8bit;
	if( m_ImageColoredUser_8bit )
		delete [] m_ImageColoredUser_8bit;
}


WSDKDevice::WSDKDevice( INuiSensor* devicePtr, const uint32_t configSensor, const uint32_t configImage, const std::string& name )
	: m_pNuiSensor( devicePtr ),
	  m_name( name ),
	  m_EventColorImage( CreateEvent( NULL, TRUE, FALSE, NULL ) ), //creating event handles
	  m_EventDepthImage( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	  m_EventSkeletonData( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	  m_EventStopThread( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	  m_HandleDepthStream( NULL ),
	  m_HandleColorStream( NULL ),
	  m_HandleThread( NULL ),
	  m_ImageColor_8bit( NULL ),
	  m_ImageDepth_8bit( NULL ),
	  m_ImageUser_8bit( NULL ),
	  m_ImageColoredUser_8bit( NULL ),
	  m_ImageDepth_16bit( NULL ),
	  m_WidthImageDepthAndUser( 0 ),
	  m_WidthImageColor( 0 ),
	  m_HeightImageDepthAndUser( 0 ),
	  m_HeightImageColor( 0 ),
	  m_IsDepthOnly( 1 ),
	  m_IsMirroringColor( 0 ),
	  m_IsMirroringDepth( 0 ),
	  m_IsMirroringUser( 0 ),
	  m_IsDeletingDevice( 0 )
{
	uint32_t config = 0;
	//setting NUI config flags depending on _2real-flags
	if( configSensor & COLORIMAGE )
		config |= NUI_INITIALIZE_FLAG_USES_COLOR;

	if( configSensor & USERIMAGE )
	{
		config |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
		config |= NUI_INITIALIZE_FLAG_USES_SKELETON;
	}
	else if( configSensor & DEPTHIMAGE )
		config |= NUI_INITIALIZE_FLAG_USES_DEPTH;

	if( configSensor & INFRAREDIMAGE )
		_2REAL_LOG(info) << "_2Real: Infrared image capability is not supported by win sdk yet!" << std::endl;
	

	int deviceID = devicePtr->NuiInstanceIndex();
	//delete block if supporting player indices + skeleton on other sensors ------>
	//player indices only available on default device 0
	//no player ids, no skeleton
	//enabling depth sensor data
	
	if( deviceID>0 && ( config & NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX ) )
	{
		_2REAL_LOG(info) << "_2Real: Disabling user image and skeleton on device: " << deviceID << " due it is not supported..." << std::endl;
		config &= ~NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
		config &= ~NUI_INITIALIZE_FLAG_USES_SKELETON;
		_2REAL_LOG(info) << "_2Real: Enabling depth-sensor on device: " << deviceID << " ..." << std::endl;
		config |= NUI_INITIALIZE_FLAG_USES_DEPTH;
	}
	

	//initializing
	HRESULT status = 0;
	if( FAILED( status = devicePtr->NuiInitialize( config ) ) )
	{ 
		throwError( "_2Real: Error when trying to initialize device");
	}

	//opening NUI streams depending on config flags------------------------------->
	//COLOR STREAM---------------------------------------------------------------->
	NUI_IMAGE_RESOLUTION imageRes;
	if( configSensor & COLORIMAGE )
	{
		uint32_t& width = const_cast<uint32_t&>( m_WidthImageColor );
		uint32_t& height = const_cast<uint32_t&>( m_HeightImageColor );

		if( configImage == IMAGE_CONFIG_DEFAULT )
		{
			imageRes = NUI_IMAGE_RESOLUTION_640x480;
			width = 640;
			height = 480;
		}
		else if( configImage & IMAGE_COLOR_1280X960 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_1280x960;
			width = 1280; 
			height = 960;
		}
		else if( configImage & IMAGE_COLOR_640X480 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_640x480;
			width = 640;
			height = 480;
		}
		else if( configImage & IMAGE_COLOR_320X240 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_320x240;
			width = 320;
			height = 240;
		}

		_2REAL_LOG(info) << "_2Real: setting RGB resolution to " << width << " x " << height << " for device: " << m_name << std::endl;
		
		//allocating image buffer
		m_ImageColor_8bit = new uchar[m_WidthImageColor*m_HeightImageColor*3]();

		_2REAL_LOG(info) << "_2Real: Starting image stream on device: " << m_name << std::endl;;
		//starting stream
		status = devicePtr->NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR, imageRes, 0, 2, m_EventColorImage, &m_HandleColorStream );
		
		if( FAILED( status ) )
		{
			throwError( ( std::string( "_2Real: Error when trying to open image stream on device: " ).append( m_name.c_str() ).append( " Try another resolution for sensor!!! ") ).c_str() );
		}
		_2REAL_LOG(info) << "OK" << std::endl;
	}

	//DEPTH, USER, SKELETON------------------------------------------------------->
	if( config & NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX )
	{
		uint32_t& width = const_cast<uint32_t&>( m_WidthImageDepthAndUser );
		uint32_t& height = const_cast<uint32_t&>( m_HeightImageDepthAndUser );
		
		if( configImage == IMAGE_CONFIG_DEFAULT)
		{
			imageRes = NUI_IMAGE_RESOLUTION_640x480;
			width = 640;
			height = 480;
		}
		else if( configImage & IMAGE_USER_DEPTH_640X480 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_640x480;
			width = 640;
			height = 480;
		}
		else if( configImage & IMAGE_USER_DEPTH_320X240 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_320x240;
			width = 320;
			height = 240;
		}
		else if( configImage & IMAGE_USER_DEPTH_80X60 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_80x60;
			width = 80;
			height = 60;
		}

		m_IsDepthOnly = false;

		m_ImageDepth_8bit = new uchar[m_WidthImageDepthAndUser*m_HeightImageDepthAndUser]();
		m_ImageDepth_16bit = new uint16_t[m_WidthImageDepthAndUser*m_HeightImageDepthAndUser]();
		m_ImageUser_8bit = new uchar[m_WidthImageDepthAndUser*m_HeightImageDepthAndUser]();
		m_ImageColoredUser_8bit = new uchar[m_WidthImageDepthAndUser*m_HeightImageDepthAndUser*3]();
		
		_2REAL_LOG(info) << "_2Real: setting depth, user resolution to " << width << " x " << height << " for device: " << m_name << std::endl;
		_2REAL_LOG(info) << "_2Real: Starting depth, user stream on device: " << m_name << " ...";
		//starting depth stream
		status = devicePtr->NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, imageRes, 
												0, 2, m_EventDepthImage, &m_HandleDepthStream );
		if( FAILED( status ) )
		{
			throwError( ( std::string( "_2Real: Error when trying to open depth, user stream on device: " ).append( m_name.c_str() ).append( " Try another resolution for sensor!!! ") ).c_str() );
		}
		_2REAL_LOG(info) << "OK" << std::endl;

		//skeleton tracking
		_2REAL_LOG(info) << "_2Real: Starting skeleton tracking on device: " << m_name << " ...";
		status = devicePtr->NuiSkeletonTrackingEnable( m_EventSkeletonData, 0 );
		if( FAILED( status ) )
		{
			throwError( ( std::string( "_2Real: Error when trying to enable skeleton tracking on device: " ).append( m_name.c_str() ) ).c_str() );
		}
		_2REAL_LOG(info) << "OK" << std::endl;
	}

	//DEPTH ONLY ----------------------------------------------------------------->
	else if( config & NUI_INITIALIZE_FLAG_USES_DEPTH )
	{
		uint32_t& width = const_cast<uint32_t&>( m_WidthImageDepthAndUser );
		uint32_t& height = const_cast<uint32_t&>( m_HeightImageDepthAndUser );

		if( configImage == IMAGE_CONFIG_DEFAULT)
		{
			imageRes = NUI_IMAGE_RESOLUTION_640x480;
			width = 640;
			height = 480;
		}
		else if( configImage & IMAGE_USER_DEPTH_640X480 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_640x480;
			width = 640;
			height = 480;
		}
		else if( configImage & IMAGE_USER_DEPTH_320X240 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_320x240;
			width = 320;
			height = 240;
		}
		else if( configImage & IMAGE_USER_DEPTH_80X60 )
		{
			imageRes = NUI_IMAGE_RESOLUTION_80x60;
			width = 80;
			height = 60;
		}
		
		m_ImageDepth_8bit = new uchar[m_WidthImageDepthAndUser*m_HeightImageDepthAndUser]();
		m_ImageDepth_16bit = new uint16_t[m_WidthImageDepthAndUser*m_HeightImageDepthAndUser]();
		_2REAL_LOG(info) << "_2Real: setting depth, user resolution to " << width << " x " << height << " for device: " << m_name << std::endl;
		_2REAL_LOG(info) << "_2Real: Starting depth stream on device: " << m_name << " ...";
		status = devicePtr->NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH, imageRes, 
												0, 2, m_EventDepthImage, &m_HandleDepthStream );
		if( FAILED( status ) )
		{
			throwError( ( std::string( "_2Real: Error when trying to open depth stream on device: " ).append( m_name.c_str() ).append( " Try another resolution for sensor!!! ") ).c_str() );
		}
		_2REAL_LOG(info) << "OK" << std::endl;
	}
	
	// set mirror flags default to true
	m_IsMirroringColor = m_IsMirroringDepth = m_IsMirroringUser = 1;
	

	_2REAL_LOG(info) << "_2Real: Starting own thread for fetching sensor data...";
	m_HandleThread = CreateThread( NULL, 0, ThreadEventsFetcher, this, NULL, NULL );
	if( m_HandleThread )
	{
		_2REAL_LOG(info) << "OK" << std::endl;
	}
	else
	{
		_2REAL_LOG(error) << "Starting thread failed" << std::endl;
	}

	_2REAL_LOG(info) << ( std::string( "Initialized device >>" ) += name ).c_str() << "<< successfully..." << std::endl;
}


DWORD WINAPI WSDKDevice::ThreadEventsFetcher( LPVOID pParam )
{
	WSDKDevice* pThis = static_cast<WSDKDevice*>( pParam );

	//list of events being waited
	HANDLE events[] = { pThis->m_EventStopThread,			//eventIndex 0
						pThis->m_EventColorImage,			//eventIndex 1
						pThis->m_EventDepthImage,			//eventIndex 2
						pThis->m_EventSkeletonData };		//eventIndex 3

	_2REAL_LOG(info) << "\nStarted thread id: " << int( pThis->m_HandleThread ) << std::endl;

	int eventIndex = 0;

	while( true )
	{
		//waiting for events
		eventIndex = WaitForMultipleObjects( 4, events, false, INFINITE );

		switch( eventIndex )
		{
		case 0:			//exit thread
			return 0;
		case 1:			//process image event
			pThis->ProcessColorImageEvent();
			break;
		case 2:			//process depth event
			pThis->ProcessDepthImageEvent();
			break;
		case 3:			//process skeleton event
			pThis->ProcessSkeletonEvent();
			break;
		}
	}
	return 0;
}

void WSDKDevice::ProcessColorImageEvent()
{	
	NUI_IMAGE_FRAME imageFrame;

	if( m_IsDeletingDevice )
		return;

    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_HandleColorStream, 0, &imageFrame );

    if ( FAILED( hr ) )
    {
		_2REAL_LOG(error) << "_2Real: Device " << m_name << " skipping frame. Error while trying to fetch color data..." << std::endl;
        return;
    }

	m_MutexImage.lock();
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
		RGB* pDestination = reinterpret_cast<RGB*>( m_ImageColor_8bit );

		for( int i = 0;	 i < size; ++i )//loop enrolled: 10 iterations in 1 step
		{
			int index = 0;
			if( !m_IsMirroringColor )
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
	m_MutexImage.unlock();
	m_NotificationNewColorImageData.notify_all();
}

void WSDKDevice::ProcessDepthImageEvent()
{
	if( m_IsDeletingDevice )
		return;

	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_HandleDepthStream, 0, &imageFrame );
	if(FAILED(hr))
	{
		_2REAL_LOG(error) << "_2Real: Device " << m_name << " skipping frame. Error while trying to fetch depth data..." << std::endl;
		return;
	}
	m_MutexDepth.lock();
	INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    
	int size = m_WidthImageDepthAndUser * m_HeightImageDepthAndUser; //get number of pixels
	uint32_t width = m_WidthImageDepthAndUser;
	uchar* destinationDepth = m_ImageDepth_8bit;
	uint16_t* destinationDepth_16 = m_ImageDepth_16bit;
	uchar* destinationUser = m_ImageUser_8bit;
	uchar* destinationColorUser = m_ImageColoredUser_8bit;

	uint16_t* source = reinterpret_cast<uint16_t*>( LockedRect.pBits );
	int colors = 8;	// max users for MS SDK
	for( int i = 0;	 i < size; ++i ) //loop enrolled: 10 iterations in 1 step
	{
		int index = 0;

		//mirror capability
		if( !m_IsMirroringDepth )
			index = (( ( i / width + 1 ) *  width ) - ( i % width )) - 1;
		else
			index = i;

		//writing depth only or user and depth image
		if( m_IsDepthOnly )
		{
			//12 low bits used of 16bit buffer
			destinationDepth_16[index] = source[i];
			destinationDepth[index] = uchar( ( (float)source[i] / _2REAL_WSDK_DEPTH_NORMALIZATION_16_TO_8 ) * 255 );
		}
		else
		{
			//using 12 high bits out of 16bit buffer
			destinationDepth_16[index] = source[i] >> 3;
			destinationDepth[index] = uchar( ( (float)( source[i] >> 3 ) / _2REAL_WSDK_DEPTH_NORMALIZATION_16_TO_8 ) * 255 ); 

			//mirror capability
			if( !m_IsMirroringUser )
				index = (( ( i / width + 1 ) *  width ) - ( i % width )) - 1;
			else
				index = i;

			//getting first 3 low bits out of 16bit buffer
			destinationUser[index] = source[i] & 0x0007;
			destinationColorUser[index*3] = 0;
			destinationColorUser[index*3+1] = 0;
			destinationColorUser[index*3+2] = 0;

			if(destinationUser[index]%colors != 0)
			{			
				destinationColorUser[index*3] = Colors[destinationUser[index]%colors][0];
				destinationColorUser[index*3+1] = Colors[destinationUser[index]%colors][1];
				destinationColorUser[index*3+2] = Colors[destinationUser[index]%colors][2];
			}
		}		
		
	}
	//remove lock -> 2 imageframes buffer
	//locking both framebuffer will cause an exception
	pTexture->UnlockRect( 0 );
	m_pNuiSensor->NuiImageStreamReleaseFrame( m_HandleDepthStream, &imageFrame );
	
	m_MutexDepth.unlock();
	m_NotificationNewDepthImageData.notify_all();
}

_2RealTrackedJoint WSDKDevice::GetJoint( _2RealJointType type, _NUI_SKELETON_POSITION_INDEX nuiType, const NUI_SKELETON_DATA& data )
{
	_2RealVector2f screenPos;
	_2RealVector3f worldPos;
	
	// Mirror skeleton joints when user image is mirrored
	if(!m_IsMirroringUser)
	{
		Vector4 tmp = data.SkeletonPositions[nuiType];
		tmp.x = (FLOAT)1.0 -tmp.x;
		NuiTransformSkeletonToDepthImage( tmp, (FLOAT*)&screenPos.x, (FLOAT*)&screenPos.y );
	}
	else
		NuiTransformSkeletonToDepthImage( data.SkeletonPositions[nuiType], (FLOAT*)&screenPos.x, (FLOAT*)&screenPos.y );

	worldPos.x = data.SkeletonPositions[nuiType].x;
	worldPos.y = data.SkeletonPositions[nuiType].y;
	worldPos.z = data.SkeletonPositions[nuiType].z;
	
	/* screen pos multiplied with depth image resolution */
	screenPos.x *= 2;
	screenPos.y *= 2;
	
	return _2RealTrackedJoint( type, screenPos, worldPos, _2RealMatrix3x3(),  _2RealConfidence(1, 0));		// orientation cofidence set to 0 because it is not yet supported
}

void WSDKDevice::ProcessSkeletonEvent()
{
	if( m_IsDeletingDevice )
		return;

	m_MutexUser.lock();
	
	NUI_SKELETON_FRAME nuiFrame;
	m_Users.clear();

	if( FAILED( m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &nuiFrame ) ) )
	{
		_2REAL_LOG(error) << "_2Real: Device " << m_name << " skipping frame. Error while trying to fetch skeleton data..." << std::endl;
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
		m_MutexUser.unlock();
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

	_2RealTrackedUser* user = NULL;
	//get or create user
	for( int i = 0; i < NUI_SKELETON_COUNT; ++i )
	{
		if( nuiFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
		{
			m_Users.push_back( _2RealTrackedUser( nuiFrame.SkeletonData[i].dwTrackingID ) );
			user = &m_Users.back();
		}
		else
			continue;

		//setting/updating joints
		user->setJoint( JOINT_HEAD, GetJoint( JOINT_HEAD, NUI_SKELETON_POSITION_HEAD, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_NECK, GetJoint( JOINT_NECK, NUI_SKELETON_POSITION_SHOULDER_CENTER, nuiFrame.SkeletonData[i] ) ); 
		user->setJoint( JOINT_TORSO, GetJoint( JOINT_TORSO, NUI_SKELETON_POSITION_SPINE, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_WAIST, GetJoint( JOINT_WAIST, NUI_SKELETON_POSITION_HIP_CENTER, nuiFrame.SkeletonData[i] ) );

		user->setJoint( JOINT_LEFT_COLLAR, _2RealTrackedJoint( JOINT_LEFT_COLLAR, _2RealVector2f(), _2RealVector3f(), _2RealMatrix3x3(), _2RealConfidence() ) );
		user->setJoint( JOINT_LEFT_SHOULDER, GetJoint( JOINT_LEFT_SHOULDER, NUI_SKELETON_POSITION_SHOULDER_LEFT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_LEFT_ELBOW, GetJoint( JOINT_LEFT_ELBOW, NUI_SKELETON_POSITION_ELBOW_LEFT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_LEFT_WRIST, GetJoint( JOINT_LEFT_WRIST, NUI_SKELETON_POSITION_WRIST_LEFT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_LEFT_HAND, GetJoint( JOINT_LEFT_HAND, NUI_SKELETON_POSITION_HAND_LEFT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_LEFT_FINGERTIP, _2RealTrackedJoint( JOINT_LEFT_FINGERTIP, _2RealVector2f(), _2RealVector3f(), _2RealMatrix3x3(), _2RealConfidence() ) );

		user->setJoint( JOINT_RIGHT_COLLAR, _2RealTrackedJoint( JOINT_LEFT_COLLAR, _2RealVector2f(), _2RealVector3f(), _2RealMatrix3x3(), _2RealConfidence() ) );
		user->setJoint( JOINT_RIGHT_SHOULDER, GetJoint( JOINT_RIGHT_SHOULDER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_RIGHT_ELBOW, GetJoint( JOINT_RIGHT_ELBOW, NUI_SKELETON_POSITION_ELBOW_RIGHT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_RIGHT_WRIST, GetJoint( JOINT_RIGHT_WRIST, NUI_SKELETON_POSITION_WRIST_RIGHT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_RIGHT_HAND, GetJoint( JOINT_RIGHT_HAND, NUI_SKELETON_POSITION_HAND_RIGHT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_RIGHT_FINGERTIP, _2RealTrackedJoint( JOINT_RIGHT_FINGERTIP, _2RealVector2f(), _2RealVector3f(), _2RealMatrix3x3(), _2RealConfidence() ) );
		
		user->setJoint( JOINT_LEFT_HIP, GetJoint( JOINT_LEFT_HIP, NUI_SKELETON_POSITION_HIP_LEFT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_LEFT_KNEE, GetJoint( JOINT_LEFT_KNEE, NUI_SKELETON_POSITION_KNEE_LEFT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_LEFT_ANKLE, GetJoint( JOINT_LEFT_ANKLE, NUI_SKELETON_POSITION_ANKLE_LEFT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_LEFT_FOOT, GetJoint( JOINT_LEFT_FOOT, NUI_SKELETON_POSITION_FOOT_LEFT, nuiFrame.SkeletonData[i] ) );

		user->setJoint( JOINT_RIGHT_HIP, GetJoint( JOINT_RIGHT_HIP, NUI_SKELETON_POSITION_HIP_RIGHT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_RIGHT_KNEE, GetJoint( JOINT_RIGHT_KNEE, NUI_SKELETON_POSITION_KNEE_RIGHT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_RIGHT_ANKLE, GetJoint( JOINT_RIGHT_ANKLE, NUI_SKELETON_POSITION_ANKLE_RIGHT, nuiFrame.SkeletonData[i] ) );
		user->setJoint( JOINT_RIGHT_FOOT, GetJoint( JOINT_RIGHT_FOOT, NUI_SKELETON_POSITION_FOOT_RIGHT, nuiFrame.SkeletonData[i] ) );
	}
	m_MutexUser.unlock();
	m_NotificationNewUserdata.notify_all();
}

void WSDKDevice::SetMirroringColor( const bool flag )
{
	m_IsMirroringColor = flag;
}

void WSDKDevice::SetMirroringDetph( const bool flag )
{
	m_IsMirroringDepth = flag;
}

void WSDKDevice::SetMirroringUser( const bool flag )
{
	m_IsMirroringUser = flag;
}

bool WSDKDevice::IsMirroingColor() const
{
	return m_IsMirroringColor;
}

bool WSDKDevice::IsMirroingDepth() const
{
	return m_IsMirroringDepth;
}

bool WSDKDevice::IsMirroingUser() const
{
	return m_IsMirroringUser;
}

_2RealTrackedUserVector& WSDKDevice::GetUsers( bool waitForNewData )
{
	boost::unique_lock<boost::mutex> lock( m_MutexFetchUser );
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		m_NotificationNewUserdata.wait( lock );
	}

	//prevent changing data fetching thread to change vector while copying
	m_MutexUser.lock();
	m_UsersShared.clear();
	m_UsersShared.insert( m_UsersShared.begin(), m_Users.begin(), m_Users.end() );//copy data to vector that can be shared with app
	m_MutexUser.unlock();
	lock.unlock();

	return m_UsersShared;
}

uchar* WSDKDevice::GetColorImageBuffer( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::unique_lock<boost::mutex> lock( m_MutexFetchColorImage );
		m_NotificationNewColorImageData.wait( lock );
		lock.unlock();
	}
	return m_ImageColor_8bit;
}

uchar* WSDKDevice::GetDepthImageBuffer( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::unique_lock<boost::mutex> lock( m_MutexFetchDepthImage );
		m_NotificationNewDepthImageData.wait( lock );
		lock.unlock();
	}
	return m_ImageDepth_8bit;
}

uchar* WSDKDevice::GetUserImageBuffer( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::unique_lock<boost::mutex> lock( m_MutexFetchDepthImage2 );
		m_NotificationNewDepthImageData.wait( lock );
		lock.unlock();
	}
	return m_ImageUser_8bit;
}


uint16_t* WSDKDevice::GetDepthImageBuffer16Bit( bool waitForNewData )
{
	if( waitForNewData ) //wait for signal for new data of processing thread
	{
		boost::unique_lock<boost::mutex> lock( m_MutexFetchDepthImage3 );
		m_NotificationNewDepthImageData.wait( lock );
		lock.unlock();
	}
	return m_ImageDepth_16bit;
}

uchar* WSDKDevice::GetColoredUserImageBuffer( bool waitForNewData )
{
	if( waitForNewData )
	{
		boost::unique_lock<boost::mutex> lock( m_MutexColoredUser );
		m_NotificationNewDepthImageData.wait( lock );
		lock.unlock();
	}
	return m_ImageColoredUser_8bit;
}

bool WSDKDevice::SetMotorAngle(int angle)
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

int WSDKDevice::GetMotorAngle()
{
	LONG tmp;
	m_pNuiSensor->NuiCameraElevationGetAngle(&tmp);
	return tmp;
}

}
#endif