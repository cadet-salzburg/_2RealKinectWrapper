#ifndef TARGET_MSKINECTSDK
#include "_2RealConfig.h"
#include <OpenNISpecific/OpenNIDevice.h>
#include <iostream>
#include <sstream>

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

	boost::mutex						m_MutexSyncProcessUsers;

OpenNIDevice::OpenNIDevice( const int id, const std::string& name, xn::Context& context, xn::NodeInfo& deviceInfo )
	: m_ID( id ),
	m_DepthImage_8bit( NULL ),
	m_InfraredImage_8bit( NULL ),
	m_UserImage_8bit( NULL ),
	m_UserImageColor_8bit( NULL ),
	m_InstanceName( name ),
	m_IsInitialized( false ),
	m_IsProcessingThread( false ),
	m_Context( context ),
	m_DeviceInfo( deviceInfo ),
	m_IsFetchingData( true ),
	m_CurrentBuffer( 0 ),
	m_TrackedUserArraySize( 10 ),
	m_TrackedUsersArray( NULL )
{
	//output node for configuring generators
	m_OutputModeColor.nFPS = m_OutputModeDepth.nFPS = m_OutputModeInfrared.nFPS = m_OutputModeUser.nFPS = 30; 

	deviceInfo.SetInstanceName( name.c_str() );

	//writing infos
	std::stringstream ss;
	ss << "_2Real: Initialized: " << m_InstanceName << ":\nInfo:\nDescription: "
		<< deviceInfo.GetDescription().strName << "\nCreation-Info: "
		<< deviceInfo.GetCreationInfo();
	_2REAL_LOG(info) << ss << std::endl;
}


OpenNIDevice::~OpenNIDevice(void)
{
	shutdown();
}


