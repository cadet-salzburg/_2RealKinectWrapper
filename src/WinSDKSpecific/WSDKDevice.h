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
#ifdef TARGET_MSKINECTSDK
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
* Used to check or enable/disable WSDKDevice capabilities
* To be used in WSDKDevice::isFlagEnabled(), WSDKDevice::setFlag()
*/
enum WSDKDeviceFlag
{
	DFLAG_ALIGN_COLOR_DEPTH = 1,
	DFLAG_MIRROR_COLOR = 2,
	DFLAG_MIRROR_DEPTH = 4,
	DFLAG_MIRROR_USER = 8,
	DFLAG_DEPTH_ONLY = 16
};

/*!
* Representing a kinect sensors functionality
*/
class WSDKDevice
{

	public:
		WSDKDevice( INuiSensor* devicePtr, const std::string& name );

		struct BGRX									{	uchar	b, g, r, x;	};
		struct RGB									{	uchar	r, g, b;	};

		bool										isNewData(_2RealGenerator type) const;
		inline bool									isDeviceStarted() const { return m_isDeviceStarted; }
		inline bool									isDeviceShutDown() const { return m_isDeviceShutDown; }
		/*! \brief     Check if certain capability is enabled, check one ore more flags concatenated with |
			\param     flags - use enum WSDKDeviceFlag
			\return    bool - If a flag is enabled on this device
		*/
		inline bool									isFlagEnabled( WSDKDeviceFlag flags ) const;

		void  										getUsers( bool waitForNewData, _2RealTrackedUserVector& out ) const;
		boost::shared_array<uchar>					getColorImageBuffer( bool waitForNewData );
		boost::shared_array<uchar>					getDepthImageBuffer( bool waitForNewData );
		boost::shared_array<uchar>					getUserImageBuffer( bool waitForNewData );
		boost::shared_array<uchar>					getColoredUserImageBuffer( bool waitForNewData );
		boost::shared_array<uint16_t>				getDepthImageBuffer16Bit( bool waitForNewData );
		int											getMotorAngle() const;
		WSDKDeviceConfiguration&					getDeviceConfiguration();
		const WSDKDeviceConfiguration&				getDeviceConfiguration() const;

		unsigned int								getNumberOfUsers() const;

		bool										setMotorAngle( int angle );
		inline void									setFlag( const WSDKDeviceFlag flags, const bool setEnabled ); 	/*!< Switch certain capability with one or more flags concatenated with | */

		void										start(); 	/*!< Use this function to initially start sensor with wsdkdeviceconfiguration */
		void										startGenerator( uint32_t generators );	/*!< Use this to reenable a gernerator if it has been removed of message-processing by stopGenerator() */
		/*! /brief     Stopping Generators + Worker-Thread for this device
			/param     const bool shutdown - False(Default): Stop Generators Color, Depth, Skeleton; True: Stop Generators + Worker Thread and closing handles
		*/
		void										stop( const bool msgThread = false );	/*!< Use this function to shutdown all generators / (shutdown = true )all generators + messaging thread */
		void										stopGenerator( uint32_t generators );	/*!< Use this to temporary disable message-processing for a certain generator */
		void										shutdown();	/*!< Shuts down system, release all handles and frees resources of kinect-sensor */

		// Member
		//image measurements - copy from m_configuration
		const uint16_t								&m_WidthImageColor, &m_WidthImageDepthAndUser;
		const uint16_t								&m_HeightImageColor, &m_HeightImageDepthAndUser;


	private:
		static DWORD WINAPI							threadEventsFetcher( LPVOID pParam );	/*!< Message Loop function running in a separate thread fetching messages about kinect sensor data */
		void										initColorCoords( uint32_t totalPixels );	/*!< init collor coordinate array for depth to colorpixel mapping, should be called in init depthstream/inituserdepthstream */
		void										initColorStream();	/*!< initialize message handles and configures image buffer to a certain resolution */
		void										initDepthStream();	/*!< initialize message handles for depth only and configures image buffer to a certain resolution */
		void										initUserDepthStream();	/*!< initialize message handles for user and depth and configures image buffer to a certain resolution */
		inline uint32_t								mirrorIndex( const uint32_t index, const uint32_t imageWidth ); 	/*!< Mirroring a index to a given index and image width */
		void										processColorImageEvent();	/*!< callback function called by threadEventFetcher function, will copy data to imagebuffer + mirror + colorToDeph handling */
		void										processDepthImageEvent();	/*!< callback function called by threadEventFetcher function, will copy data to imagebuffer + mirror + color coords processing */
		void										processSkeletonEvent();	/*!< Creates Users and their Skeletons/Bones which can later be fetched through getUsers() */
		_2RealTrackedJoint_sptr						createJointFromNUI( _2RealJointType type, _NUI_SKELETON_POSITION_INDEX nuiType, const NUI_SKELETON_DATA& nuiData, const NUI_SKELETON_BONE_ORIENTATION* nuiOrientation ); 	/*!< Creates a joint from the given nui objects */
		inline void									setMappedColorCoords( const uint32_t index, uint16_t* depthSource, const NUI_IMAGE_VIEW_AREA* vArea ); /*! function to fill mapped color coordinates, to be called from processDepthImageEvent()! */

		// Member
		INuiSensor*									m_NuiSensor;	/*!< holding the instance of the nui-sensor, coomunication with kinect through this */
		std::string									m_Name;
		int											m_DeviceID;
		WSDKDeviceConfiguration						m_Configuration; 	/*!< configuration set up with configureDevice will be saved here */

		//events
		HANDLE										m_EventColorImage;
		HANDLE										m_EventDepthImage;
		HANDLE										m_EventSkeletonData;
		HANDLE										m_EventStopThread;
		//stream handles
		HANDLE										m_HandleColorStream;
		HANDLE										m_HandleDepthStream;
		HANDLE										m_HandleThread;

		bool										m_isDeviceStarted;
		bool										m_isDeviceShutDown;
		bool										m_IsDeletingDevice;
		uint32_t									m_DeviceFlags;

		/*!
		* used to handle kinect events in separate thread
		* use this enum to lookup m_WTEvents
		*/
		enum WorkerThreadEvents
		{	
			WT_STOP_THREAD = 0,
			WT_EVENT_COLOR,
			WT_EVENT_DEPTH,
			WT_EVENT_SKELETON
		};
		HANDLE										m_WTEvents[4];

		//image buffers
		boost::shared_array<uchar>					m_ImageColor_8bit;
		boost::shared_array<uchar>					m_ImageDepth_8bit;
		boost::shared_array<uchar>					m_ImageUser_8bit;
		boost::shared_array<uchar>					m_ImageColoredUser_8bit;
		boost::shared_array<uint16_t>				m_ImageDepth_16bit;
		boost::shared_array<LONG>					m_ColorCoords;
		uint32_t									m_ColorCoordsSize;

		//sync
		mutable boost::mutex						m_MutexImage;
		boost::mutex								m_MutexDepth;
		mutable boost::mutex						m_MutexUser;
		boost::mutex								m_MutexColoredUser;
		mutable boost::mutex						m_MutexFetchUser;
		boost::mutex								m_MutexFetchColorImage;
		boost::mutex								m_MutexFetchDepthImage, m_MutexFetchDepthImage2, m_MutexFetchDepthImage3;
		boost::condition							m_NotificationNewColorImageData;
		boost::condition							m_NotificationNewDepthImageData;
		mutable boost::condition					m_NotificationNewUserdata;

		_2RealTrackedUserVector						m_Users;
};

}
#endif
