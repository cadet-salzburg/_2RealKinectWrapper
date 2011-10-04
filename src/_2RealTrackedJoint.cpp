#include "_2RealTrackedJoint.h"
#include "XnTypes.h"


namespace _2Real
{

_2RealTrackedJoint::_2RealTrackedJoint( const _2RealJointType jointType, const _2RealVector2f& screenPosition, const _2RealVector3f& worldPosition,
	const _2RealMatrix3x3& worldOrientation, const float positionConfidence, const float orientationConfidence )
	: 
#ifdef TARGET_MSKINECTSDK
	m_JointType( jointType ),							// MS Kinect
#else
	m_JointType( (_2RealJointType)(jointType - 1)),		// OpenNI joints starts at 1
#endif	
	m_ScreenPosition( screenPosition ),
	m_WorldPosition( worldPosition ),
	m_WorldOrientation( worldOrientation ),
	m_fPositionConfidence( positionConfidence ),
	m_fOrientationConfidence( orientationConfidence )
{


}

_2RealTrackedJoint::_2RealTrackedJoint(void)
{
}

_2RealTrackedJoint::_2RealTrackedJoint( const _2RealTrackedJoint& o )
	: m_JointType( o.m_JointType ),
	m_WorldPosition( o.m_WorldPosition ),
	m_ScreenPosition( o.m_ScreenPosition ),
	m_WorldOrientation( o.m_WorldOrientation ),
	m_fPositionConfidence( o.m_fPositionConfidence ),
	m_fOrientationConfidence( o.m_fOrientationConfidence )
{

}


_2RealTrackedJoint::~_2RealTrackedJoint(void)
{
}

_2RealTrackedJoint &_2RealTrackedJoint::operator=( const _2RealTrackedJoint &joint )
{
	if( this == &joint ) //self assignment?!
		return *this;

	this->m_JointType = joint.m_JointType;	
	this->m_WorldPosition = joint.m_WorldPosition;
	this->m_ScreenPosition = joint.m_ScreenPosition;
	this->m_WorldOrientation = joint.m_WorldOrientation;
	this->m_fPositionConfidence = joint.m_fPositionConfidence;
	this->m_fOrientationConfidence = joint.m_fOrientationConfidence;

	return *this;
}

////-------------------> Setter

void _2RealTrackedJoint::setWorldPosition(const _2RealVector3f& position)
{
	m_WorldPosition = position;
}


void _2RealTrackedJoint::setScreenPosition( const _2RealVector2f& position )
{
	m_ScreenPosition.x = position.x;
	m_ScreenPosition.y = position.y;
}

void _2RealTrackedJoint::setWorldOrientation(const _2RealMatrix3x3& orientation)
{
	m_WorldOrientation = orientation;
}

void _2RealTrackedJoint::setPositionConfidence(const float confidence)
{
	m_fPositionConfidence = confidence;
}

void _2RealTrackedJoint::setOrientationConfidence(const float confidence)
{
	m_fOrientationConfidence = confidence;
}

//<----------------------------------

//------------------> Getter
_2RealJointType _2RealTrackedJoint::getJointType() const
{
	return m_JointType;
}

_2RealVector3f _2RealTrackedJoint::getWorldPosition() const
{
	return m_WorldPosition;
}

_2RealVector2f _2RealTrackedJoint::getScreenPosition() const
{
	return m_ScreenPosition;
}

_2RealMatrix3x3 _2RealTrackedJoint::getWorldOrientation() const
{
	return m_WorldOrientation;
}

float _2RealTrackedJoint::getOrientationConfidence() const
{
	return m_fOrientationConfidence;
}

float _2RealTrackedJoint::getPositionConfidence() const
{
	return m_fPositionConfidence;
}

//<------------------------------

}