void OpenNIDevice::update( void* instance )
{
	OpenNIDevice* devicePtr = static_cast<OpenNIDevice*>( instance );

	while( devicePtr->m_IsProcessingThread )
	{
		int buffer = devicePtr->m_CurrentBuffer; //taking not to buffer for readout

		//updating generators blocking or non-blocking
		/*if( waitAndBlock )
		{
		checkError( m_Context.WaitAndUpdateAll(),
		"Error _2RealImplOpenNI::getUsers() in m_Context::WaitAndUpdateAll\n" );
		}*/

		//only thread 0 updating openni context
		//if( !devicePtr->m_ID )
		
		// FETCHING USER ----------------------------------------------------------------------------->
		m_MutexSyncProcessUsers.lock();
		devicePtr->m_MutexFetchUsers.lock();

		checkError( devicePtr->m_Context.WaitNoneUpdateAll(),
					"_2Real: getUsers() in m_Context::WaitNoneUpdateAll\n" );

		//filling users vector and getting position (world + screen)
		if( devicePtr->GetOpenNIDepthGenerator().IsValid() && devicePtr->GetOpenNIUserGenerator().IsValid() )
		{
			checkError( devicePtr->m_UserGenerator.getTrackedUsers( devicePtr->m_TrackedUsersArray, devicePtr->m_TrackedUserArraySize ),
						"_2Real: getUsers() m_usergenerator::getTrackedUsers\n" );
			checkError( devicePtr->m_DepthGenerator.getUserScreenPositions( devicePtr->m_TrackedUsersArray, devicePtr->m_TrackedUserArraySize ),
						"_2Real: getUsers() m_usergenerator::getscreenposition\n" );
		}
		devicePtr->m_MutexFetchUsers.unlock();

		
		//user
		if( devicePtr->GetOpenNIUserGenerator().IsValid() )
		{
			int colors = 11;	// max users for openni
			int sizeUser = devicePtr->m_UserGenerator.getNrOfUsers();

			for( int u=0; u<sizeUser; ++u )
			{
				devicePtr->m_MutexFetchUsers.lock();
				checkError( devicePtr->m_UserGenerator.getUserData( u, devicePtr->m_UserImage ),
							"_2Real: Error getImageData() type user image\n" );
				devicePtr->m_MutexFetchUsers.unlock();

				_2RealVector2f v2Size = devicePtr->m_UserImage.getFullResolution();

				const uint16_t* source = devicePtr->m_UserImage.getData(); /*source 16bit image data*/
				uint32_t size = int( v2Size.x * v2Size.y );

				//fetching pixel buffer
				for( uint32_t i=0; i < size; ++i )
				{
					devicePtr->m_UserImage_8bit[buffer][i] = (uint8_t)source[i]; //writing "normal" pixel data
					
					devicePtr->m_UserImageColor_8bit[buffer][i*3] = 0;
					devicePtr->m_UserImageColor_8bit[buffer][i*3+1] = 0;
					devicePtr->m_UserImageColor_8bit[buffer][i*3+2] = 0;

					//writing colored user image
					if(source[i]%colors != 0)
					{			
						devicePtr->m_UserImageColor_8bit[buffer][i*3] = Colors[source[i]%colors][0];
						devicePtr->m_UserImageColor_8bit[buffer][i*3+1] = Colors[source[i]%colors][1];
						devicePtr->m_UserImageColor_8bit[buffer][i*3+2] = Colors[source[i]%colors][2];
					}
				}
			}

		}
		m_MutexSyncProcessUsers.unlock();

		// FETCHING IMAGE DATA ----------------------------------------------------------------------->
		//color
		if( devicePtr->GetOpenNIImageGenerator().IsValid() )
		{
			checkError( devicePtr->m_ColorGenerator.getData( devicePtr->m_ColorImage ),
						"_2Real: Error getImageData() type color image\n" );
		}

		//depth
		if( devicePtr->GetOpenNIDepthGenerator().IsValid() )
		{
			checkError( devicePtr->m_DepthGenerator.getData( devicePtr->m_DepthImage ),
						"_2Real: Error getImageData() type depth image\n" );

			_2RealVector2f v2Size = devicePtr->m_DepthImage.getFullResolution();
			convertImage_16_to_8( devicePtr->m_DepthImage.getData(), /*source 16bit image data*/
								  devicePtr->m_DepthImage_8bit[buffer], /*converted 8bit image*/
								  int( v2Size.x * v2Size.y ), /*amount of pixels*/
								  _2REAL_OPENNI_DEPTH_NORMALIZATION_16_TO_8 ); /* highest possible value 3bits reserved of 16bit buffer*/
		}

		//infrared
		if( devicePtr->GetOpenNIInfraredGenertor().IsValid() )
		{
			checkError( devicePtr->m_InfraredGenerator.getData( devicePtr->m_InfraredImage ),
						"_2Real: Error getImageData() type infrared image\n" );

			_2RealVector2f v2Size = devicePtr->m_InfraredImage.getFullResolution();
			convertImage_16_to_8( devicePtr->m_InfraredImage.getData(), /*source 16bit image data*/
								  devicePtr->m_InfraredImage_8bit[buffer], /*converted 8bit image*/
								  int( v2Size.x * v2Size.y ), /*amount of pixels*/
								  255 ); /*no normalization 255 / 255*/
		}

		//at the and swap buffers
		devicePtr->m_CurrentBuffer = !devicePtr->m_CurrentBuffer;
	}
}

void OpenNIDevice::checkError( XnStatus status, std::string error )
{
	if ( status != XN_STATUS_OK )
		throwError( ( error += xnGetStatusString( status ) ).c_str() );	
}

xn::DepthGenerator& OpenNIDevice::GetOpenNIDepthGenerator()
{
	return m_DepthGenerator.m_DepthGenerator;
}

xn::UserGenerator& OpenNIDevice::GetOpenNIUserGenerator()
{
	return m_UserGenerator.m_UserGenerator;
}

xn::ImageGenerator& OpenNIDevice::GetOpenNIImageGenerator()
{
	return m_ColorGenerator.m_ColorGenerator;
}

xn::IRGenerator& OpenNIDevice::GetOpenNIInfraredGenertor()
{
	return m_InfraredGenerator.m_InfraredGenerator;
}

