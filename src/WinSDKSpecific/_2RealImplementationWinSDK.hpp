/*
	CADET - Center for Advances in Digital Entertainment Technologies
	Copyright 2011 University of Applied Science Salzburg / MultiMediaTechnology

	http://www.cadet.at
	http://multimediatechnology.at/

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

	Authors: Robert Praxmarer, Gerlinde Emsenhuber, Robert Sommeregger
	Email: support@cadet.at
	Created: 08-09-2011
*/


#pragma once
#ifdef TARGET_MSKINECTSDK
#include "_2RealConfig.h"
#include "I_2RealImplementation.h"
#include "_2RealUtility.h"
#include <Windows.h>
#include "MSR_NuiApi.h" // enumerate devices, access multiple devices
#include "MSR_NuiImageCamera.h" //adjust camera angle, open streams, read image frames
#include "MSR_NuiProps.h" //defines api properties enumerators
#include "MSR_NuiSkeleton.h" //enable skeleton tracking, skeleton data, transforming skeleton
#include <iostream>
#include <vector>
#include "WSDKDevice.h"
#include <sstream>


//Implementation WinSDK

namespace _2Real
{

class _2RealImplementationWinSDK : public I_2RealImplementation
{
private:
	uint32_t						m_NumDevices;
	std::vector<WSDKDevice*>		m_Devices;
	bool							m_IsInitialized;
	uint32_t						m_GeneratorConfig;
	uint32_t						m_ImageConfig;

	void checkDeviceRunning(uint8_t deviceID, std::string strError) const
	{
		//checking device id
		if ( deviceID > (m_NumDevices - 1) )
			throwError((strError+" error, deviceID out of bounds!").c_str());
	}

public:

	_2RealImplementationWinSDK()
		: m_NumDevices( 0 ),
		m_IsInitialized( 0 ),
		m_GeneratorConfig( CONFIG_DEFAULT ),
		m_ImageConfig( IMAGE_CONFIG_DEFAULT )
	{
		
	}

	~_2RealImplementationWinSDK()
	{
		shutdown();
	}

	virtual bool start( uint32_t startGenerators, uint32_t configureImages ) 
	{
		if( m_IsInitialized )
			return false;
		m_GeneratorConfig = startGenerators;
		m_ImageConfig = configureImages;

		//initialization --------------------------------------------------------->
		_2REAL_LOG(info) << "_2Real: Initializing" << std::endl;
		
		HRESULT status = 0;
		//get number of devices
		int deviceCount = 0;
		if( FAILED( status = MSR_NUIGetDeviceCount( &deviceCount ) ) )
		{
			throwError( "_2Real: Error when trying to enumerate devices\n" );
		}

		//abort if no devices found
		if( ( m_NumDevices = deviceCount ) == 0 )
		{
			_2REAL_LOG(error) << "_2Real: No devices found" << std::endl;
			return false;
		}

		_2REAL_LOG(info) << "_2Real: detected number of sensors: " << deviceCount << std::endl;
		std::stringstream ss;

		//initialization putting device instances to m_devices vector
		INuiInstance* devicePtr;
		for( int i = 0; i < deviceCount; ++i )
		{
			//creating instance
			if( FAILED( status = MSR_NuiCreateInstanceByIndex( i, &devicePtr ) ) )
			{
				throwError(("_2Real: Error when trying to create device: " + i));
			}
			//saving to vector
			ss.str( "" ); 
			ss << "kinect_device_" << i;
			m_Devices.push_back( new WSDKDevice( devicePtr, startGenerators, configureImages, ss.str().c_str() ) );
		}
		m_IsInitialized = 1;
		_2REAL_LOG(info) << "_2Real: Initialization: OK" << std::endl;
		return true;
	}

	virtual const _2RealTrackedUserVector& getUsers( const uint32_t deviceID, bool waitAndBlock ) 
	{
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getUsers()" );
		return m_Devices[deviceID]->GetUsers( waitAndBlock );
	}

	virtual const _2RealVector3f getJointWorldPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
	{
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getJointWorldPosition()" );
		_2RealTrackedUserVector& users = m_Devices[deviceID]->GetUsers( 0 ); 
		if( userID >= users.size() )
			throwError( "_2RealImplWSDK::getJointWorldPosition() Error, userID out of bounds!" );
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2RealImplWinSDK::getJointWorldPosition() Error, joint id out of bounds!" );
		return users[userID].getJointWorldPosition( type );
	}

