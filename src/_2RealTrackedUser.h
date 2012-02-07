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
   Created: 08-31-2011
*/


#pragma once

#include "_2RealTrackedJoint.h"
#include "_2RealTypes.h"
#include <stdint.h>
#include <vector>


namespace _2Real
{

class _2RealTrackedUser
{
	public:

		_2RealTrackedUser();
		_2RealTrackedUser( uint32_t id );
		virtual ~_2RealTrackedUser();
		_2RealTrackedUser( const _2RealTrackedUser& o );
		_2RealTrackedUser& operator=( const _2RealTrackedUser& o );

		/*! /brief     returns the ID of the user
			/return    std::uint32_t - ID
		!*/
		uint32_t								getUserID() const;

		/*! /brief returns the object of a particular joint
			/param jointType the name of the joint
			/return const _2RealTrackedJoint* - reference to joint - return null if jointtype is not available
		*/
		const _2RealTrackedJoint*				getJoint( const _2RealJointType jointType ) const;

		/*! /brief returns a vector containing the users' joint positions in world space; if confidence expectations aren't met position for joint will be (0,0,0)
			Use _2RealJointType enum to return a specific joint through the operator[] ( getJointWorldPositions()[JOINT_HEAD] )
			/param jointVertices a vector of floats where the xyz coordinates of the users' joints will be stored
			*/
		_2RealPositionsVector3f&					getSkeletonWorldPositions( );

		/*! /brief		returns a vector containing the users' joint positions in screen space; if confidence expectations aren't met position for joint will be (0,0,0)
						Use _2RealJointType enum to return a specific joint through the operator[] ( getJointWorldPositions()[JOINT_HEAD] )
			/param		jointVertices a vector of floats where the xyz coordinates of the users' joints will be stored
		*/
		_2RealPositionsVector2f&					getSkeletonScreenPositions( );

		/*! /brief		returns a vector containing the users' joint orientations; if confidence expectations aren't met position for joint will be (0,0,0)
						Use _2RealJointType enum to return a specific joint through the operator[] ( getJointWorldPositions()[JOINT_HEAD] )
		*/
		_2RealOrientationsMatrix3x3& getSkeletonWorldOrientations( );

		/*! /brief     returns the world position of a specific user joint
			/return    _2Real::_2RealVector3f
		!*/
		const _2RealVector3f					getJointWorldPosition( _2RealJointType type ) const;

		/*! /brief     returns the screen position of a specific user joint
			/return    _2Real::_2RealVector3f
		!*/
		const _2RealVector2f					getJointScreenPosition( _2RealJointType type ) const;

		/*! /brief     returns the 3x3 orientation matrix of a specific user joint
			/param     _2RealJointType type - Type of the joint to be returned (hand, ...)
			/return    _2Real::_2RealVector3f
		!*/
		const _2RealMatrix3x3					getJointWorldOrientation( _2RealJointType type ) const;

		/*! /brief     returns the position and orientation condfidence of a joint
			/param     _2RealJointType type - Type of the joint to be returned (hand, ...)
			/return    _2RealConfidence
		!*/
	    _2RealConfidence					getJointConfidence( _2RealJointType type );

		/*! /brief     Return maximal number of joints available, Notice: There will be more entries in the joint vector created than used, for information
					   see _2RealStructuresTypes.h
			/return    const uint32_t
		!*/
		const uint32_t							getMaxNumberOfJoints() const;


	private:

		void									setJoint( const _2RealJointType jointType, const _2RealTrackedJoint& joint );

		//the joints stored in a vector
		_2RealTrackedJointVector					m_Joints;
		_2RealPositionsVector2f						m_JointScreenPositions;
		_2RealPositionsVector3f						m_JointWorldPositions;
		_2RealOrientationsMatrix3x3					m_JointWorldOrientations;
		uint32_t									m_ID;

		friend class OpenNIUserGenerator;
		friend class OpenNIDepthGenerator;
		friend class WSDKDevice;
};

}