void OpenNIDevice::startupProcessingColorGenerator( xn::NodeInfo& node, const uint32_t configureImages )
{
	//configure image-generator output size
	if( configureImages & IMAGE_COLOR_1280X1024 )
	{
		/*XN_RES_QVGA
		XN_RES_VGA
		XN_RES_SXGA
		XN_RES_UXGA
		XnMapOutputMode Mode;
		pGenerator->GetMapOutputMode(Mode);
		Mode.nXRes = Resolution((XnResolution)XN_RES_UXGA).GetXResolution();
		Mode.nYRes = Resolution((XnResolution)XN_RES_UXGA).GetYResolution();
		XnStatus nRetVal = pGenerator->SetMapOutputMode(Mode);
*/
		m_OutputModeColor.nXRes = xn::Resolution::Resolution((XnResolution)XN_RES_SXGA).GetXResolution();
		m_OutputModeColor.nYRes = xn::Resolution::Resolution((XnResolution)XN_RES_SXGA).GetYResolution();
		m_OutputModeColor.nFPS = 10;
	}
	else if( configureImages & IMAGE_COLOR_640X480 )
	{
		m_OutputModeColor.nXRes = 640;
		m_OutputModeColor.nYRes = 480;
	}
	else if( configureImages & IMAGE_COLOR_320X240 )
	{
		m_OutputModeColor.nXRes = 320;
		m_OutputModeColor.nYRes = 240;
	}
	else //default
	{
		m_OutputModeColor.nXRes = 640;
		m_OutputModeColor.nYRes = 480;
	}

	_2REAL_LOG(info) << "_2Real: Processing color node for device " << m_InstanceName.c_str() << " ...OK" << std::endl;
	_2REAL_LOG(info) << "_2Real: Creating color generator production tree...";

	//creating production tree for image generator
	checkError( m_Context.CreateProductionTree( node, GetOpenNIImageGenerator() ),
				"_2Real: Error when creating production tree for >>image-generator<< \n" );

	_2REAL_LOG(info) << "OK" << std::endl;
	_2REAL_LOG(info) << "_2Real: Configuring output mode for color generator res: " << m_OutputModeColor.nXRes << "x" << m_OutputModeColor.nYRes << " fps: " << m_OutputModeColor.nFPS << std::endl;

	//setting mirror capability as default
	checkError( m_ColorGenerator.setMirroring( true ),	"_2Real: Error when setting mirror capability for >>image-generator<< \n" );

	//setting output mode
	checkError( m_ColorGenerator.setOutputMode( m_OutputModeColor ),
				"_2Real: Error when setting outputmode for >>image-generator<< \n" );

	//register callbacks
	checkError( m_ColorGenerator.registerCallbacks(),
				"_2Real: Error when registering callbacks for >>image-generator<< \n" );
}

void OpenNIDevice::startupProcessingDepthGenerator( xn::NodeInfo& node, const uint32_t configureImages )
{
	//configuring image size
	if ( configureImages & IMAGE_USER_DEPTH_640X480 )
	{
		m_OutputModeDepth.nXRes = 640;
		m_OutputModeDepth.nYRes = 480;
	}
	else if ( configureImages & IMAGE_USER_DEPTH_320X240 )
	{
		m_OutputModeDepth.nXRes = 320;
		m_OutputModeDepth.nYRes = 240;
	}
	else if ( configureImages & IMAGE_USER_DEPTH_80X60 )
	{
		m_OutputModeDepth.nXRes = 80;
		m_OutputModeDepth.nYRes = 60;
	}
	else //default
	{
		m_OutputModeDepth.nXRes = 640;
		m_OutputModeDepth.nYRes = 480;
	}

	
	_2REAL_LOG(info) << "_2Real: Processing depth node for device " << m_InstanceName.c_str() << " ...OK" << std::endl;
	_2REAL_LOG(info) << "_2Real: Creating depth generator production tree...";

	//creating production tree for depthGenerator
	checkError( m_Context.CreateProductionTree( node, GetOpenNIDepthGenerator() ),
				"_2Real: Error when creating production tree for >>depth-generator<< \n" );

	_2REAL_LOG(info) << "OK" << std::endl;

	//allocating memory for image buffers
	m_DepthImage_8bit = new unsigned char*[2]();
	m_DepthImage_8bit[0] = new unsigned char[m_OutputModeDepth.nXRes*m_OutputModeDepth.nYRes]();
	m_DepthImage_8bit[1] = new unsigned char[m_OutputModeDepth.nXRes*m_OutputModeDepth.nYRes]();

	_2REAL_LOG(info) << "_2Real: Configuring output mode for depth generator res: " << m_OutputModeDepth.nXRes << "x" << m_OutputModeDepth.nYRes << " fps: " << m_OutputModeDepth.nFPS << std::endl;

	// mirror camera
	checkError( m_DepthGenerator.setMirroring( true ),	"_2Real: Error when setting mirror capability for >>depth-generator<< \n" );
	

	//setting output mode
	checkError( m_DepthGenerator.setOutputMode( m_OutputModeDepth ),
				"_2Real: Error when setting outputmode for >>depth-generator<< \n" );

	//fetching depth generator instance
	checkError( m_DepthGenerator.registerCallbacks(),
				"_2Real: Error when registering callbacks for >>depth-generator<< \n" );
}

