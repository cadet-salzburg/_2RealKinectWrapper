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
*/


#pragma once
#include <vector>
#include "_2RealTypes.h"

//enum XnSkeletonJoint;
namespace _2Real
{
	class _2RealTrackedJoint;
	typedef std::vector<_2RealTrackedJoint>	_2RealTrackedJointVector;

class _2RealTrackedJoint
{
	public:
		_2RealTrackedJoint( void );
		_2RealTrackedJoint( const _2RealJointType jointType, const _2RealVector2f& screenPosition, const _2RealVector3f& worldPosition,
							const _2RealMatrix3x3& worldOrientation, const _2RealConfidence& confidence );
		virtual ~_2RealTrackedJoint(void);
		_2RealTrackedJoint( const _2RealTrackedJoint& o );
		_2RealTrackedJoint& operator=( const _2RealTrackedJoint& joint );

		/*! /brief returns the joints' position in world coordinates
		*/
		_2RealVector3f					getWorldPosition() const;

		/*! /brief returns the joints' x & y screen coordinates
		*/
		_2RealVector2f					getScreenPosition() const;

		/*! /brief returns the joints' orientation in the form of 3x3-Matrix representing the x, y & z axis
			/return position a reference to a 3x3 matrix where the orientation will be stored
		*/
		_2RealMatrix3x3					getWorldOrientation() const;

		/*! /brief returns the joints' confidence for position and orientation 
		*/
		_2RealConfidence				getConfidence();

		/*! /brief     returns the type of the joint
			/return    _2Real::JointType
		!*/
		_2RealJointType					getJointType() const;

	private:


		// name of the joint
		_2RealJointType							m_JointType;
		// xy screen corrdinates, z = 0
		_2RealVector2f							m_ScreenPosition;
		// xyz world coordinates
		_2RealVector3f							m_WorldPosition;
		// orientation, represented by 3x3-Matrix
		_2RealMatrix3x3							m_WorldOrientation;
		// orientation, position confidence 
		_2RealConfidence						m_Confidence;

		friend class _2RealTrackedUser;
		friend class OpenNIDepthGenerator;

		void			setScreenPosition( const _2RealVector2f& position );
};

}