	virtual const _2RealPositionsVector3f& getSkeletonWorldPositions( const uint32_t deviceID, const uint8_t userID )
	{
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getSkeletonWorldPositions()" );
		_2RealTrackedUserVector& users = m_Devices[deviceID]->GetUsers( 0 ); 
		if( userID >= users.size() )
			throwError( "_2RealImplWSDK::getSkeletonWorldPositions() Error, userID out of bounds!" );
		return users[userID].getSkeletonWorldPositions();
	}

	virtual const _2RealVector2f getJointScreenPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
	{
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getJointScreen()" );
		_2RealTrackedUserVector& users = m_Devices[deviceID]->GetUsers( 0 ); 
		if( userID >= users.size() )
			throwError( "_2RealImplWSDK::getJointScreen() Error, userID out of bounds!" );
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2RealImplWinSDK::getJointScreen() Error, joint id out of bounds!" );
		return users[userID].getJointScreenPosition( type );
	}

	virtual const _2RealPositionsVector2f& getSkeletonScreenPositions( const uint32_t deviceID, const uint8_t userID )
	{
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getSkeletonScreenPositions()" );
		_2RealTrackedUserVector& users = m_Devices[deviceID]->GetUsers( 0 ); 
		if( userID >= users.size() )
			throwError( "_2RealImplWSDK::getSkeletonScreenPositions() Error, userID out of bounds!" );
		return users[userID].getSkeletonScreenPositions();
	}

