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
#include <Windows.h>
#include "_2RealConfig.h"
#include "I_2RealImplementation.h"
#include "_2RealUtility.h"
#include "NuiApi.h" // enumerate devices, access multiple devices
#include "NuiImageCamera.h" //adjust camera angle, open streams, read image frames
#include "NuiSkeleton.h" //enable skeleton tracking, skeleton data, transforming skeleton
#include <iostream>
#include <vector>
#include "WSDKDevice.h"
#include <sstream>

//Implementation WinSDK

namespace _2RealKinectWrapper
{


class _2RealImplementationWinSDK : public I_2RealImplementation
{
public:

	// called by pimple idiom
	_2RealImplementationWinSDK()
		: m_NumDevices( 0 ),
		  m_IsInitialized( 0 )
	{
		initialize();
	}

	~_2RealImplementationWinSDK()
	{
		
	}


	virtual bool configureDevice( const uint32_t deviceID, uint32_t startGenerators, uint32_t configureImages ) 
	{
		if( !isValidDevice( deviceID ) )
		{
			_2REAL_LOG( error ) << "_2Real: configureDevice() Error, deviceID is not valid!" << std::endl;
			return false;
		}
		if( m_Devices[deviceID]->isDeviceStarted() )
		{
			_2REAL_LOG( error ) << "_2Real: Device: " << deviceID << " has to be stopped before configuring it! Use shutdown()" << std::endl;
			return false;
		}

		WSDKDeviceConfiguration& config = m_Devices[deviceID]->getDeviceConfiguration();
		config.reset();
		config.m_Generators2Real = startGenerators;
		config.m_ImageConfig2Real = configureImages;

		// setting NUI config flags depending on _2real-flags
		if( startGenerators & COLORIMAGE )
		{
			config.m_GeneratorsWSDK |= NUI_INITIALIZE_FLAG_USES_COLOR;
			configureImageColor( config );
		}
		if( startGenerators & USERIMAGE )
		{
			if( deviceID == 0 ) // user-image + skeleton-tracking only on device 0
			{
				config.m_GeneratorsWSDK |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
				config.m_GeneratorsWSDK |= NUI_INITIALIZE_FLAG_USES_SKELETON;
			}
			else // enabling on other devices depth image instead
			{
				_2REAL_LOG( warn ) << "_2Real: Disabling user image and skeleton on device: " << deviceID << " due it is not supported..." << std::endl;
				_2REAL_LOG( warn ) << "_2Real: Enabling depth-sensor ONLY on device: " << deviceID << " ..." << std::endl;
				config.m_GeneratorsWSDK |= NUI_INITIALIZE_FLAG_USES_DEPTH;
			}
			configureImageDepthUser( config );
		}
		else if( startGenerators & DEPTHIMAGE )
		{	
			config.m_GeneratorsWSDK |= NUI_INITIALIZE_FLAG_USES_DEPTH;
			configureImageDepthUser( config );
		}

		if( ( startGenerators & INFRAREDIMAGE ) )
		{
			_2REAL_LOG( info ) << "_2Real: Infrared capability is not supported by Win-SDK!" << std::endl;
		}

		return true;
	}


	virtual void update() 
	{

	}

	virtual void convertProjectiveToWorld( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inProjective, _2RealVector3f* outWorld ) 
	{
		if( !isDeviceStarted( deviceID ) )
			throwError( "_2Real: convertProjectiveToWorld() Error, device not started!" );

		//fetching and writing data to array
		Vector4 out;
		for( uint32_t i=0; i < coordinateCount; ++i )
		{
			out = NuiTransformDepthImageToSkeleton( (LONG)inProjective[i].x, (LONG)inProjective[i].y, (USHORT)inProjective[i].z );
			outWorld[i].x = out.x;
			outWorld[i].y = out.y;
			outWorld[i].z = out.z;
		}
	}

	virtual void convertWorldToProjective( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inWorld, _2RealVector3f* outProjective ) 
	{
		if( !isDeviceStarted( deviceID ) )
			throwError( "_2Real: convertWorldToProjective() Error, device not started!" );

		//fetching and writing data
		Vector4 in;
		USHORT depth;
		for( uint32_t i=0; i<coordinateCount; ++i )
		{
			in.x = inWorld[i].x;
			in.y = inWorld[i].y;
			in.z = inWorld[i].z;
			LONG x,y;
			NuiTransformSkeletonToDepthImage( in, &x, &y, &depth );
			outProjective[i].x = (float)x;
			outProjective[i].y = (float)y;
			outProjective[i].z = depth;
		}
	}