void OpenNIDevice::startupProcessingInfraredGenerator( xn::NodeInfo& node, const uint32_t configureImages )
{
	if ( configureImages & IMAGE_INFRARED_640X480 )
	{
		m_OutputModeInfrared.nXRes = 640;
		m_OutputModeInfrared.nYRes = 480;
	}
	else if ( configureImages & IMAGE_INFRARED_320X240 )
	{
		m_OutputModeInfrared.nXRes = 320;
		m_OutputModeInfrared.nYRes = 240;
	}
	else //default
	{
		m_OutputModeInfrared.nXRes = 640;
		m_OutputModeInfrared.nYRes = 480;
	}

	_2REAL_LOG(info) << "_2Real: Processing infrared node for device " << m_InstanceName.c_str() << " ...OK" << std::endl;
	_2REAL_LOG(info) << "_2Real: Creating infrared generator production tree...";

	//creating production tree for infrared generator
	checkError( m_Context.CreateProductionTree( node, GetOpenNIInfraredGenertor() ),
				"_2Real: Error when creating production tree for >>infrared-generator<< \n" );

	_2REAL_LOG(info) << "OK" << std::endl;

	//allocating pixel buffer
	m_InfraredImage_8bit = new unsigned char*[2]();
	m_InfraredImage_8bit[0] = new unsigned char[m_OutputModeInfrared.nXRes*m_OutputModeInfrared.nYRes*3]();
	m_InfraredImage_8bit[1] = new unsigned char[m_OutputModeInfrared.nXRes*m_OutputModeInfrared.nYRes*3]();

	_2REAL_LOG(info) << "_2Real: Configuring output mode for infrared generator res: " << m_OutputModeInfrared.nXRes << "x" << m_OutputModeInfrared.nYRes << " fps: " << m_OutputModeInfrared.nFPS << std::endl;

	//setting mirror as default
	checkError( m_InfraredGenerator.setMirroring( true ), "_2Real: Error when setting mirror capability for >>infrared-generator<< \n" );

	//setting output mode
	checkError( m_InfraredGenerator.setOutputMode( m_OutputModeInfrared ),
				"_2Real: Error when setting outputmode for >>infrared-generator<< \n" );

	//registering callbacks
	checkError( m_InfraredGenerator.registerCallbacks(),
				"_2Real: Error when registering callbacks for >>infrared-generator<< \n" );
}

void OpenNIDevice::startupProcessingUserGenerator( xn::NodeInfo& node, const uint32_t configureImages )
{
	if ( configureImages & IMAGE_USER_DEPTH_640X480 )
	{
		m_OutputModeUser.nXRes = 640;
		m_OutputModeUser.nYRes = 480;
	}
	else if ( configureImages & IMAGE_USER_DEPTH_320X240 )
	{
		m_OutputModeUser.nXRes = 320;
		m_OutputModeUser.nYRes = 240;
	}
	else if ( configureImages & IMAGE_USER_DEPTH_80X60 )
	{
		m_OutputModeUser.nXRes = 80;
		m_OutputModeUser.nYRes = 60;
	}
	else //default
	{
		m_OutputModeUser.nXRes = 640;
		m_OutputModeUser.nYRes = 480;
	}
	
	_2REAL_LOG(info) << "_2Real: Processing user generator node for device " << m_InstanceName.c_str() << " ...OK" << std::endl;
	_2REAL_LOG(info) << "_2Real: Creating user generator production tree...";

	//creating production tree for user-generator
	checkError( m_Context.CreateProductionTree( node, GetOpenNIUserGenerator() ),
				"_2Real: Error when creating production tree for >>user-generator<< \n" );

	_2REAL_LOG(info) << "OK" << std::endl;
	
	//creating pixel buffer
	m_UserImage_8bit = new unsigned char*[2](); 
	m_UserImageColor_8bit = new unsigned char*[2]();
	m_UserImage_8bit[0] = new unsigned char[m_OutputModeUser.nXRes*m_OutputModeUser.nYRes](); //user image -> ids, () is for default 0 initialization
	m_UserImage_8bit[1] = new unsigned char[m_OutputModeUser.nXRes*m_OutputModeUser.nYRes](); //user image -> ids, () is for default 0 initialization
	m_UserImageColor_8bit[0] = new unsigned char[m_OutputModeUser.nXRes*m_OutputModeUser.nYRes*3](); //user image -> colored contours, () is for default 0 initialization
	m_UserImageColor_8bit[1] = new unsigned char[m_OutputModeUser.nXRes*m_OutputModeUser.nYRes*3](); //user image -> colored contours, () is for default 0 initialization

	//setting skeleton profile
	checkError( m_UserGenerator.setSkeletonProfile( XN_SKEL_PROFILE_ALL ),
				"_2Real: Error when setting skeleton profile for >>user-generator<< \n" );

	//registering callbacks
	checkError( m_UserGenerator.registerCallbacks(),
				"_2Real: Error when registering callbacks for >>user-generator<< \n" );
}

