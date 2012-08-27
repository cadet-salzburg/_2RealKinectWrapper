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
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/shared_array.hpp"
#include "I_2RealImplementation.h"
#include "_2RealTypes.h"
#include "XnOpenNI.h"
#include "OpenNISpecific/OpenNIDevice.h"
#include <iostream>
#include <istream>
#include "_2RealUtility.h"

namespace _2RealKinectWrapper
{
	typedef std::vector<XnPredefinedProductionNodeType> RequestedNodeVector;
	boost::mutex						m_MutexSyncProcessUsers;
class _2RealImplementationOpenNI : public I_2RealImplementation
{

	private:
		xn::Context															m_Context;
		xn::NodeInfoList													m_DeviceInfo;
		uint8_t																m_NumDevices;
		std::vector<boost::shared_ptr<OpenNIDevice> >						m_Devices;
		uint32_t															m_GeneratorConfig;
		uint32_t															m_ImageConfig;
		bool		 														m_IsInitialized;
		_2RealTrackedUserVector												m_TrackedUserVector;
		bool																m_ShouldUpdate;
		bool																m_StopRequested;
		boost::thread														m_ProcessingThread;
		virtual void initialize()
		{
			_2REAL_LOG(info) << "\n_2Real: Init OpenNI SDK " + std::string(XN_VERSION_STRING);
			checkError( m_Context.Init(), "\n_2Real: Error Could not Initialize OpenNI Context ...\n" );
			checkError( m_Context.EnumerateProductionTrees( XN_NODE_TYPE_DEVICE, NULL, m_DeviceInfo, NULL ), "\n_2Real: Error while enumerating available devices ...\n" );
  			xn::NodeInfoList::Iterator deviceIter = m_DeviceInfo.Begin();
			for ( ; deviceIter!=m_DeviceInfo.End(); ++deviceIter )
			{
				std::stringstream deviceName;
				deviceName << "Device_" << m_NumDevices;
				NodeInfoRef devInfo = NodeInfoRef( new xn::NodeInfo( *deviceIter ) );
				boost::shared_ptr<OpenNIDevice> deviceRef( new OpenNIDevice( m_Context, devInfo,deviceName.str() ) );
				m_Devices.push_back( deviceRef );
				deviceRef->addDeviceToContext();
				m_NumDevices += 1;
			}
			_2REAL_LOG(info) << "\n_2Real: Found and init " << (int) m_NumDevices << " device[s]" << std::endl;
			//Add threaded update
			m_ProcessingThread = boost::thread( boost::bind( &_2RealImplementationOpenNI::update, this ), this );
			m_IsInitialized = true;
		}

		virtual RequestedNodeVector getRequestedNodes( uint32_t startGenerators ) const
		{
			RequestedNodeVector XnRequestedNodeSet;

			if(  ( startGenerators & COLORIMAGE )  &&  ( startGenerators & INFRAREDIMAGE )  )
			{
				_2REAL_LOG(warn) << "_2Real: Cannot have color and infrared generators at same time!" << std::endl;
			}

			if( startGenerators & COLORIMAGE )
			{
				XnRequestedNodeSet.push_back( XN_NODE_TYPE_IMAGE );
			}
			if( startGenerators & DEPTHIMAGE )
			{
				XnRequestedNodeSet.push_back( XN_NODE_TYPE_DEPTH );
			}

			if( startGenerators & USERIMAGE || startGenerators & USERIMAGE_COLORED )
			{
				XnRequestedNodeSet.push_back( XN_NODE_TYPE_USER );
			}

			if( !( startGenerators & COLORIMAGE ) && startGenerators & INFRAREDIMAGE )
			{
				XnRequestedNodeSet.push_back( XN_NODE_TYPE_IR );
			}

			return XnRequestedNodeSet;
		}