	virtual bool isMirrored( const uint32_t deviceID, _2RealGenerator type ) const
	{
		return true;
	}

	virtual bool generatorIsActive( const uint32_t deviceID, _2RealGenerator type ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}

	virtual void setResolution( const uint32_t deviceID, _2RealGenerator type, unsigned int hRes, unsigned int vRes ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}

	virtual void setLogLevel( _2RealLogLevel iLevel ) 
	{
		_2RealLogger::getInstance().setLogLevel(iLevel);
	}

	virtual void resetSkeleton( const uint32_t deviceID, const uint32_t id ) 
	{
		_2REAL_LOG(warn) << "_2Real: There is no use for this functionality in the WSDK" << std::endl;
	}

	virtual void resetAllSkeletons() 
	{
		_2REAL_LOG(warn) << "_2Real: There is no use for this functionality in the WSDK" << std::endl;
	}

	virtual void addGenerator( const uint32_t deviceID, uint32_t configureGenerators, uint32_t configureImages ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}

	virtual bool isJointAvailable( _2RealJointType type ) const
	{
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2Real: isJointAvailable() Error, type index out of bounds!" );

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

	virtual void setMirrored( const uint32_t deviceID, _2RealGenerator type, bool flag ) 
	{
		
	}

	virtual void removeGenerator( const uint32_t deviceID, uint32_t configureGenerators ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}

	virtual const bool isNewData( const uint32_t deviceID, _2RealGenerator type ) const
	{
		if( isDeviceStarted( deviceID ) )
			return m_Devices[deviceID]->isNewData(type);
		return false;
	}

	virtual void setLogOutputStream( std::ostream* outStream ) 
	{
		_2RealLogger::getInstance().setLogOutputStream(outStream); 
	}

	virtual bool hasFeatureJointOrientation() const
	{
		return true;
	}

	virtual bool setMotorAngle( int deviceID, int& angle ) 
	{
		return m_Devices[deviceID]->setMotorAngle(angle);
	}

	virtual bool depthIsAlignedToColor( const uint32_t deviceID ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}

	virtual const _2RealOrientationsMatrix3x3 getSkeletonWorldOrientations( const uint32_t deviceID, const uint8_t userID ) 
	{
		return getCheckedUser( "getSkeletonWorlOrientations", deviceID, userID )->getSkeletonWorldOrientations();
	}
	
	virtual const _2RealJointConfidence getSkeletonJointConfidence( const uint32_t deviceID, const uint8_t userID, _2RealJointType type ) 
	{
		return getCheckedUser( "getSkeletonJointConfidence", deviceID, userID )->getJointConfidence(type);
	}

	virtual const _2RealJointConfidences getSkeletonJointConfidences( const uint32_t deviceID, const uint8_t userID ) 
	{
		return getCheckedUser( "getSkeletonJointConfidences", deviceID, userID )->getJointConfidences();
	}

	virtual const _2RealPositionsVector3f getSkeletonWorldPositions( const uint32_t deviceID, const uint8_t userID ) 
	{
		return getCheckedUser( "getSkeletonWorldPositions", deviceID, userID )->getSkeletonWorldPositions();
	}

	virtual const _2RealPositionsVector3f getSkeletonScreenPositions( const uint32_t deviceID, const uint8_t userID ) 
	{
		return getCheckedUser( "getSkeletonScreenPositions", deviceID, userID )->getSkeletonScreenPositions();
	}

	virtual boost::shared_array<unsigned char> getImageData( const uint32_t deviceID, _2RealGenerator type, bool waitAndBlock=false, const uint8_t userId=0 ) 
	{
		if( !isDeviceStarted( deviceID ) )
			throwError( "_2Real: getImageData() Error, device is not started!" );

		boost::shared_array<unsigned char> return_ptr;
		if( type == COLORIMAGE )
			return_ptr = m_Devices[deviceID]->getColorImageBuffer( waitAndBlock );
		else if( type == DEPTHIMAGE )
			return_ptr = m_Devices[deviceID]->getDepthImageBuffer( waitAndBlock );
		else if( type == USERIMAGE_COLORED )
			return_ptr = m_Devices[deviceID]->getColoredUserImageBuffer( waitAndBlock );			
		else if( type == USERIMAGE )
			return_ptr = m_Devices[deviceID]->getUserImageBuffer( waitAndBlock );
		else if( type == INFRAREDIMAGE )
		{
			_2REAL_LOG(warn) << "_2Real: Infrared image isn't supported at the moment!" << std::endl;
		}
		else
		{
			throwError( "_2RealKinectWrapper::getImageDataOf() Error: Wrong type of generator assigned?!" );
		}
		return return_ptr;
	}

	virtual boost::shared_array<uint16_t> getImageDataDepth16Bit( const uint32_t deviceID, bool waitAndBlock=false ) 
	{
		if( !isDeviceStarted( deviceID ) )
			throwError( "_2Real: getImageDataDepth16Bit() Error, device is not started!" );
		return m_Devices[deviceID]->getDepthImageBuffer16Bit( waitAndBlock );
	}

	virtual uint32_t getImageHeight( const uint32_t deviceID, _2RealGenerator type ) 
	{
		if( !isValidDevice( deviceID ) || type == INFRAREDIMAGE )
			return 0;

		if( type == COLORIMAGE )
			return m_Devices[deviceID]->m_HeightImageColor;
		else if( type == DEPTHIMAGE ||
			type == USERIMAGE ||
			type == USERIMAGE_COLORED )
			return m_Devices[deviceID]->m_HeightImageDepthAndUser;
		return 0;
	}

	virtual uint32_t getImageWidth( const uint32_t deviceID, _2RealGenerator type ) 
	{
		if( !isValidDevice( deviceID ) || type == INFRAREDIMAGE )
			return 0;

		if( type == COLORIMAGE )
			return m_Devices[deviceID]->m_WidthImageColor;
		else if( type == DEPTHIMAGE ||
			type == USERIMAGE ||
			type == USERIMAGE_COLORED )
			return m_Devices[deviceID]->m_WidthImageDepthAndUser;
		return 0;
	}

	virtual const _2RealMatrix3x3 getJointWorldOrientation( const uint32_t deviceID, const uint8_t userID, _2RealJointType type ) 
	{
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2Real: getJointWorldOrientation() Error, joint id out of bounds!" );
		return getCheckedUser( "getJointWorldOrientation", deviceID, userID )->getJointWorldOrientation( type );
	}

	virtual const _2RealVector3f getJointWorldPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type ) 
	{
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2Real: getJointWorldPosition() Error, joint id out of bounds!" );
		return getCheckedUser( "getJointWorldPosition", deviceID, userID )->getJointWorldPosition( type );
	}