bool OpenNIDevice::startGenerators( const uint32_t startGenerators )
{
	if( !m_IsInitialized )
	{
		//allocating buffer for users
		m_TrackedUsersArray = new _2RealTrackedUser*[m_TrackedUserArraySize];
		//init with null values
		for( int i=0; i < m_TrackedUserArraySize; ++i )
			m_TrackedUsersArray[i] = NULL;

		try
		{
			if( startGenerators & COLORIMAGE )
			{
				_2REAL_LOG(info) << "_2Real: Starting image generator device " << m_InstanceName << std::endl;
				std::string error = "_2Real: Error when starting >>color-generator<< on device " + m_InstanceName + "\n";
				checkError( m_ColorGenerator.startGenerating(),
							error.c_str() );
				_2REAL_LOG(info) << " ...OK" << std::endl;
			}
			if( startGenerators & INFRAREDIMAGE )
			{
				_2REAL_LOG(info) << "_2Real: Starting infrared generator device " << m_InstanceName.c_str();
				std::string error = "_2Real: Error when starting >>infrared-generator<< on device " + m_InstanceName + "\n";
				checkError( m_InfraredGenerator.startGenerating(),
							error.c_str() );
				_2REAL_LOG(info) << " ...OK" << std::endl;
			}
			if( startGenerators & DEPTHIMAGE )
			{
				_2REAL_LOG(info) << "_2Real: Starting depth generator device " << m_InstanceName.c_str();
				std::string error = "_2Real: Error when starting >>depth-generator<< on device " + m_InstanceName + "\n";
				checkError( GetOpenNIDepthGenerator().StartGenerating(),
							error.c_str() );
				_2REAL_LOG(info) << " ...OK" << std::endl;
			}
			if( startGenerators & USERIMAGE || startGenerators & USERIMAGE_COLORED )
			{
				_2REAL_LOG(info) << "_2Real: Starting user generator device " << m_InstanceName.c_str();
				std::string error = "_2Real: Error when starting >>user-generator<< on device " + m_InstanceName + "\n";
				checkError( m_UserGenerator.startGenerating(),
							error.c_str() );
				_2REAL_LOG(info) << " ...OK" << std::endl;
			}

			//starting boost thread
			m_IsProcessingThread = true;
			_2REAL_LOG(info) << "_2Real: Starting processing thread for device " << m_InstanceName.c_str();
			m_ProcessingThread = boost::thread( boost::bind( &OpenNIDevice::update, this ), this );
			_2REAL_LOG(info) << " ...OK" << std::endl;
			
			return ( m_IsInitialized = true );
		}
		catch( std::exception& e )
		{
			_2REAL_LOG(info) << "_2Real: Error startGenerators() " << std::endl << e.what() << std::endl;
			return false;
		}
	}
	return false; //already started
}

/*! /brief     Normalizing and copying pixel data from source to destination pixel buffer
/param     const uint16_t * source - Pixeldata will be read and normalized
/param     unsigned char * destination - Normalized pixeldata will be written in here
/param     uint32_t size - Size of pixel buffer array - Notice that source and destination have to be the same size!
/return    void
!*/
void OpenNIDevice::convertImage_16_to_8( const uint16_t* source, unsigned char* destination, uint32_t size, const int normalizing )
{
	//iterating each pixel and writing normalized pixel data
	for( unsigned int i=0; i<size; ++i )
		destination[i] = (unsigned char) ( source[i] * ( (float)( 1 << 8 ) / normalizing ) ); //normalized 16bit to 8bit
}