		virtual void setGeneratorState( const uint32_t deviceID, uint32_t requestedGenerator, bool start )
		{
			if ( deviceID + 1 > m_NumDevices  )
			{
				throwError("_2Real: Error, deviceID out of bounds!\n");
			}
			std::vector<XnPredefinedProductionNodeType> requestedNodes = getRequestedNodes( requestedGenerator );
			if ( requestedNodes.empty() ) 
			{
				return;
			}

			for ( std::vector<XnPredefinedProductionNodeType>::iterator iter = requestedNodes.begin(); iter!=requestedNodes.end(); ++iter ) 
			{
				if ( start ){
					m_Devices[ deviceID ]->startGenerator(*iter);
				} else {
					m_Devices[ deviceID ]->stopGenerator(*iter);
				}
			}
		}

		 void update()
		 {
			 while ( !m_StopRequested )
			 {
				 if ( m_ShouldUpdate )
				 {
					 m_MutexSyncProcessUsers.lock();
					 checkError( m_Context.WaitNoneUpdateAll(), "_2Real: Error while trying to update context." );
					 boost::this_thread::sleep(boost::posix_time::milliseconds(1));
					 m_MutexSyncProcessUsers.unlock();
				 }
			 }
		};

	public:
		_2RealImplementationOpenNI()
			:m_NumDevices( 0 ),
			 m_IsInitialized( false ),
			 m_GeneratorConfig( CONFIG_DEFAULT ),
			 m_ImageConfig( IMAGE_CONFIG_DEFAULT ),
			 m_ShouldUpdate( false ),
			 m_StopRequested( false )
		{
			initialize();
		}

		~_2RealImplementationOpenNI()
		{
			shutdown();
		}

		//virtual bool start( uint32_t startGenerators, uint32_t configureImages )
		virtual bool configureDevice( const uint32_t deviceID,  uint32_t startGenerators, uint32_t configureImages )
		{
			m_GeneratorConfig	= startGenerators;
			m_ImageConfig		= configureImages;
			RequestedNodeVector requestedNodes		 =  getRequestedNodes( m_GeneratorConfig );
			for ( RequestedNodeVector::iterator iter = requestedNodes.begin(); iter!=requestedNodes.end(); ++iter ) 
			{
				m_Devices[ deviceID ]->addGenerator( *iter, m_ImageConfig );
			}
			return true;
		}

		virtual void startGenerator( const uint32_t deviceID, uint32_t configureGenerators )
		{
			setGeneratorState( deviceID, configureGenerators, true );
			m_ShouldUpdate = true;
		}

		virtual void stopGenerator( const uint32_t deviceID, uint32_t configureGenerators )
		{
			setGeneratorState( deviceID, configureGenerators, false );
		}

		virtual void addGenerator( const uint32_t deviceID, uint32_t configureGenerators, uint32_t configureImages )
		{
			m_MutexSyncProcessUsers.lock();
			RequestedNodeVector requestedNodes = getRequestedNodes( configureGenerators );
			for ( RequestedNodeVector::iterator iter = requestedNodes.begin(); iter!=requestedNodes.end(); ++iter ) 
			{
				m_Devices[ deviceID ]->addGenerator( *iter, configureImages );
			}
			m_MutexSyncProcessUsers.unlock();
		}

		virtual void removeGenerator( const uint32_t deviceID, uint32_t configureGenerators )
		{
			m_MutexSyncProcessUsers.lock();
			RequestedNodeVector requestedNodes = getRequestedNodes( configureGenerators );
			for ( RequestedNodeVector::iterator iter = requestedNodes.begin(); iter!=requestedNodes.end(); ++iter ) 
			{
				m_Devices[ deviceID ]->removeGenerator( *iter );
			}
			m_MutexSyncProcessUsers.unlock();
		}

		virtual bool generatorIsActive( const uint32_t deviceID, _2RealGenerator type )
		{
			checkDeviceRunning(deviceID);
			RequestedNodeVector requestedNodes =  getRequestedNodes( type );
			return m_Devices[deviceID]->generatorIsActive( requestedNodes[0] );
		}
		virtual void setMirrored( const uint32_t deviceID, _2RealGenerator type, bool flag )
		{
			checkDeviceRunning(deviceID);
			RequestedNodeVector requestedNodes =  getRequestedNodes( type );
			if ( requestedNodes[0] == XN_NODE_TYPE_USER )
			{ 
				//OpenNI cannot mirror User Image.
				return;
			}

			if( !m_Devices[ deviceID ]->hasGenerator( requestedNodes[0] ) )
			{
				_2REAL_LOG(warn) << "_2Real: Cannot set mirror capability due to non activated generator..." << std::endl;
				return;
			}
			xn::Generator gen;
			m_Devices[ deviceID ]->getExistingProductionNode( requestedNodes[0], gen );

			//checkError( gen.GetMirrorCap().SetMirror( flag ), "Error when trying to set mirroring for image\n" );

			try 
			{
				checkError( gen.GetMirrorCap().SetMirror( flag ), "Error when trying to set mirroring for image\n" );
			} 
			catch (_2RealException& e )
			{
				std::cout << e.what() << std::endl;
			}		
		}