	virtual const _2RealVector3f getJointScreenPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type ) 
	{
		if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2Real: getJointScreenPosition() Error, joint id out of bounds!" );
		return 	getCheckedUser( "getJointScreenPosition", deviceID, userID )->getJointScreenPosition( type );
	}

	virtual uint32_t getBytesPerPixel( _2RealGenerator type ) const
	{
		if( type == COLORIMAGE || type == USERIMAGE_COLORED ) //rgb image 3byte/Pixel
			return 3;
		return 1; //depth-, and userimage will be converted to 1byte/Pixel (8bit uchar*)
	}

	virtual uint32_t getNumberOfDevices() const
	{
		return m_NumDevices;
	}

	virtual const uint32_t getNumberOfUsers( const uint32_t deviceID ) const
	{
		if( !isDeviceStarted( deviceID ) )
			throwError( "_2Real: getNumberOfUsers() Error, device is not started!" );
		return m_Devices[deviceID]->getNumberOfUsers();
	}

	virtual const uint32_t getNumberOfSkeletons( const uint32_t deviceID ) const
	{
		return getNumberOfUsers( deviceID );
	}

	virtual int getMotorAngle( int deviceID ) 
	{
		if( !isValidDevice( deviceID ) )
			throwError( "_2Real: getMotorAngle() Error, deviceID is not valid!" );
		return m_Devices[deviceID]->getMotorAngle();
	}

	virtual const _2RealVector3f getUsersScreenCenterOfMass( const uint32_t deviceID, const uint8_t userID ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}

	virtual const _2RealVector3f getUsersWorldCenterOfMass( const uint32_t deviceID, const uint8_t userID ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}

	virtual bool restart() 
	{
		shutdown();
		
		Sleep( 1000 ); //preventing reinitialization t00 fast

		_2REAL_LOG(info) << "_2Real: Restarting system..." << std::endl;
		for( uint8_t i = 0; i < m_NumDevices; ++i )
			startGenerator( i, m_Devices[i]->getDeviceConfiguration().m_Generators2Real );
		_2REAL_LOG(info) << "_2Real: Restart: OK" << std::endl;

		return true;
	}

