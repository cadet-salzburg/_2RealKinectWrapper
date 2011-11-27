/*
	CADET - Center for Advances in Digital Entertainment Technologies
	Copyright 2011 Fachhochschule Salzburg GmbH

	http://www.cadet.at

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
	Email: info@cadet.at
	Created: 08-09-2011
*/


#pragma once
#ifndef TARGET_MSKINECTSDK
#include "_2RealConfig.h"
#include "boost/thread.hpp"
#include "boost/thread/mutex.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include "I_2RealImplementation.h"
#include "_2RealTypes.h"
#include "XnOpenNI.h"
#include "OpenNISpecific/OpenNIDevice.h"
#include <iostream>
#include <istream>


namespace _2Real
{

class _2RealImplementationOpenNI : public I_2RealImplementation
{
	private:
		 //the XnContext used to create the production nodes
		xn::Context								m_Context;
		boost::mutex							m_Mutex;
		uint8_t									m_NumDevices;
		OpenNIDevice**							m_Devices;
		uint32_t								m_GeneratorConfig;
		uint32_t								m_ImageConfig;
		bool									m_IsInitialized;
		_2RealTrackedUserVector					m_TrackedUserVector;

	public:

		_2RealImplementationOpenNI()
			: m_NumDevices( 0 ),
			m_IsInitialized( 0 ),
			m_GeneratorConfig( CONFIG_DEFAULT ),
			m_ImageConfig( IMAGE_CONFIG_DEFAULT )
		{

		}

		~_2RealImplementationOpenNI()
		{
			shutdown();
		}