		virtual bool isMirrored( const uint32_t deviceID, _2RealGenerator type ) const
		{
			checkDeviceRunning( deviceID );
            XnBool flag = TRUE;
			RequestedNodeVector requestedNodes = getRequestedNodes( type );
			if ( requestedNodes.empty() )
			{
				throwError( "_2Real::setMirrored() Error: wrong type of generator provided!");
			} else if ( requestedNodes.size() > 1 ) {
				_2REAL_LOG(warn) << "_2Real: isMirrored() doesn't accept combinations of _2RealGenerator types..." << std::endl;
				return false;
			} 

			if( !m_Devices[ deviceID ]->hasGenerator( requestedNodes[0] ) )
			{
				_2REAL_LOG(warn) << "_2Real: Cannot check mirror capability due to non activated generator..." << std::endl;
				return false;
			}

			if ( requestedNodes[0] != XN_NODE_TYPE_USER )
			{
				xn::Generator gen;
				m_Devices[ deviceID ]->getExistingProductionNode( requestedNodes[0], gen );
				flag = gen.GetMirrorCap().IsMirrored();
			}
			return !!flag; //yay! :p
		}

		virtual bool isJointAvailable( _2RealJointType type ) const
		{
			if( int( type ) < 0 || int( type ) > _2REAL_NUMBER_OF_JOINTS - 1 )
			throwError( "_2Real: isJointAvailable() error, joint id out of bounds!" );

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

		virtual const bool isNewData(const uint32_t deviceID, _2RealGenerator type) const
		{
			RequestedNodeVector requestedNodes =  getRequestedNodes( type );
			xn::Generator generator;
			m_Devices[deviceID]->getExistingProductionNode( requestedNodes[0], generator );
			XnBool newData = generator.IsDataNew();
			return !!newData;
		}

		virtual void resetSkeleton( const uint32_t deviceID, const uint32_t id )
		{
			checkDeviceRunning( deviceID );
			m_Devices[deviceID]->forceResetUser( id );
		}

		virtual void resetAllSkeletons()
		{
			for ( size_t  deviceID=0; deviceID < m_NumDevices; ++deviceID )
			{
				m_Devices[ deviceID ]->forceResetUsers();
			}
		}
		virtual bool shutdown()
		{
			m_StopRequested = true;
			m_ProcessingThread.join();
			return false;

			if( m_IsInitialized )
			{
				_2REAL_LOG( info )  << std::endl << "_2Real: Shutting down system..." << std::endl;
				m_StopRequested = true;
				m_ProcessingThread.join();
				m_IsInitialized = false;
				_2REAL_LOG( info ) << "OK" << std::endl;
				return true;
			}
			_2REAL_LOG( warn ) << std::endl << "_2Real: System not shutdown correctly..." << std::endl;
			return false;


		}
		virtual boost::shared_array<unsigned char> getImageData( const uint32_t deviceID, _2RealGenerator type, bool waitAndBlock, const uint8_t userId )
		{
			//boost::mutex::scoped_lock lock(m_MutexSyncProcessUsers);
			checkDeviceRunning( deviceID );
			RequestedNodeVector requestedNodes = getRequestedNodes( type );
			if ( requestedNodes.size() > 1 )
			{
				throwError( "_2Real:: getImageData() Error: wrong type of generator provided!");
			} else if ( requestedNodes.empty() )
			{
				throwError( "_2Real:: getImageData() Error: WTF!");
			}

			if ( !m_Devices[ deviceID ]->hasGenerator( requestedNodes[0] )  )
			{
				_2REAL_LOG(warn) << "_2Real: getImageData()  Generator is not activated! Cannot fetch image..." << std::endl;
				return boost::shared_array<unsigned char>(); 
			}
			ImageDataRef imageBuffer = m_Devices[ deviceID ]->getBuffer( requestedNodes[0] );
			return imageBuffer;
		}

		virtual boost::shared_array<uint16_t> getImageDataDepth16Bit( const uint32_t deviceID, bool waitAndBlock=false)
		{
			//boost::mutex::scoped_lock lock(m_MutexSyncProcessUsers);
			checkDeviceRunning( deviceID );
			ImageData16Ref imgBuffer16 = m_Devices[deviceID]->getBuffer16( XN_NODE_TYPE_DEPTH );
			return imgBuffer16;
		}

		virtual uint32_t getBytesPerPixel( _2RealGenerator type ) const
		{
			if( type == COLORIMAGE || type == USERIMAGE_COLORED ) //rgb image 3byte/Pixel
				return 3;
			return 1; //depth-, and userimage will be converted to 1byte/Pixel (8bit uchar*)
		}

		virtual void setResolution( const uint32_t deviceID, _2RealGenerator type, unsigned int hRes, unsigned int vRes )
		{
			checkDeviceRunning( deviceID );
			RequestedNodeVector requestedNodes = getRequestedNodes( type );
			if ( requestedNodes.size() > 1 )
			{
				throwError( "_2Real:: getImageData() Error: wrong type of generator provided!");
			} else if ( requestedNodes.empty() )
			{
				throwError( "_2Real:: getImageData() Error: WTF!");
			}

			if ( !m_Devices[ deviceID ]->hasGenerator( requestedNodes[0] )  )
			{
				_2REAL_LOG(warn) << "_2Real: getImageData()  Generator is not activated! Cannot set resolution..." << std::endl;
			}
			m_Devices[ deviceID ]->setGeneratorResolution(  requestedNodes[0], hRes, vRes );
		}

		virtual uint32_t getImageWidth( const uint32_t deviceID, _2RealGenerator type )
		{
			checkDeviceRunning(deviceID);
			RequestedNodeVector requestedNodes = getRequestedNodes( type );
			if ( requestedNodes.size() > 1 )
			{
				throwError( "_2Real:: getImageData() Error: Combinations of generators are not allowed!");
			} else if ( requestedNodes.empty() )
			{
				throwError( "_2Real:: getImageData() Error: wrong type of generator provided!");
			}

			//Only map generators have an output mode.
			if ( requestedNodes[0] == XN_NODE_TYPE_USER  )
			{	
				requestedNodes[0] = XN_NODE_TYPE_DEPTH;
			}

			if ( m_Devices[deviceID]->hasGenerator( requestedNodes[0] )  )
			{
				xn::MapGenerator gen;
				m_Devices[deviceID]->getExistingProductionNode(requestedNodes[0], gen);
				XnMapOutputMode mapMode;
				gen.GetMapOutputMode( mapMode );
				return mapMode.nXRes;
			}
			return NULL;
		}

		virtual uint32_t getImageHeight( const uint32_t deviceID, _2RealGenerator type )
		{
			checkDeviceRunning(deviceID);
			RequestedNodeVector requestedNodes = getRequestedNodes( type );
			if ( requestedNodes.size() > 1 )
			{
				throwError( "_2Real:: getImageData() Error: Combinations of generators are not allowed!");
			} else if ( requestedNodes.empty() )
			{
				throwError( "_2Real:: getImageData() Error: wrong type of generator provided!");
			}

			//Only map generators have an output mode.
			if ( requestedNodes[0] == XN_NODE_TYPE_USER  )
			{	
				requestedNodes[0] = XN_NODE_TYPE_DEPTH;
			}

			if ( m_Devices[deviceID]->hasGenerator( requestedNodes[0] )  )
			{
				xn::MapGenerator gen;
				m_Devices[deviceID]->getExistingProductionNode(requestedNodes[0], gen);
				XnMapOutputMode mapMode;
				gen.GetMapOutputMode( mapMode );
				return mapMode.nYRes;
			}
			return NULL;
		}

		virtual uint32_t getNumberOfDevices() const
		{
			return m_NumDevices;
		}

		virtual const _2RealVector3f getJointWorldPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
		{
			checkDeviceRunning(deviceID);

			return m_TrackedUserVector[userID]->getJointWorldPosition( type );
		}

		virtual const _2RealPositionsVector3f getSkeletonWorldPositions( const uint32_t deviceID, const uint8_t userID )
		{
			checkDeviceRunning(deviceID);

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID >= m_TrackedUserVector.size() )
			{
				throwError( "_2Real: getSkeletonWorldPositions() Error, userID out of bounds!");
			}
			return m_TrackedUserVector[userID]->getSkeletonWorldPositions();
		}