	virtual bool shutdown() 
	{
		if( m_IsInitialized )
		{
			_2REAL_LOG( info )  << std::endl << "_2Real: Shutting down system..." << std::endl;
			//m_IsInitialized = 0;
			// stopping all devices
			for(unsigned int i=0; i<m_Devices.size(); i++)
				m_Devices[i]->shutdown();
			m_IsInitialized = false;
			_2REAL_LOG( info ) << "OK" << std::endl;
			return true;
		}
		_2REAL_LOG( warn ) << std::endl << "_2Real: System not shutdown correctly..." << std::endl;
		return false;
	}

	virtual void startGenerator( const uint32_t deviceID, uint32_t configureGenerators ) 
	{
		if( !isValidDevice( deviceID ) )
			throwError( "_2Real: Error, passing wrong index to startGenerator()" );

		// runtime configuration change
		if( isDeviceStarted( deviceID ) )
		{
			m_Devices[deviceID]->startGenerator( configureGenerators );
		}
		// called on first start/restart
		else if( m_Devices[deviceID]->isDeviceShutDown() )
		{
			m_Devices[deviceID]->start();
			_2REAL_LOG(info) << "_2Real: Initialization: OK" << std::endl;
		}
	}

	virtual void stopGenerator( const uint32_t deviceID, uint32_t configureGenerators ) 
	{
		if( isValidDevice( deviceID ) && isDeviceStarted( deviceID ) )
		{
			m_Devices[deviceID]->stopGenerator( configureGenerators );
		}
	}

	virtual void alignDepthToColor( const uint32_t deviceID, bool flag ) 
	{
		throw std::exception("The method or operation is not implemented.");
	}


	private:

		/*! /brief     Determines if a given device-id/index is valid/has been detected
			/param     uint8_t deviceID - Device-id to check
			/return    bool - If device-id is valid
		!*/
		bool isValidDevice( uint8_t deviceID ) const
		{
			if ( deviceID > (m_NumDevices - 1) )
				return false;
			return true;
		}

		bool isDeviceStarted( uint8_t deviceID ) const
		{
			if( !isValidDevice( deviceID ) )
				throwError( "_2Real: isDeviceStarted() Error, deviceID is not valid!" );
			return m_Devices[deviceID]->isDeviceStarted();
		}

		// Todo: Solution for copy over _2realtrackeduservector 3times to "final" user
		/*! /brief     Checking method for getter functions
		!*/ 
		boost::shared_ptr<_2RealTrackedUser> getCheckedUser( const std::string& methodName, uint8_t deviceID, uint8_t userID ) const
		{
			if( !isDeviceStarted( deviceID ) )
				throwError( "_2Real: " + methodName + "() Error, device is not started" );
			_2RealTrackedUserVector OutUsers;
			m_Devices[deviceID]->getUsers( false, OutUsers ); 
			if( userID >= OutUsers.size() )
				throwError( "_2Real: " + methodName + "() Error, userID out of bounds!" );
			return OutUsers[userID];
		}

		/*! /brief     Assembles image-configuration for the color image of this device
			/param     uint32_t config - Configuration of the given device
		!*/
		void configureImageColor( WSDKDeviceConfiguration& config  )
		{
			if( config.m_ImageConfig2Real & IMAGE_COLOR_1280X960 )
			{
				config.m_ImageResColor.WSDKResType = NUI_IMAGE_RESOLUTION_1280x960;
				config.m_ImageResColor.width = 1280; 
				config.m_ImageResColor.height = 960;
			}
			else if( config.m_ImageConfig2Real & IMAGE_COLOR_640X480 )
			{
				config.m_ImageResColor.WSDKResType = NUI_IMAGE_RESOLUTION_640x480;
				config.m_ImageResColor.width = 640;
				config.m_ImageResColor.height = 480;
			}
			else if( config.m_ImageConfig2Real & IMAGE_COLOR_320X240 )
			{
				config.m_ImageResColor.WSDKResType = NUI_IMAGE_RESOLUTION_320x240;
				config.m_ImageResColor.width = 320;
				config.m_ImageResColor.height = 240;
			}
			else
			{
				config.m_ImageResColor.WSDKResType = NUI_IMAGE_RESOLUTION_640x480;
				config.m_ImageResColor.width = 640;
				config.m_ImageResColor.height = 480;
			}
			_2REAL_LOG(info)	<< "_2Real: Set COLOR-IMAGE to resolution: " << config.m_ImageResColor.width << "x"
								<< config.m_ImageResColor.height << std::endl;
		}