		virtual bool start( uint32_t startGenerators, uint32_t configureImages )
		{
			if( m_IsInitialized ) //check if already initialized
				return false;

			m_GeneratorConfig = startGenerators;
			m_ImageConfig = configureImages;

			//INITIALIZING!!!
			_2REAL_LOG(info) << "\n_2Real: Initialized context...";
			checkError( m_Context.Init(), "_2RealIMplOpenNI::start Error: Could not Initialize kinect...?!\n" );
			_2REAL_LOG(info) << "OK" << std::endl;

			
			//get number of devices -------------------------------------------------------------------------------->
			
			//fetching all detected kinect devices
			xn::NodeInfoList deviceNodes;
			checkError( m_Context.EnumerateProductionTrees( XN_NODE_TYPE_DEVICE, NULL, deviceNodes, NULL ),
					   "_2RealIMplOpenNI::start Error when enumerating device Nodes\n" );
			
			xn::NodeInfoList::Iterator deviceIter = deviceNodes.Begin();
			for( ; deviceIter != deviceNodes.End(); ++deviceIter )
				++m_NumDevices;
			_2REAL_LOG(info) << "_2Real: Found and allocated " << (int)m_NumDevices << " device[s]" << std::endl;

			//no devices found
			if( m_NumDevices == 0 )
			{
				m_Context.Release();
				return false;
			}

			//allocating devices ----------------------------------------------------------------------------------->
			m_Devices = new OpenNIDevice*[m_NumDevices];
			
			xn::NodeInfoList::Iterator devIter = deviceNodes.Begin();
			for ( int i = 0; i < m_NumDevices; ++i, ++devIter )
			{
				std::stringstream name;
				name << "_2Real_Kinect_" << i;
				//initializing device
				xn::NodeInfo nodeinfo = (*devIter);
				m_Devices[i] = new OpenNIDevice( i, name.str(), m_Context, nodeinfo );
			}


			//PROCESSING COLOR GENERATORS--------------------------------------------------------------------------->
			if( startGenerators & COLORIMAGE )
			{
				//fetching all image generator-nodes
				xn::NodeInfoList imageNodes;
				checkError( m_Context.EnumerateProductionTrees( XN_NODE_TYPE_IMAGE, NULL, imageNodes, NULL ),
						   "_2RealIMplOpenNI::start Error when enumerating image nodes\n" );
				
				//iterate through all image-generators
				xn::NodeInfoList::Iterator iter = imageNodes.Begin();
				for( int i = 0 ; iter != imageNodes.End(); ++iter, ++i )
				{
					//break if there are more image gen-nodes than devices in array
					if( i >= m_NumDevices )
						break;
                    
					xn::NodeInfo nodeinfo = *iter; //hmm...
					// in theory this should work, but ...
					m_Devices[i]->startupProcessingColorGenerator( nodeinfo, configureImages );
				}
			}

			//PROCESSING DEPTH GENERATORS -------------------------------------------------------------------------->
			if( startGenerators & DEPTHIMAGE )
			{
				//fetching all depthGen-nodes
				xn::NodeInfoList depthNodes;
				checkError( m_Context.EnumerateProductionTrees( XN_NODE_TYPE_DEPTH, NULL, depthNodes, NULL ),
						   "_2RealImplOpenNI::start Error when enumerating depth nodes\n" );
				
				//iterate through all depth-generators
				xn::NodeInfoList::Iterator iter = depthNodes.Begin();
				for( int i = 0 ; iter != depthNodes.End(); ++iter, ++i )
				{
					//break if there are more depthNodes than devices in array
					if( i >= m_NumDevices )
						break;
                    xn::NodeInfo nodeinfo = *iter;
					m_Devices[i]->startupProcessingDepthGenerator( nodeinfo, configureImages );
				}
			}

			if( startGenerators & COLORIMAGE && startGenerators & INFRAREDIMAGE )
				_2REAL_LOG(warn) << "_2Real: Cannot start color and infraredgenerator at same time! disabling infrared. change flags in start()!" << std::endl;

			//PROCESSING INFRARED GENERATORS------------------------------------------------------------------------>
			if( !( startGenerators & COLORIMAGE ) && startGenerators & INFRAREDIMAGE )
			{
				//fetching infrared-generator-nodes
				xn::NodeInfoList irNodes;
				checkError( m_Context.EnumerateProductionTrees( XN_NODE_TYPE_IR, NULL, irNodes, NULL ),
						   "_2RealIMplOpenNI::start Error when enumerating infrared nodes\n" );
								
				//iterate through all infrared-generators
				xn::NodeInfoList::Iterator iter = irNodes.Begin();
				for( int i = 0 ; iter != irNodes.End(); ++iter, ++i )
				{
					//break if there are more infrared nodes than devices in array
					if( i >= m_NumDevices )
						break;
                    xn::NodeInfo nodeinfo = *iter;
					m_Devices[i]->startupProcessingInfraredGenerator( nodeinfo, configureImages );
				}
			}

			//PROCESSING USER GENERATORS --------------------------------------------------------------------------->
			if( startGenerators & USERIMAGE || startGenerators & USERIMAGE_COLORED )
			{
				//fetching all user-generator-nodes
				xn::NodeInfoList userNodes;
				checkError( m_Context.EnumerateProductionTrees( XN_NODE_TYPE_USER, NULL, userNodes, NULL ),
						   "_2RealIMplOpenNI::start Error when enumerating user nodes\n" );
				
				//iterate through all user-generators
				xn::NodeInfoList::Iterator iter = userNodes.Begin();

				for( int i = 0 ; iter != userNodes.End(); ++iter, ++i )
				{
					//break if there are more user nodes than devices in array
					if( i >= m_NumDevices )
						break;
                    xn::NodeInfo nodeinfo = *iter;
					m_Devices[i]->startupProcessingUserGenerator( nodeinfo, configureImages );
				}
			}

			//finally starting all generators----------------------------------------------------------------------->
			for( int i = 0 ; i < m_NumDevices; ++i )
			{
				if( !m_Devices[i]->startGenerators( startGenerators ) )
					return false;
			}

			//success!!! =)
			return ( m_IsInitialized = true );
		}

		virtual void setMirrored( const uint32_t deviceID, _2RealGenerator type, bool flag )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::setMirrored() Error, deviceID out of bounds!" );

