#include "_2RealTrackedUser.h"
#include "_2RealConfig.h"
#include "_2RealUtility.h"

namespace _2Real
{

_2RealTrackedUser::_2RealTrackedUser( const _2RealTrackedUser& o )
	: m_ID( o.m_ID )
{
	m_Joints.resize( _2REAL_NUMBER_OF_JOINTS );
	int size = o.m_Joints.size();
	for( int i = 0; i < size; ++i )
	{
		m_Joints[i] = o.m_Joints[i];
	}
}

_2RealTrackedUser::_2RealTrackedUser( uint32_t id )
	: m_ID( id )
{
	m_Joints.resize( _2REAL_NUMBER_OF_JOINTS );
	for( int i = 0; i < _2REAL_NUMBER_OF_JOINTS; ++i )
	{
		m_Joints[i] = _2RealTrackedJoint();
	}
}

_2RealTrackedUser::_2RealTrackedUser()
{
	m_Joints.resize( _2REAL_NUMBER_OF_JOINTS );
	for( int i = 0; i < _2REAL_NUMBER_OF_JOINTS; ++i )
	{
		m_Joints[i] = _2RealTrackedJoint();
	}
}

_2RealTrackedUser& _2RealTrackedUser::operator=( const _2RealTrackedUser& o )
{
	if( this == &o ) //self assignment?!
		return *this;

	//copy all joints
	int size = o.m_Joints.size();
	for( int i=0; i < size; ++i )
	{
		m_Joints[i] = o.m_Joints[i];
	}
	m_ID = o.m_ID;
	return *this;
}

_2RealTrackedUser::~_2RealTrackedUser()
{
	m_Joints.clear();
}

_2RealPositionVector3f& _2RealTrackedUser::getJointWorldPositions( const float confidence/*=0.0f */ )
{
	//m_JointWorldPositions.clear(); //empty world position vector
	int size = (int)m_Joints.size();
	_2RealVector3f pos;

	for ( int i=0; i<size; ++i )
	{		
		if ( m_Joints[i].getPositionConfidence() >= confidence )
		{
			m_JointWorldPositions[i] = m_Joints[i].getWorldPosition();
		}
		else //filling with zero value
			m_JointWorldPositions.push_back( _2RealVector3f() );
	}
	return m_JointWorldPositions;
}

_2RealPositionVector2f& _2RealTrackedUser::getJointScreenPositions( const float confidence/*=0.0f */ )
{
	m_JointScreenPositions.clear(); //empty world position vector
	int size = (int)m_Joints.size();
	_2RealVector3f pos;

	for ( int i=0; i<size; ++i )
	{		
		if ( m_Joints[i].getPositionConfidence() >= confidence ) 
		{
			m_JointScreenPositions.push_back( m_Joints[i].getScreenPosition() );
		}
		else //filling with zero value
			m_JointScreenPositions.push_back( _2RealVector2f() );
	}
	return m_JointScreenPositions;
}

const _2RealTrackedJoint* _2RealTrackedUser::getJoint( const _2RealJointType jointType ) const
{
	//checking if joint type is in between 0 and max of bones-1 (24-1)
	if( jointType < _2REAL_NUMBER_OF_JOINTS && jointType >= 0 )
		return &m_Joints[jointType];
	return NULL;
}

const _2RealVector3f _2RealTrackedUser::getJointWorldPosition( _2RealJointType type, const float confidence ) const
{
	//checking if joint type is in between 1 and max of bones (24)
	if( type < _2REAL_NUMBER_OF_JOINTS && type >= 0 )
		return m_Joints[type].getWorldPosition();
	return _2RealVector3f();
}

const _2RealVector2f _2RealTrackedUser::getJointScreenPosition( _2RealJointType type, const float confidence ) const
{
	//checking if joint type is in between 1 and max of bones (24)
	if( type < _2REAL_NUMBER_OF_JOINTS && type >= 0 )
		return m_Joints[type].getScreenPosition();
	return _2RealVector2f();
}

uint32_t _2RealTrackedUser::getUserID() const
{
	return m_ID;
}

void _2RealTrackedUser::setJoint( const _2RealJointType jointType, const _2RealTrackedJoint& joint )
{
	if( jointType < 0 || jointType >= (int)m_Joints.size() )
		throwError( "_2Real: OpenNIImpl::_2RealTrackedUser::setJoint() jointType out of bounds!" );
	m_Joints[jointType] = joint;
}

const uint32_t _2RealTrackedUser::getMaxNumberOfJoints() const
{
	return _2REAL_NUMBER_OF_JOINTS;
}

}