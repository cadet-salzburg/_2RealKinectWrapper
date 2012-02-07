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
#include "_2RealTypes.h"
#include "_2RealUtility.h"
#include <Windows.h>
#include "NuiApi.h"
#include <iostream>
#include <stdint.h>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/condition.hpp>


namespace _2Real
{
	typedef unsigned char uchar;
	class _2RealTrackedUser;
	class _2RealTrackedJoint;
	typedef std::vector<_2RealTrackedUser>	_2RealTrackedUserVector;

class WSDKDevice
{
	public:
		WSDKDevice( INuiSensor* devicePtr, const uint32_t configSensor, const uint32_t configImage, const std::string& name );
		virtual ~WSDKDevice(void);

		struct BGRX
		{
			uchar		b, g, r, x;
		};

		struct RGB
		{
			uchar		r, g, b;
		};

		void						SetMirroringColor( const bool flag );
		void						SetMirroringDetph( const bool flag );
		void						SetMirroringUser( const bool flag );
		bool						IsMirroingColor() const;
		bool						IsMirroingDepth() const;
		bool						IsMirroingUser() const;
		_2RealTrackedUserVector&	GetUsers( bool waitForNewData );
		uchar*						GetColorImageBuffer( bool waitForNewData );
		uchar*						GetDepthImageBuffer( bool waitForNewData );
		uchar*						GetUserImageBuffer( bool waitForNewData );
		uchar*						GetColoredUserImageBuffer( bool waitForNewData );
		uint16_t*					GetDepthImageBuffer16Bit( bool waitForNewData );
		bool						SetMotorAngle(int angle);
		int							GetMotorAngle();

		//image measure
		const uint32_t				m_WidthImageColor, m_WidthImageDepthAndUser;
		const uint32_t				m_HeightImageColor, m_HeightImageDepthAndUser;

	private:

		static DWORD WINAPI			ThreadEventsFetcher( LPVOID pParam );
		void						ProcessColorImageEvent();
		void						ProcessDepthImageEvent();
		void						ProcessSkeletonEvent();
		_2RealTrackedJoint			GetJoint( _2RealJointType type, _NUI_SKELETON_POSITION_INDEX nuiType, const NUI_SKELETON_DATA& data );
								

		INuiSensor*					m_pNuiSensor;
		std::string					m_name;
		//events
		HANDLE						m_EventColorImage;
		HANDLE						m_EventDepthImage;
		HANDLE						m_EventSkeletonData;
		HANDLE						m_EventStopThread;
		//stream handles
		HANDLE						m_HandleColorStream;
		HANDLE						m_HandleDepthStream;
		HANDLE						m_HandleThread;

		bool						m_IsDepthOnly;
		bool						m_IsMirroringColor;
		bool						m_IsMirroringDepth;
		bool						m_IsMirroringUser;
		bool						m_IsDeletingDevice;

		//image buffers
		uchar*						m_ImageColor_8bit;
		uchar*						m_ImageDepth_8bit;
		uchar*						m_ImageUser_8bit;
		uchar*						m_ImageColoredUser_8bit;
		uint16_t*					m_ImageDepth_16bit;
		
		//users
		_2RealTrackedUserVector		m_Users;
		_2RealTrackedUserVector		m_UsersShared;

		//sync
		boost::mutex				m_MutexImage;
		boost::mutex				m_MutexDepth;
		boost::mutex				m_MutexUser;
		boost::mutex				m_MutexColoredUser;
		boost::mutex				m_MutexFetchUser;
		boost::mutex				m_MutexFetchColorImage;
		boost::mutex				m_MutexFetchDepthImage, m_MutexFetchDepthImage2, m_MutexFetchDepthImage3;
		boost::condition			m_NotificationNewColorImageData;
		boost::condition			m_NotificationNewDepthImageData;
		boost::condition			m_NotificationNewUserdata;
};

}