_2Real::_2RealTrackedUserVector OpenNIDevice::getUsers()
{
	m_MutexFetchUsers.lock();
	_2RealTrackedUserVector retUsers;
	for( int i=0; i<m_TrackedUserArraySize; ++i )
	{
		if( m_TrackedUsersArray[i] )
			retUsers.push_back( *m_TrackedUsersArray[i] );
	}
	m_MutexFetchUsers.unlock();
	return retUsers;
}

const XnMapOutputMode& OpenNIDevice::getOutputmodeColor() const
{
	return m_OutputModeColor;
}

const XnMapOutputMode& OpenNIDevice::getOutputmodeDepth() const
{
	return m_OutputModeDepth;
}

const XnMapOutputMode& OpenNIDevice::getOutputmodeInfrared() const
{
	return m_OutputModeInfrared;
}

const XnMapOutputMode& OpenNIDevice::getOutputmodeUser() const
{
	return m_OutputModeUser;
}

uint16_t* OpenNIDevice::getDepthBuffer_16bit()
{
	return const_cast<uint16_t*>( m_DepthImage.getData() );
}

uint8_t* OpenNIDevice::getImageBuffer()
{
	return const_cast<uint8_t*>( m_ColorImage.getData() );
}

uint8_t* OpenNIDevice::getDepthBuffer()
{
	return m_DepthImage_8bit[m_CurrentBuffer];
}

uint8_t* OpenNIDevice::getInfraredBuffer()
{
	return m_InfraredImage_8bit[m_CurrentBuffer];
}

uint8_t* OpenNIDevice::getUserImageBuffer()
{
	return m_UserImage_8bit[m_CurrentBuffer];
}

uint8_t* OpenNIDevice::getUserColorImageBuffer()
{
	return m_UserImageColor_8bit[m_CurrentBuffer];
}

bool OpenNIDevice::shutdown()
{
	if( !m_IsInitialized ) //has not been initialized
		return false;
	try
	{
		//killing processing thread and waiting to join
		m_IsProcessingThread = m_IsInitialized = m_IsFetchingData = false;
		m_ProcessingThread.join();
		
		//stopping OpenNI
		if( GetOpenNIImageGenerator().IsValid() )
		{
			m_ColorGenerator.unlockGenerator();
			m_ColorGenerator.stopGenerating();
		}
		if( GetOpenNIDepthGenerator().IsValid() )
		{
			m_DepthGenerator.unlockGenerator();
			m_DepthGenerator.stopGenerating();
		}
		if( GetOpenNIUserGenerator().IsValid() )
		{
			m_UserGenerator.unlockGenerator();
			m_UserGenerator.stopGenerating();
		}
		if( GetOpenNIInfraredGenertor().IsValid() )
		{
			m_InfraredGenerator.unlockGenerator();
			m_InfraredGenerator.stopGenerating();
		}
		
		//clearing user buffer
		for( int i=0; i < m_TrackedUserArraySize; ++i )
		{
			if( m_TrackedUsersArray[i] )
				delete m_TrackedUsersArray[i];
		}

		//free memory of pixel buffers
		if( m_DepthImage_8bit )
		{
			delete [] m_DepthImage_8bit[0];
			delete [] m_DepthImage_8bit[1];
			delete [] m_DepthImage_8bit;
		}
		if( m_InfraredImage_8bit )
		{
			delete [] m_InfraredImage_8bit[0];
			delete [] m_InfraredImage_8bit[1];
			delete [] m_InfraredImage_8bit;
		}
		if( m_UserImage_8bit )
		{
			delete [] m_UserImage_8bit[0];
			delete [] m_UserImage_8bit[1];
			delete [] m_UserImage_8bit;
		}
		if( m_UserImageColor_8bit )
		{
			delete [] m_UserImageColor_8bit[0];
			delete [] m_UserImageColor_8bit[1];
			delete [] m_UserImageColor_8bit;
		}

	} catch( std::exception& e )
	{
		_2REAL_LOG(info) << "_2Real: Error when trying to shutdown device " << m_InstanceName.c_str()
				  << std::endl << e.what() << std::endl;
	}
	return false;
}

}
#endif