		/*! /brief     Assembles image-configuration for the user & color image of this device
			/param     uint32_t config - Configuration of the given device
		!*/
		void configureImageDepthUser( WSDKDeviceConfiguration& config )
		{
			if( config.m_ImageConfig2Real & IMAGE_USER_DEPTH_640X480 )
			{
				config.m_ImageResDepth.WSDKResType = config.m_ImageResUser.WSDKResType = NUI_IMAGE_RESOLUTION_640x480;
				config.m_ImageResDepth.width = config.m_ImageResUser.width = 640;
				config.m_ImageResDepth.height = config.m_ImageResUser.height = 480;
			}
			else if( config.m_ImageConfig2Real & IMAGE_USER_DEPTH_320X240 )
			{
				config.m_ImageResDepth.WSDKResType = config.m_ImageResUser.WSDKResType = NUI_IMAGE_RESOLUTION_320x240;
				config.m_ImageResDepth.width = config.m_ImageResUser.width = 320;
				config.m_ImageResDepth.height = config.m_ImageResUser.height = 240;
			}
			else if( config.m_ImageConfig2Real & IMAGE_USER_DEPTH_80X60 )
			{
				config.m_ImageResDepth.WSDKResType = config.m_ImageResUser.WSDKResType = NUI_IMAGE_RESOLUTION_80x60;
				config.m_ImageResDepth.width = config.m_ImageResUser.width = 80;
				config.m_ImageResDepth.height = config.m_ImageResUser.height = 60;
			}
			else
			{
				config.m_ImageResDepth.WSDKResType = config.m_ImageResUser.WSDKResType = NUI_IMAGE_RESOLUTION_320x240;					// !!!!! depth with 640x480 and 640x480 color will only work in release mode for some strange reaseon, so default set to 320x240
				config.m_ImageResDepth.width = config.m_ImageResUser.width = 320;
				config.m_ImageResDepth.height = config.m_ImageResUser.height = 240;
			}
			_2REAL_LOG(info)	<< "_2Real: Set USER_DEPTH-IMAGE to resolution: " << config.m_ImageResUser.width << "x"
								<< config.m_ImageResUser.height << std::endl;
		}

		void initialize()
		{
			if( !m_IsInitialized )
			{
				_2REAL_LOG( info ) << "_2Real: Initializing Microsoft-Kinect-SDK" << std::endl;
				m_IsInitialized = true;

				HRESULT status = 0;
				//get number of devices
				int deviceCount = 0;
				if( FAILED( status = NuiGetSensorCount( &deviceCount ) ) )
				{
					throwError( "_2Real: Error when trying to enumerate devices" );
				}

				//abort if no devices found
				if( ( m_NumDevices = deviceCount ) == 0 )
				{
					_2REAL_LOG( error ) << "_2Real: No devices found" << std::endl;
				}
				else
				{
					//m_Configurations = boost::shared_array<WSDKDeviceConfiguration>( new WSDKDeviceConfiguration[deviceCount] );
					m_Devices.resize( deviceCount ); // deviceID is also unique vector-index

					char cBuf[16];
					strcpy_s( cBuf, "kinect_device_" );
					for( int i = 0; i < deviceCount; ++i )
					{
						//creating instance
						INuiSensor *pSensor;
						if( FAILED( NuiCreateSensorByIndex( i, &pSensor ) ) )
							throwError( ( "_2Real: Error when trying to create device: " + i ) );

						_itoa_s( i, &cBuf[14], 2, 10 );

						boost::shared_ptr<WSDKDevice> device( new WSDKDevice( pSensor, cBuf ) );
						m_Devices[i] = device; // put device to according index
					}			
				}

				
				_2REAL_LOG( info ) << "_2Real: detected number of sensors: " << deviceCount << std::endl;
			}
		}

		uint32_t										m_NumDevices;		// number of detected kinect sensors
		std::vector<boost::shared_ptr<WSDKDevice>>		m_Devices;			// array holding references to device-implementation
		bool											m_IsInitialized;	// information if wrapper has been initialized
};

}

#endif
