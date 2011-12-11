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

_2RealPositionsVector3f& _2RealTrackedUser::getSkeletonWorldPositions()
{
	m_JointWorldPositions.clear(); //empty world position vector
	int size = (int)m_Joints.size();

	for ( int i=0; i<size; ++i )
	{		
		m_JointWorldPositions.push_back( getJointWorldPosition(m_Joints[i].getJointType()) );
	}
	return m_JointWorldPositions;
}

_2RealPositionsVector2f& _2RealTrackedUser::getSkeletonScreenPositions()
{
	m_JointScreenPositions.clear(); //empty world position vector
	int size = (int)m_Joints.size();

	for ( int i=0; i<size; ++i )
	{		
		m_JointScreenPositions.push_back( getJointScreenPosition(m_Joints[i].getJointType()));
	}
	return m_JointScreenPositions;
}

_2RealOrientationsMatrix3x3& _2RealTrackedUser::getSkeletonWorldOrientations()
{
	m_JointWorldOrientations.clear(); //empty world position vector
	int size = (int)m_Joints.size();

	for ( int i=0; i<size; ++i )
	{		
		m_JointWorldOrientations.push_back( getJointWorldOrientation(m_Joints[i].getJointType()));
	}
	return m_JointWorldOrientations;
}

const _2RealTrackedJoint* _2RealTrackedUser::getJoint( const _2RealJointType jointType ) const
{
	//checking if joint type is in between 0 and max of bones-1 (24-1)
	if( jointType < _2REAL_NUMBER_OF_JOINTS && jointType >= 0 )
		return &m_Joints[jointType];
	return NULL;
}

const _2RealVector3f _2RealTrackedUser::getJointWorldPosition( _2RealJointType type ) const
{
	//checking if joint type is in between 1 and max of bones (24)
	if( type < _2REAL_NUMBER_OF_JOINTS && type >= 0  )
	{
		return m_Joints[type].getWorldPosition();
	}
	return _2RealVector3f();
}

const _2RealVector2f _2RealTrackedUser::getJointScreenPosition( _2RealJointType type ) const
{
	//checking if joint type is in between 1 and max of bones (24)
	if( type < _2REAL_NUMBER_OF_JOINTS && type >= 0 )
	{
		return m_Joints[type].getScreenPosition();
	}
	return _2RealVector2f();
}

const _2RealMatrix3x3 _2RealTrackedUser::getJointWorldOrientation( _2RealJointType type ) const
{
	if( type < _2REAL_NUMBER_OF_JOINTS && type >= 0 )
	{
		return m_Joints[type].getWorldOrientation();
	}
	return _2RealMatrix3x3();
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