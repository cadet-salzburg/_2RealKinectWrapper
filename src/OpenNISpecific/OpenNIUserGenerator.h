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
#include "OpenNISpecific/OpenNIGenerator.h"


namespace _2Real
{

	template <typename T> class _2RealImageSource;

class OpenNIUserGenerator : public OpenNIGenerator
{
	public:

		OpenNIUserGenerator();
		~OpenNIUserGenerator();

		XnStatus				startGenerating();
		XnStatus				stopGenerating();	
		bool					isGenerating() const;	
	
		// lock the generator so that no changes can be made to it's configuration without the lock handle
		XnStatus				lockGenerator();
		// unlock the generator so that changes can be made to it's configuration
		XnStatus				unlockGenerator();
		// chose the skeleton to be tracked
		XnStatus				setSkeletonProfile( const XnSkeletonProfile profile );
		// register user callbacks
		XnStatus				registerCallbacks();
		// return the generator's error state
		XnStatus				getErrorState() const;
		// return the timestamp of the currently available data
		XnStatus				getTimestamp( uint64_t& time ) const;	
		// return all users who are currently being tracked
		XnStatus				getTrackedUsers( _2RealTrackedUserVector& users );
		XnStatus				getTrackedUsers( _2RealTrackedUser** userArray, const int size );
		// get a particular joint from a particular user
		_2RealTrackedJoint		getUserJoint( const uint32_t userID, XnSkeletonJoint type ) const;
		// get a particular user
		XnStatus				getUserByID( uint32_t userID, _2RealTrackedUser& user );
		// enable or disable mirroring
		XnStatus				setMirroring(const bool mirror);
		// get current fps
		XnStatus				getFramesPerSecond(int &fps) const;
		// get the labelled user map
		XnStatus				getUserData(int iID, _2RealImageSource<uint16_t>& data) const;
		// get nr of users
		uint32_t				getNrOfUsers() const;
		//get mirror setting
		bool					isMirrored() const;
	
		void					forceResetUser( const uint32_t id );
		void					forceResetUsers();


		// the XnUserGenerator
		xn::UserGenerator	m_UserGenerator;	
		// lock handle
		XnLockHandle		m_UserLock;
};

}