		virtual const _2RealVector3f getJointScreenPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
		{
			checkDeviceRunning(deviceID);
			//check userid
			if( userID >= m_TrackedUserVector.size() )
			{
				throwError( "_2Real: getJointScreenPosition() Error, userID out of bounds!");
			}
			return m_TrackedUserVector[userID]->getJointScreenPosition( type );
		}

		virtual const _2RealPositionsVector3f getSkeletonScreenPositions( const uint32_t deviceID, const uint8_t userID )
		{
			checkDeviceRunning(deviceID);

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID >= m_TrackedUserVector.size() )
			{
				throwError( "_2Real: getSkeletonScreenPositions() error, userID out of bounds!");
			}
			return m_TrackedUserVector[userID]->getSkeletonScreenPositions();
		}

		virtual const _2RealOrientationsMatrix3x3 getSkeletonWorldOrientations( const uint32_t deviceID, const uint8_t userID )
		{
			checkDeviceRunning(deviceID);

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID >= m_TrackedUserVector.size() )
			{
				throwError( "_2Real: getSkeletonWorldOrientations() error, userID out of bounds!");
			}
			return m_TrackedUserVector[userID]->getSkeletonWorldOrientations();
		}

		virtual const _2RealMatrix3x3 getJointWorldOrientation( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
		{
			checkDeviceRunning(deviceID);

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID >= m_TrackedUserVector.size() )
			{
				throwError( "_2Real: getJointWorldOrientation() Error, userID out of bounds!");
			}
			return m_TrackedUserVector[userID]->getJointWorldOrientation( type );
		}

		virtual const _2RealJointConfidence getSkeletonJointConfidence(const uint32_t deviceID, const uint8_t userID, _2RealJointType type)
		{
			checkDeviceRunning(deviceID);

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID >= m_TrackedUserVector.size() )
			{
				throwError( "getJointConfidence() Error, userID out of bounds!");
			}
			return m_TrackedUserVector[userID]->getJointConfidence( type );
		}

		virtual const _2RealJointConfidences getSkeletonJointConfidences(const uint32_t deviceID, const uint8_t userID)
		{
			checkDeviceRunning(deviceID);

			//fetching user -> always nonblocking
			m_TrackedUserVector = m_Devices[deviceID]->getUsers();

			//check userid
			if( userID >= m_TrackedUserVector.size() )
			{
				throwError( "getJointConfidence() Error, userID out of bounds!");
			}
			return m_TrackedUserVector[userID]->getJointConfidences();
		}

		virtual const uint32_t getNumberOfUsers( const uint32_t deviceID ) const
		{
			checkDeviceRunning(deviceID);
			return m_Devices[deviceID]->getNumberOfUsers();		// return number of segmented users
		}

		virtual const uint32_t getNumberOfSkeletons( const uint32_t deviceID ) const
		{
			checkDeviceRunning(deviceID);
			return m_Devices[deviceID]->getUsers().size(); //awesome!!! :) -> terror by robz
		}

		virtual void alignDepthToColor( const uint32_t deviceID, bool flag )
		{
			checkDeviceRunning(deviceID);
			m_Devices[deviceID]->alignDepthToColor(flag);
		}

		virtual bool depthIsAlignedToColor( const uint32_t deviceID )
		{
			return m_Devices[deviceID]->depthIsAlignedToColor();
		}

		virtual bool hasFeatureJointOrientation() const
		{
			return true;
		}

		virtual bool restart()
		{
			_2REAL_LOG(info) << std::endl << "_2Real: Shutting system down..." << std::endl;
			shutdown();
			boost::this_thread::sleep(boost::posix_time::seconds((long)3)); //preventing reinitialization too fast (wait 3 sec)
			_2REAL_LOG(info) << "_2Real: Restarting system..." << std::endl;
			return false;
			//return start( m_GeneratorConfig, m_ImageConfig );
		}

		virtual void convertProjectiveToWorld( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inProjective, _2RealVector3f* outWorld )
		{
			checkDeviceRunning(deviceID);
			xn::DepthGenerator depthGen;
			m_Devices[ deviceID ]->getExistingProductionNode( XN_NODE_TYPE_DEPTH, depthGen );
			if( depthGen.IsValid() )
			{
				xnConvertProjectiveToRealWorld( depthGen.GetHandle(), coordinateCount, (XnPoint3D*)inProjective, (XnPoint3D*)outWorld );
			}
			else
			{
				_2REAL_LOG(warn) << "_2Real: ConvertProjectivToWorld, depth generator of device id: " << deviceID << " is not enabled!" << std::endl;
			}
		}

		virtual void convertWorldToProjective( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inWorld, _2RealVector3f* outProjective )
		{
			checkDeviceRunning(deviceID);
			xn::DepthGenerator depthGen;
			m_Devices[ deviceID ]->getExistingProductionNode( XN_NODE_TYPE_DEPTH, depthGen );

			if( depthGen.IsValid() )
			{
				xnConvertRealWorldToProjective( depthGen.GetHandle(), coordinateCount, (XnPoint3D*)inWorld, (XnPoint3D*)outProjective );
			}
			else
			{
				_2REAL_LOG(warn) << "_2Real: ConvertWorldToProjective, depth generator of device id: " << deviceID << " is not enabled!" << std::endl;
			}
		}

		virtual bool setMotorAngle(int deviceID, int& angle)
		{
			if ( angle < -31 )
				angle = -31;
			if ( angle > 31 )
				angle = 31;
			m_Devices[ deviceID ]->setMotorAngle( angle );
			return true;
		}

		virtual int getMotorAngle(int deviceID)
		{
			return m_Devices[ deviceID ]->getMotorAngle();
		}

		virtual const _2RealVector3f getUsersWorldCenterOfMass(const uint32_t deviceID, const uint8_t userID)
		{
			checkDeviceRunning(deviceID);

			//check userid
			if( userID >= getNumberOfUsers( deviceID ) || userID > MAX_USERS)
			{
				throwError( "_2Real: getUsersCenterOfMass() Error, userID out of bounds!");
			}
			else
			{
				XnPoint3D center;
				XnUserID currentUsers[MAX_USERS];
				XnUInt16 numberOfUsers = MAX_USERS;	// is used as an input parameter for GetUser, means how many users to retrieve
				xn::UserGenerator userGen;
				m_Devices[ deviceID ]->getExistingProductionNode( XN_NODE_TYPE_USER, userGen );
				checkError( userGen.GetUsers( currentUsers, numberOfUsers ), "getCenterOfMass failed to get userIDs");
				checkError( userGen.GetCoM(currentUsers[userID], center), "Could not retrieve com");
				_2RealVector3f centerOfMass(center.X, center.Y, center.Z);
				return centerOfMass;
			}
			return _2RealVector3f();
		}

		virtual const _2RealVector3f getUsersScreenCenterOfMass(const uint32_t deviceID, const uint8_t userID)
		{
			_2RealVector3f tmp = getUsersWorldCenterOfMass(deviceID, userID);
			_2RealVector3f screen(0,0,0);
			convertWorldToProjective(deviceID, 1, &tmp, &screen);
			return screen;
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
			{
				throwError( strError + " " + xnGetStatusString(status));
			}
		}

		void checkDeviceRunning(uint8_t deviceID) const
		{
			//checking id
			if ( deviceID + 1 > m_NumDevices )
				throwError("_2Real: Error, deviceID out of bounds!\n");
		}
};

}
#endif