	virtual const _2RealMatrix3x3 getJointWorldOrientation( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
	{
		_2REAL_LOG(warn) << "_2Real: Joint rotations aren't supported yet by MS Kinect SDK" << std::endl;
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getJointScreen()" );
		_2RealTrackedUserVector& users = m_Devices[deviceID]->GetUsers( 0 ); 
		if( userID >= users.size() )
			throwError( "_2RealImplWSDK::getJointScreen() Error, userID out of bounds!" );
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2RealImplWinSDK::getJointScreen() Error, joint id out of bounds!" );
		return users[userID].getJointWorldOrientation( type );
	}

	virtual const _2RealOrientationsMatrix3x3& getSkeletonWorldOrientations( const uint32_t deviceID, const uint8_t userID )
	{
		_2REAL_LOG(warn) << "_2Real: Joint rotations aren't supported yet by MS Kinect SDK" << std::endl;
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getSkeletonWorldOrientations()" );
		_2RealTrackedUserVector& users = m_Devices[deviceID]->GetUsers( 0 ); 
		if( userID >= users.size() )
			throwError( "_2RealImplWSDK::getSkeletonWorldOrientations() Error, userID out of bounds!" );
		return users[userID].getSkeletonWorldOrientations();
	}

	virtual const uint32_t getNumberOfUsers( const uint32_t deviceID ) const
	{
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getNumberOfUsers()" );
		return m_Devices[deviceID]->GetUsers( 0 ).size();
	}

	virtual const uint32_t getNumberOfSkeletons( const uint32_t deviceID ) const
	{
		checkDeviceRunning(deviceID, "_2RealImplWSDK::getNumberOfSkeletons()" );
		return m_Devices[deviceID]->GetUsers( 0 ).size();		
	}

	virtual bool isJointAvailable( _2RealJointType type ) const
	{
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2RealImplWinSDK::isJointAvailable()" );

		switch( int( type ) )
		{
		case JOINT_LEFT_COLLAR:
		case JOINT_LEFT_FINGERTIP:
		case JOINT_RIGHT_COLLAR:
		case  JOINT_RIGHT_FINGERTIP:
			return false;
		default:
			return true;
		}
	}

	virtual bool hasFeatureJointOrientation() const
	{
		return false;
	}

	virtual void resetAllSkeletons() 
	{
		_2REAL_LOG(warn) << "_2Real: There is no use for this functionality in the WSDK" << std::endl;
	}

	virtual void resetSkeleton( const uint32_t deviceID, const uint32_t id ) 
	{
		_2REAL_LOG(warn) << "_2Real: There is no use for this functionality in the WSDK" << std::endl;
	}

	virtual bool isMirrored( const uint32_t deviceID, _2RealGenerator type ) const
	{
		checkDeviceRunning(deviceID, "_2RealImplWinSDK::setMirrored()" );

		bool value = 0;
		switch( type )
		{
		case COLORIMAGE:
			{
				value = m_Devices[deviceID]->IsMirroingColor();
				break;
			}
		case DEPTHIMAGE:
			{
				value = m_Devices[deviceID]->IsMirroingDepth();
				break;
			}
		case INFRAREDIMAGE:
			{
				_2REAL_LOG(warn) << "_2Real: Infrared image isn't supported at the moment!" << std::endl;
				value = 0;
				break;
			}
		case USERIMAGE_COLORED:
		case USERIMAGE:
			{
				value = m_Devices[deviceID]->IsMirroingUser();
				break;
			}
		default:
			throwError( "_2RealImplWinSDK::setMirrored() Error: Wrong type of generator assigned?!" );
		}
		return value;
	}

	virtual void setMirrored( const uint32_t deviceID, _2RealGenerator type, bool flag ) 
	{
		checkDeviceRunning(deviceID, "_2RealImplWinSDK::setMirrored()" );

		switch( type )
		{
		case COLORIMAGE:
			{
				m_Devices[deviceID]->SetMirroringColor( flag );
				break;
			}
		case DEPTHIMAGE:
			{
				m_Devices[deviceID]->SetMirroringDetph( flag );

				break;
			}
		case INFRAREDIMAGE:
			{
				_2REAL_LOG(warn) << "_2Real: Infrared image isn't supported at the moment!" << std::endl;
				break;
			}
		case USERIMAGE_COLORED:
		case USERIMAGE:
			{
				m_Devices[deviceID]->SetMirroringUser( flag );
				break;
			}
		default:
			throwError( "_2RealImplWinSDK::setMirrored() Error: Wrong type of generator assigned?!" );
		}
	}

	virtual uint32_t getNumberOfDevices() const
	{
		return m_NumDevices;
	}

	virtual bool shutdown() 
	{
		if( m_IsInitialized )
		{
			_2REAL_LOG(info)  << std::endl << "_2Real: Shutting down system..." << std::endl;
			m_IsInitialized = 0;
			for( unsigned int i = 0; i < m_Devices.size(); ++i )
			{
				WSDKDevice* device = m_Devices[i];
				delete device;
			}
			_2REAL_LOG(info) << "OK" << std::endl;
			return true;
		}
		_2REAL_LOG(warn) << std::endl << "_2Real: System not initialized..." << std::endl;
		return false;
	}

	virtual uint32_t getImageHeight( const uint32_t deviceID, _2RealGenerator type ) 
	{
		checkDeviceRunning(deviceID, "_2RealImplWinSDK::getImageHeight()" );

		uint32_t value = 0;
		switch( type )
		{
		case COLORIMAGE:
			{
				value = m_Devices[deviceID]->m_HeightImageColor;
				break;
			}
		case DEPTHIMAGE:
			{
				value = m_Devices[deviceID]->m_HeightImageDepthAndUser;
				break;
			}
		case INFRAREDIMAGE:
			{
				_2REAL_LOG(warn) << "_2Real: Infrared image isn't supported at the moment!" << std::endl;
				value = 0;
				break;
			}
		case USERIMAGE_COLORED:
		case USERIMAGE:
			{
				value = m_Devices[deviceID]->m_HeightImageDepthAndUser;
				break;
			}
		default:
			throwError( "_2RealImplWinSDK::getImageHeight() Error: Wrong type of generator assigned?!" );
		}
		return value;
	}

	virtual uint32_t getImageWidth( const uint32_t deviceID, _2RealGenerator type ) 
	{
		checkDeviceRunning(deviceID, "_2RealImplWinSDK::getImageWidth()" );

		uint32_t value = 0;
		switch( type )
		{
		case COLORIMAGE:
			{
				value = m_Devices[deviceID]->m_WidthImageColor;
				break;
			}
		case DEPTHIMAGE:
			{
				value = m_Devices[deviceID]->m_WidthImageDepthAndUser;
				break;
			}
		case INFRAREDIMAGE:
			{
				_2REAL_LOG(warn) << "_2Real: Infrared image isn't supported at the moment!" << std::endl;
				value = 0;
				break;
			}
		case USERIMAGE_COLORED:
		case USERIMAGE:
			{
				value = m_Devices[deviceID]->m_WidthImageDepthAndUser;
				break;
			}
		default:
			throwError( "_2RealImplWinSDK::getImageWidth() Error: Wrong type of generator assigned?!" );
		}
		return value;
	}

	virtual uint32_t getBytesPerPixel( _2RealGenerator type ) const
	{
		if( type == COLORIMAGE || type == USERIMAGE_COLORED ) //rgb image 3byte/Pixel
			return 3;
		return 1; //depth-, and userimage will be converted to 1byte/Pixel (8bit uchar*)
	}

	virtual unsigned char* getImageData( const uint32_t deviceID, _2RealGenerator type, bool waitAndBlock=false, const uint8_t userId=0 ) 
	{
		checkDeviceRunning(deviceID, "_2RealImplOpenNI::getUsers" );

		uchar* return_ptr = NULL;
		switch( type )
		{
		case COLORIMAGE:
			{
				return_ptr = m_Devices[deviceID]->GetColorImageBuffer( waitAndBlock );
				break;
			}
		case DEPTHIMAGE:
			{
				return_ptr = m_Devices[deviceID]->GetDepthImageBuffer( waitAndBlock );
				break;
			}
		case INFRAREDIMAGE:
			{
				_2REAL_LOG(warn) << "_2Real: Infrared image isn't supported at the moment!" << std::endl;
				return_ptr = 0;
				break;
			}

		case USERIMAGE_COLORED:
			{
				return_ptr = m_Devices[deviceID]->GetColoredUserImageBuffer( waitAndBlock );			
				break;
			}
		case USERIMAGE:
			{							
				return_ptr = m_Devices[deviceID]->GetUserImageBuffer( waitAndBlock );
				break;
			}
		default:
			throwError( "_2RealImplOpenNI::getImageDataOf() Error: Wrong type of generator assigned?!" );
		}

		return return_ptr;
	}

	virtual uint16_t* getImageDataDepth16Bit( const uint32_t deviceID, bool waitAndBlock=false)
	{
		checkDeviceRunning(deviceID, "_2RealImplOpenNI::getDepthImageData16Bit" );
		return m_Devices[deviceID]->GetDepthImageBuffer16Bit( waitAndBlock );
	}

	virtual void setAlignColorDepthImage( const uint32_t deviceID, bool flag ) 
	{
		throwError("The method or operation is not implemented.");
	}

	virtual bool restart()
	{
		shutdown();
		m_Devices.clear();
		Sleep( 3000 ); //preventing reinitialization t00 fast
		_2REAL_LOG(info) << "_2Real: Restarting system..." << std::endl;
		return start( m_GeneratorConfig, m_ImageConfig );
	}

	virtual void convertProjectiveToWorld( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inProjective, _2RealVector3f* outWorld )
	{
		checkDeviceRunning(deviceID,  "_2RealImplOpenNI::convertProjectiveToWorld()" );

		//fetching and writing data to array
		Vector4 out;
		for( uint32_t i=0; i < coordinateCount; ++i )
		{
			out = NuiTransformDepthImageToSkeletonF( inProjective[i].x, inProjective[i].y, (USHORT)inProjective[i].z );
			outWorld[i].x = out.x;
			outWorld[i].y = out.y;
			outWorld[i].z = out.z;
		}
	}

	virtual void convertWorldToProjective( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inWorld, _2RealVector3f* outProjective )
	{
		checkDeviceRunning(deviceID, "_2RealImplOpenNI::convertWorldToProjective()" );

		//fetching and writing data
		Vector4 in;
		USHORT depth;
		for( uint32_t i=0; i<coordinateCount; ++i )
		{
			in.x = inWorld[i].x;
			in.y = inWorld[i].y;
			in.z = inWorld[i].z;
			NuiTransformSkeletonToDepthImageF( in, &outProjective[i].x, &outProjective[i].y, &depth );
			outProjective[i].z = depth;
		}
	}

	virtual void setLogLevel(_2RealLogLevel iLevel) 
	{ 
		_2RealLogger::getInstance().setLogLevel(iLevel); 
	};

	virtual void setLogOutputStream(std::ostream* outStream) 
	{  
		_2RealLogger::getInstance().setLogOutputStream(outStream); 
	};
};

}

#endif
