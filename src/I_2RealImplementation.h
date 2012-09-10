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

#include "_2RealTrackedUser.h"
#include <stdint.h>
#include <vector>



namespace _2RealKinectWrapper
{
	// Fwds

	struct _2RealVector2f;
	struct _2RealVector3f;

// 2Real-Interface for OpenNI/WSDK Implementations
class I_2RealImplementation
{
	public:

		virtual void								addGenerator( const uint32_t deviceID, uint32_t configureGenerators, uint32_t configureImages ) = 0;
		virtual void								alignDepthToColor( const uint32_t deviceID, bool flag ) = 0;
		virtual bool								depthIsAlignedToColor( const uint32_t deviceID ) = 0;
		virtual bool								configureDevice( const uint32_t deviceID,  uint32_t startGenerators, uint32_t configureImages ) = 0;
		virtual void								convertProjectiveToWorld( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inProjective, _2RealVector3f* outWorld ) = 0;
		virtual void								convertWorldToProjective( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inWorld, _2RealVector3f* outProjective ) = 0;
		virtual bool								generatorIsActive( const uint32_t deviceID, _2RealGenerator type ) = 0;

		virtual uint32_t							getBytesPerPixel( _2RealGenerator type) const = 0;
		virtual boost::shared_array<unsigned char>	getImageData( const uint32_t deviceID, _2RealGenerator type, bool waitAndBlock=false, const uint8_t userId=0 ) = 0;
		virtual boost::shared_array<uint16_t>		getImageDataDepth16Bit( const uint32_t deviceID, bool waitAndBlock=false) = 0;
		virtual uint32_t							getImageHeight( const uint32_t deviceID, _2RealGenerator type) = 0;
		virtual uint32_t							getImageWidth( const uint32_t deviceID, _2RealGenerator type) = 0;
		virtual const _2RealVector3f				getJointScreenPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type ) = 0;
		virtual const _2RealMatrix3x3				getJointWorldOrientation( const uint32_t deviceID, const uint8_t userID, _2RealJointType type ) = 0;
		virtual const _2RealVector3f				getJointWorldPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type ) = 0;
		virtual int									getMotorAngle(int deviceID) = 0;
		virtual uint32_t							getNumberOfDevices() const = 0;
		virtual _2RealFov							getFieldOfView( const uint32_t deviceID ) = 0;
		virtual void								setResolution( const uint32_t deviceID, _2RealGenerator type, unsigned int hRes, unsigned int vRes ) = 0;
		virtual const _2RealJointConfidence			getSkeletonJointConfidence(const uint32_t deviceID, const uint8_t userID, _2RealJointType type) = 0;
		virtual const _2RealJointConfidences		getSkeletonJointConfidences(const uint32_t deviceID, const uint8_t userID) = 0;
		virtual const _2RealPositionsVector3f		getSkeletonScreenPositions( const uint32_t deviceID, const uint8_t userID ) = 0;
		virtual const _2RealOrientationsMatrix3x3	getSkeletonWorldOrientations( const uint32_t deviceID, const uint8_t userID ) = 0;
		virtual const _2RealPositionsVector3f		getSkeletonWorldPositions( const uint32_t deviceID, const uint8_t userID ) = 0;

		virtual const uint32_t						getNumberOfUsers( const uint32_t deviceID ) const = 0;
		virtual const uint32_t						getNumberOfSkeletons( const uint32_t deviceID ) const = 0;
		virtual const _2RealVector3f				getUsersWorldCenterOfMass(const uint32_t deviceID, const uint8_t userID) = 0;
		virtual const _2RealVector3f				getUsersScreenCenterOfMass(const uint32_t deviceID, const uint8_t userID) = 0;

		virtual bool								hasFeatureJointOrientation() const = 0;
		virtual bool								isJointAvailable( _2RealJointType type ) const = 0;
		virtual bool								isMirrored( const uint32_t deviceID, _2RealGenerator type ) const = 0;
		virtual const bool							isNewData(const uint32_t deviceID, _2RealGenerator type) const = 0;

		virtual void								removeGenerator( const uint32_t deviceID, uint32_t configureGenerators ) = 0;
		virtual void								resetAllSkeletons() = 0;
		virtual void								resetSkeleton( const uint32_t deviceID, const uint32_t id ) = 0;
		virtual bool								restart() = 0;

		virtual void								setLogLevel(_2RealLogLevel iLevel) = 0;
		virtual void								setLogOutputStream(std::ostream* outStream) = 0;
		virtual void								setMirrored( const uint32_t deviceID, _2RealGenerator type, bool flag ) = 0;
		virtual bool								setMotorAngle(int deviceID, int& angle) = 0;

		virtual bool								shutdown() = 0;
		virtual void								startGenerator( const uint32_t deviceID, uint32_t configureGenerators ) = 0;
		virtual void								stopGenerator( const uint32_t deviceID, uint32_t configureGenerators ) = 0;
		virtual void								update() = 0;
};

}