			switch( type )
			{
			case COLORIMAGE:
				if( !m_Devices[deviceID]->GetOpenNIImageGenerator().IsValid() )
				{
					_2REAL_LOG(warn) << "_2Real: Error OpenNIImpl::setMirrored() cannot set capability due non activated color generator..." << std::endl;
					return;
				}
				checkError( m_Devices[deviceID]->m_ColorGenerator.setMirroring( flag ),
							"Error _2RealImplOpenNI::setMirrored() Error when trying to set mirroring for color image\n" );
				break;
			case DEPTHIMAGE:
				if( !m_Devices[deviceID]->GetOpenNIDepthGenerator().IsValid() )
				{
					_2REAL_LOG(warn) << "_2Real: Error OpenNIImpl::setMirrored() cannot set capability due non activated depth generator..." << std::endl;
					return;
				}
				checkError( m_Devices[deviceID]->m_DepthGenerator.setMirroring( flag ),
							"Error _2RealImplOpenNI::setMirrored() Error when trying to set mirroring for color image\n" );
				break;
			case INFRAREDIMAGE:
				if( !m_Devices[deviceID]->GetOpenNIInfraredGenertor().IsValid() )
				{
					_2REAL_LOG(warn) << "_2Real: Error OpenNIImpl::setMirrored() cannot set capability due non activated infrared generator..." << std::endl;
					return;
				}
				checkError( m_Devices[deviceID]->m_InfraredGenerator.setMirroring( flag ),
							"Error _2RealImplOpenNI::setMirrored() Error when trying to set mirroring for color image\n" );
				break;
			case USERIMAGE_COLORED:
			case USERIMAGE:
				break;
			default:
				throwError( "_2RealImplOpenNI::setMirrored() Error: Wrong type of generator assigned?!" );
			}
		}

		virtual bool isMirrored( const uint32_t deviceID, _2RealGenerator type ) const
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::isMirrored() Error, deviceID out of bounds!" );

