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

	CADET - Center for Advances in Digital Entertainment Technologies

	Authors: Robert Praxmarer, Gerlinde Emsenhuber, Robert Sommeregger
	Email: support@cadet.at
	Created: 08-09-2011
*/

#pragma once
#include "_2RealUtility.h"
#include <Windows.h>
#include "NuiApi.h"
#include <iostream>
#include <stdint.h>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/condition.hpp>
#include "WSDKDeviceConfiguration.h"
#include "_2RealTypes.h"


namespace _2RealKinectWrapper
{


/*!
* Representing a kinect sensors functionality
*/
class WSDKDevice
{

	public:
		WSDKDevice( INuiSensor* devicePtr, const std::string& name );
		virtual ~WSDKDevice(void);

		struct BGRX
		{
			uchar		b, g, r, x;
		};

		struct RGB
		{
			uchar		r, g, b;
		};


		bool							isMirroringColor() const;
		bool							isMirroringDepth() const;
		bool							isMirroringUser() const;
		const bool						isNewData(_2RealGenerator type) const;
		bool							isDeviceStarted() const;
		bool							isDeviceShutDown() const;

		void  		getUsers( bool waitForNewData, _2RealTrackedUserVector& out );
		boost::shared_array<uchar>		getColorImageBuffer( bool waitForNewData );
		boost::shared_array<uchar>		getDepthImageBuffer( bool waitForNewData );
		boost::shared_array<uchar>		getUserImageBuffer( bool waitForNewData );
		boost::shared_array<uchar>		getColoredUserImageBuffer( bool waitForNewData );
		boost::shared_array<uint16_t>	getDepthImageBuffer16Bit( bool waitForNewData );
		int								getMotorAngle();
		WSDKDeviceConfiguration&		getDeviceConfiguration() const;
		unsigned int					getNumberOfUsers();

		bool							setMotorAngle(int angle);
		void							setMirroringColor( const bool flag );
		void							setMirroringDepth( const bool flag );
		void							setMirroringUser( const bool flag );
		void							start();
		void							startGenerator( uint32_t generators );
		/*! /brief     Stopping Generators + Worker-Thread for this device
			/param     const bool shutdown - False(Default): Stop Generators Color, Depth, Skeleton; True: Stop Generators + Worker Thread and closing handles
		*/
		void							stop( const bool shutdown = false );
		void							stopGenerator( uint32_t generators );
		void							shutdown();

		// Member
		//image measurements - copy from m_configuration
		const uint16_t					&m_WidthImageColor, &m_WidthImageDepthAndUser;
		const uint16_t					&m_HeightImageColor, &m_HeightImageDepthAndUser;


	private:
		static DWORD WINAPI							threadEventsFetcher( LPVOID pParam );
		void										initColorStream();
		void										initDepthStream();
		void										initUserDepthStream();
		void										processColorImageEvent();
		void										processDepthImageEvent();
		void										processSkeletonEvent();
		_2RealTrackedJoint_sptr						getJoint( _2RealJointType type, _NUI_SKELETON_POSITION_INDEX nuiType, const NUI_SKELETON_DATA& data );
								

	// Member
		INuiSensor*		m_pNuiSensor;
		std::string							m_Name;
		int									m_DeviceID;
		mutable WSDKDeviceConfiguration		m_Configuration;

		//events
		HANDLE								m_EventColorImage;
		HANDLE								m_EventDepthImage;
		HANDLE								m_EventSkeletonData;
		HANDLE								m_EventStopThread;
		//stream handles
		HANDLE								m_HandleColorStream;
		HANDLE								m_HandleDepthStream;
		HANDLE								m_HandleThread;

		bool								m_isDeviceStarted;
		bool								m_isDeviceShutDown;
		bool								m_bIsDepthOnly;
		bool								m_bIsMirroringColor;
		bool								m_bIsMirroringDepth;
		bool								m_bIsMirroringUser;
		bool								m_bIsDeletingDevice;

		bool								m_bIsNewColorData;
		bool								m_bIsNewDepthData;
		bool								m_bIsNewUserData;
		bool								m_bIsNewSkeletonData;
		bool								m_bIsNewInfraredData;

		// used to handle kinect events in separate thread
		// use this enum to lookup m_WTEvents
		enum WorkerThreadEvents
		{	
			WT_STOP_THREAD = 0,
			WT_EVENT_COLOR,
			WT_EVENT_DEPTH,
			WT_EVENT_SKELETON
		};
		HANDLE								m_WTEvents[4];

		//sync
		boost::mutex						m_MutexImage;
		boost::mutex						m_MutexDepth;
		boost::mutex						m_MutexUser;
		boost::mutex						m_MutexColoredUser;
		boost::mutex						m_MutexFetchUser;
		boost::mutex						m_MutexFetchColorImage;
		boost::mutex						m_MutexFetchDepthImage, m_MutexFetchDepthImage2, m_MutexFetchDepthImage3;
		boost::condition					m_NotificationNewColorImageData;
		boost::condition					m_NotificationNewDepthImageData;
		boost::condition					m_NotificationNewUserdata;

		//image buffers
		boost::shared_array<uchar>			m_ImageColor_8bit;
		boost::shared_array<uchar>			m_ImageDepth_8bit;
		boost::shared_array<uchar>			m_ImageUser_8bit;
		boost::shared_array<uchar>			m_ImageColoredUser_8bit;
		boost::shared_array<uint16_t>		m_ImageDepth_16bit;

		_2RealTrackedUserVector				m_Users;
};

}

