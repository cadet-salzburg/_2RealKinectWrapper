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
#include "OpenNISpecific/OpenNIColorGenerator.h"
#include "OpenNISpecific/OpenNIDepthGenerator.h"
#include "OpenNISpecific/OpenNIInfraredGenerator.h"
#include "OpenNISpecific/OpenNIUserGenerator.h"
#include "_2RealUtility.h"
#include "_2RealTrackedUser.h"
#include "XnOpenNI.h"
#include "boost/thread/mutex.hpp"
#include <boost/thread/thread.hpp>
#include <vector>



namespace _2Real
{
	typedef std::vector<_2RealTrackedUser>	_2RealTrackedUserVector;

//container for the generators of a kinect device
class OpenNIDevice
{
	public:
		OpenNIDevice( const int id, const std::string& name, xn::Context& context, xn::NodeInfo& deviceInfo );
		~OpenNIDevice( void );

		xn::DepthGenerator&			GetOpenNIDepthGenerator();
		xn::UserGenerator&			GetOpenNIUserGenerator();
		xn::ImageGenerator&			GetOpenNIImageGenerator();
		xn::IRGenerator&			GetOpenNIInfraredGenertor();

		bool						startGenerators( const uint32_t startGenerators );
		void						startupProcessingColorGenerator( xn::NodeInfo& node, const uint32_t configureImages );
		void						startupProcessingDepthGenerator( xn::NodeInfo& node, const uint32_t configureImages );
		void						startupProcessingInfraredGenerator( xn::NodeInfo& node, const uint32_t configureImages );
		void						startupProcessingUserGenerator( xn::NodeInfo& node, const uint32_t configureImages );

		bool						shutdown();

		_2RealTrackedUserVector		getUsers();

		const XnMapOutputMode&		getOutputmodeColor() const;
		const XnMapOutputMode&		getOutputmodeDepth() const;
		const XnMapOutputMode&		getOutputmodeInfrared() const;
		const XnMapOutputMode&		getOutputmodeUser() const;

		uint16_t*					getDepthBuffer_16bit();
		uint8_t*					getImageBuffer();
		uint8_t*					getDepthBuffer();
		uint8_t*					getInfraredBuffer();
		uint8_t*					getUserImageBuffer();
		uint8_t*					getUserColorImageBuffer();

	private:
		static void					update( void* instance );
		static void					checkError( XnStatus status, std::string error );
		static void					convertImage_16_to_8( const uint16_t* source, unsigned char* destination, uint32_t size, const int normalizing );

	public:
		// the available generators - depth map, user data, color image, infrared image
		OpenNIDepthGenerator					m_DepthGenerator;
		OpenNIColorGenerator					m_ColorGenerator;
		OpenNIUserGenerator						m_UserGenerator;
		OpenNIInfraredGenerator					m_InfraredGenerator;

	private:
		//image buffer obtained from kinect
		_2RealImageSource<uint8_t>				m_ColorImage;
		_2RealImageSource<uint16_t>				m_DepthImage, m_InfraredImage, m_UserImage;

		//converted image buffers
		unsigned char**							m_DepthImage_8bit;
		unsigned char**							m_InfraredImage_8bit;
		unsigned char**							m_UserImage_8bit;
		unsigned char**							m_UserImageColor_8bit;

		//misc
		const uint32_t							m_ID;
		std::string								m_InstanceName;
		_2RealTrackedUser**						m_TrackedUsersArray;
		const int								m_TrackedUserArraySize;
		XnMapOutputMode							m_OutputModeColor, m_OutputModeDepth, m_OutputModeInfrared, m_OutputModeUser;
		xn::Context&							m_Context;
		xn::NodeInfo&							m_DeviceInfo;
		boost::thread							m_ProcessingThread;
		boost::mutex							m_MutexFetchUsers;
		uint8_t									m_CurrentBuffer;
		bool									m_IsInitialized;
		bool									m_IsProcessingThread;
		bool									m_IsFetchingData;
};

}