			bool flag = true;
			switch( type )
			{
			case COLORIMAGE:
				if( !m_Devices[deviceID]->GetOpenNIImageGenerator().IsValid() )
				{
					_2REAL_LOG(warn) << "_2Real: OpenNIImpl::isMirrored() color generator is not avtivated..." << std::endl;
					return false;
				}
				flag = m_Devices[deviceID]->m_ColorGenerator.isMirrored();
				break;
			case DEPTHIMAGE:
				if( !m_Devices[deviceID]->GetOpenNIDepthGenerator().IsValid() )
				{
					_2REAL_LOG(warn) << "_2Real: OpenNIImpl::isMirrored() depth generator is not avtivated..." << std::endl;
					return false;
				}
				flag = m_Devices[deviceID]->m_DepthGenerator.isMirrored();
				break;
			case INFRAREDIMAGE:
				if( !m_Devices[deviceID]->GetOpenNIInfraredGenertor().IsValid() )
				{
					_2REAL_LOG(warn) << "_2Real:OpenNIImpl::isMirrored() infrared generator is not avtivated..." << std::endl;
					return false;
				}
				flag = m_Devices[deviceID]->m_InfraredGenerator.isMirrored();
				break;
			case USERIMAGE_COLORED:
			case USERIMAGE:
				return false;
				break;
			default:
				throwError( "_2RealImplOpenNI::isMirrored() error: Wrong type of generator assigned?!" );
			}
			return flag;
		}

		virtual bool isJointAvailable( _2RealJointType type ) const
		{
			if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2RealImplOpenNI::isJointAvailable() error, joint id out of bounds!" );

			switch( int( type ) )
			{
			case JOINT_LEFT_COLLAR:
			case JOINT_LEFT_WRIST:
			case JOINT_LEFT_FINGERTIP:
			case JOINT_RIGHT_COLLAR:
			case JOINT_RIGHT_WRIST:
			case  JOINT_RIGHT_FINGERTIP:
			case JOINT_LEFT_ANKLE:
			case JOINT_RIGHT_ANKLE:
			case JOINT_WAIST:
				return false;
			default:
				return true;
			}
		}

		virtual void resetSkeleton( const uint32_t deviceID, const uint32_t id )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::resetSkeleton() Error, deviceID out of bounds!" );

			if ( m_Devices[deviceID]->m_UserGenerator.isGenerating() )
			{
				m_Devices[deviceID]->m_UserGenerator.forceResetUser( id );
			}
		}

		virtual void resetAllSkeletons()
		{
			for ( int i = 0; i < m_NumDevices; ++i )
			{
				if ( m_Devices[i]->m_UserGenerator.isGenerating() )
				{
					m_Devices[i]->m_UserGenerator.forceResetUsers();
				}
			}
		}

		virtual bool shutdown()
		{
			//freeing memory
			if( m_IsInitialized )
			{
				m_IsInitialized = false;
				_2REAL_LOG(info) << "_2Real: Shutting down system...";
				//unlocking and stopping all generators
				for ( int i = 0; i < m_NumDevices; ++i )
				{
					m_Devices[i]->shutdown();
				}
				//releasing context object
				m_Context.StopGeneratingAll();
				m_Context.Release();

				//freeing devices
				for( int u = 0; u < m_NumDevices; ++u )
				{
					OpenNIDevice* device = m_Devices[u];
					delete device;
				}
				delete [] m_Devices;
				m_Devices = NULL;

				m_NumDevices = 0;

				_2REAL_LOG(info) << "OK" << std::endl;
				return true;
			}
			_2REAL_LOG(info) << std::endl << "_2Real: System not initialized..." << std::endl;
			return false;
		}

		virtual unsigned char* getImageData( const uint32_t deviceID, _2RealGenerator type, bool waitAndBlock, const uint8_t userId )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getImageData() error, deviceID out of bounds!" );

			bool colorUserImage = false;
			//refreshing user data
			OpenNIDevice* device = m_Devices[deviceID];

			switch( type )
			{
			case COLORIMAGE:
				{
					if( !device->GetOpenNIImageGenerator().IsValid() )
					{
						_2REAL_LOG(warn) << "_2Real: OpenNIImpl::getImageData() color generator is not activated! Cannot fetch image..." << std::endl;
						return NULL;
					}
					//fetching new image data
					return device->getImageBuffer();
				}
			case DEPTHIMAGE:
				{
					if( !device->GetOpenNIDepthGenerator().IsValid() )
					{
						_2REAL_LOG(warn) << "_2Real: OpenNIImpl::getImageData() depth generator is not activated! Cannot fetch image..." << std::endl;
						return NULL;
					}
					return device->getDepthBuffer();
				}
			case INFRAREDIMAGE:
				{
					if( !device->GetOpenNIInfraredGenertor().IsValid() )
					{
						_2REAL_LOG(warn) << "_2Real: OpenNIImpl::getImageData() infrared generator is not activated! Cannot fetch image..." << std::endl;
						return NULL;
					}
					return device->getInfraredBuffer();
				}
			case USERIMAGE_COLORED:
				colorUserImage = true;
			case USERIMAGE:
				{
					if( !device->GetOpenNIUserGenerator().IsValid() )
					{
						_2REAL_LOG(warn) << "_2Real: OpenNIImpl::getImageData() user generator is not activated! Cannot fetch image..." << std::endl;
						return NULL;
					}
					if( colorUserImage )
						return device->getUserColorImageBuffer();
					else
						return device->getUserImageBuffer();

				}
			default:
				throwError( "_2RealImplOpenNI::getImageData() Error: Wrong type of generator assigned?!" );
			}
			return NULL;
		}

		virtual uint16_t* getImageDataDepth16Bit( const uint32_t deviceID, bool waitAndBlock=false)
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getDepthImageData16Bit() Error, deviceID out of bounds!" );
			if( !m_Devices[deviceID]->GetOpenNIDepthGenerator().IsValid() )
			{
				_2REAL_LOG(warn) << "_2Real: OpenNIImpl::getDepthImageData16Bit() depth generator is not activated! Cannot fetch image..." << std::endl;
				return NULL;
			}

			return m_Devices[deviceID]->getDepthBuffer_16bit();
		}

		virtual uint32_t getBytesPerPixel( _2RealGenerator type ) const
		{
			if( type == COLORIMAGE || type == USERIMAGE_COLORED ) //rgb image 3byte/Pixel
				return 3;
			return 1; //depth-, and userimage will be converted to 1byte/Pixel (8bit uchar*)
		}

		virtual uint32_t getImageWidth( const uint32_t deviceID, _2RealGenerator type )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getImageWidth Error, deviceID out of bounds!" );

			switch( type )
			{
			case COLORIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeColor().nXRes;
				}
			case DEPTHIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeDepth().nXRes;
				}
			case INFRAREDIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeInfrared().nXRes;
				}
			case USERIMAGE_COLORED:
			case USERIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeUser().nXRes;
				}
			default:
				throwError( "_2RealImplOpenNI::getImageWidth() Error: Wrong type of generator assigned?!" );
			}
			return NULL;
		}

		virtual uint32_t getImageHeight( const uint32_t deviceID, _2RealGenerator type )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getImageHeight() Error, deviceID out of bounds!" );

			switch( type )
			{
			case COLORIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeColor().nYRes;
				}
			case DEPTHIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeDepth().nYRes;
				}
			case INFRAREDIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeInfrared().nYRes;
				}
			case USERIMAGE_COLORED:
			case USERIMAGE:
				{
					return (uint32_t)m_Devices[deviceID]->getOutputmodeUser().nYRes;
				}
			default:
				throwError( "_2RealImplOpenNI::getImageHeight() Error: Wrong type of generator assigned?!" );
			}
			return NULL;
		}

		virtual uint32_t getNumberOfDevices() const
		{
			return m_NumDevices;
		}

		virtual const _2RealVector3f getJointWorldPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getJointWorldPosition() Error, deviceID out of bounds!" );

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID < m_TrackedUserVector.size() )
				return m_TrackedUserVector[userID].getJointWorldPosition( type );
			throwError( "_2RealImplOpenNI:getJointWorldPosition() Error, userID out of bounds!");
		}

		virtual const _2RealPositionsVector3f& getSkeletonWorldPositions( const uint32_t deviceID, const uint8_t userID )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getSkeletonWorldPositions() Error, deviceID out of bounds!" );

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID < m_TrackedUserVector.size() )
				return m_TrackedUserVector[userID].getSkeletonWorldPositions();
			throwError( "_2RealImplOpenNI:getSkeletonWorldPositions() Error, userID out of bounds!");
		}

		virtual const _2RealVector2f getJointScreenPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getJointScreenPosition() Error, deviceID out of bounds!" );

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID < m_TrackedUserVector.size() )
				return m_TrackedUserVector[userID].getJointScreenPosition( type );
			throwError( "_2RealImplOpenNI:getJointScreenPosition() Error, userID out of bounds!");
		}

		virtual const _2RealPositionsVector2f& getSkeletonScreenPositions( const uint32_t deviceID, const uint8_t userID )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getSkeletonScreenPositions() Error, deviceID out of bounds!" );

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID < m_TrackedUserVector.size() )
				return m_TrackedUserVector[userID].getSkeletonScreenPositions();
			throwError( "_2RealImplOpenNI:getSkeletonScreenPositions() error, userID out of bounds!");
		}

		virtual const _2RealOrientationsMatrix3x3& getSkeletonWorldOrientations( const uint32_t deviceID, const uint8_t userID )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getSkeletonWorldOrientations() Error, deviceID out of bounds!" );

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID < m_TrackedUserVector.size() )
				return m_TrackedUserVector[userID].getSkeletonWorldOrientations();
			throwError( "_2RealImplOpenNI:getSkeletonWorldOrientations() error, userID out of bounds!");
		}

		virtual const _2RealMatrix3x3 getJointWorldOrientation( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::getJointWorldOrientation() Error, deviceID out of bounds!" );

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID < m_TrackedUserVector.size() )
				return m_TrackedUserVector[userID].getJointWorldOrientation( type );
			throwError( "_2RealImplOpenNI:getJointWorldOrientation() Error, userID out of bounds!");
		}

		virtual const uint32_t getNumberOfUsers( const uint32_t deviceID ) const
		{
			checkDeviceRunning(deviceID, "2RealImplOpenNI::getNumberOfUsers()");
			return m_Devices[deviceID]->getUsers().size(); //awesome!!! :) -> terror by robz
		}

		virtual const uint32_t getNumberOfSkeletons( const uint32_t deviceID ) const
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::setAlignColorDepthImage()");
			return m_Devices[deviceID]->getUsers().size(); //awesome!!! :) -> terror by robz
		}

		virtual void setAlignColorDepthImage( const uint32_t deviceID, bool flag )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::setAlignColorDepthImage()");
			xn::DepthGenerator& dg = m_Devices[deviceID]->GetOpenNIDepthGenerator();
			if( dg.IsValid() &&
				m_Devices[deviceID]->GetOpenNIImageGenerator().IsValid() &&
				dg.IsCapabilitySupported( XN_CAPABILITY_ALTERNATIVE_VIEW_POINT ) )
			{
				if( flag ) //enabling
				{
					checkError( dg.GetAlternativeViewPointCap().ResetViewPoint(),
								 "Error _2RealImplOpenNI::setAlignColorDepthImage() Error when trying to set color to depth alignment\n" );

					checkError( dg.GetAlternativeViewPointCap().SetViewPoint( m_Devices[deviceID]->GetOpenNIImageGenerator() ),
								 "Error _2RealImplOpenNI::setAlignColorDepthImage() Error when trying to set color to depth alignment\n" );
				}
				else
					checkError( dg.GetAlternativeViewPointCap().ResetViewPoint(),
								 "Error _2RealImplOpenNI::setAlignColorDepthImage() Error when trying to set color to depth alignment\n" );
			}
			_2REAL_LOG(warn) << "_2Real: _2RealOpenNI::setAlignColorDepthImage() cannot execute, because depth or colorgenerator isnt initialized properly!" << std::endl;
		}

		virtual bool hasFeatureJointOrientation() const
		{
			return true;
		}

		virtual bool restart()
		{
			_2REAL_LOG(info) << std::endl << "_2Real: Shutting system down..." << std::endl;
			shutdown();
			boost::this_thread::sleep(boost::posix_time::seconds((long)3)); //preventing reinitialization to fast (wait 3 sec)
			_2REAL_LOG(info) << "_2Real: Restarting system..." << std::endl;
			return start( m_GeneratorConfig, m_ImageConfig );
		}

		virtual void convertProjectiveToWorld( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inProjective, _2RealVector3f* outWorld )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::convertWorldToProjective()");
				throwError( "_2RealImplOpenNI::convertProjectiveToWorld() Error, deviceID out of bounds!" );

			if( m_Devices[deviceID]->GetOpenNIDepthGenerator().IsValid() )
				xnConvertProjectiveToRealWorld( m_Devices[deviceID]->GetOpenNIDepthGenerator().GetHandle(), coordinateCount, (XnPoint3D*)inProjective, (XnPoint3D*)outWorld );
			else
				_2REAL_LOG(warn) << "_2Real: ConvertProjectivToWorld, depth generator of device id: " << deviceID << " is not enabled!" << std::endl;
		}

		virtual void convertWorldToProjective( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inWorld, _2RealVector3f* outProjective )
		{
			checkDeviceRunning(deviceID, "_2RealImplOpenNI::convertWorldToProjective()" );

			if( m_Devices[deviceID]->GetOpenNIDepthGenerator().IsValid() )
				xnConvertRealWorldToProjective( m_Devices[deviceID]->GetOpenNIDepthGenerator().GetHandle(), coordinateCount, (XnPoint3D*)inWorld, (XnPoint3D*)outProjective );
			else
				_2REAL_LOG(warn) << "_2Real: ConvertWorldToProjective, depth generator of device id: " << deviceID << " is not enabled!" << std::endl;
		}

		virtual void setLogLevel(_2RealLogLevel iLevel)
		{
			_2RealLogger::getInstance().setLogLevel(iLevel);
		};

		virtual void setLogOutputStream(std::ostream* outStream)
		{
			_2RealLogger::getInstance().setLogOutputStream(outStream);
		};

	private:

		void checkError( XnStatus status, std::string strError ) const
		{
			if ( status != XN_STATUS_OK )
				throwError( strError );
		}

		void checkDeviceRunning(uint8_t deviceID, std::string strError) const
		{
			//checking id
			if ( deviceID > (m_NumDevices - 1) )
				throwError(strError + " error, deviceID out of bounds!\n");
		}
};

}
#endif
