#include "_2RealKinect.h"
#include "_2RealVersion.h"
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <stdint.h>


#ifndef TARGET_MSKINECTSDK
	#include "OpenNISpecific/_2RealImplementationOpenNI.hpp"
#else
	#include "WinSDKSpecific/_2RealImplementationWinSDK.hpp"
#endif

namespace _2RealKinectWrapper
{

boost::mutex _global_2RealMutex;
_2RealKinectWrapper::_2RealKinect* _2RealKinect::m_Instance = NULL;

_2RealKinect* _2RealKinect::getInstance()
{
	boost::interprocess::scoped_lock<boost::mutex> lock( _global_2RealMutex );
	if( !m_Instance )
	{
		m_Instance = new _2RealKinect();
	}
	return m_Instance;
}

std::string _2RealKinect::getVersion()
{
	return _2RealVersion::getVersion();
}

bool _2RealKinect::isAtLeastVersion(int major, int minor, int patch)
{
	return _2RealVersion::isAtLeast(major, minor, patch);
}

_2RealKinect::_2RealKinect() : 
#ifndef TARGET_MSKINECTSDK 
	m_Implementation( new _2RealImplementationOpenNI() )
#else
	m_Implementation( new _2RealImplementationWinSDK() )
#endif
{

}

_2RealKinect::~_2RealKinect()
{
	//shutdown && freeing memory
	shutdown();
	delete m_Instance;
}
void _2RealKinect::update()
{
	m_Implementation->update();
}
bool _2RealKinect::configure( const uint32_t deviceID, uint32_t startGenerators, uint32_t configureImages )
{
	return m_Implementation->configureDevice( deviceID, startGenerators, configureImages );
}
void _2RealKinect::startGenerator( const uint32_t deviceID, uint32_t configureGenerators )
{
	m_Implementation->startGenerator( deviceID, configureGenerators );
}

bool _2RealKinect::shutdown()
{
	return m_Implementation->shutdown();
}

const bool _2RealKinect::isNewData(const uint32_t deviceID, _2RealGenerator type) const
{
	return m_Implementation->isNewData(deviceID, type);
}

uint32_t _2RealKinect::getBytesPerPixel( _2RealGenerator type ) const
{
	return m_Implementation->getBytesPerPixel( type );
}

boost::shared_array<unsigned char> _2RealKinect::getImageData( const uint32_t deviceID, _2RealGenerator type, bool waitAndBlock, const uint8_t userId)
{
	return m_Implementation->getImageData( deviceID, type, waitAndBlock, userId );
}

boost::shared_array<uint16_t> _2RealKinect::getImageDataDepth16Bit( const uint32_t deviceID, bool waitAndBlock/*=false*/ )
{
	return m_Implementation->getImageDataDepth16Bit( deviceID, waitAndBlock );
}

bool _2RealKinect::isMirrored( const uint32_t deviceID, _2RealGenerator type ) const
{
	return m_Implementation->isMirrored( deviceID, type );
}

void _2RealKinect::setMirrored( const uint32_t deviceID, _2RealGenerator type, bool flag )
{
	m_Implementation->setMirrored( deviceID, type, flag );
}

uint32_t _2RealKinect::getImageWidth( const uint32_t deviceID, _2RealGenerator type )
{
	return m_Implementation->getImageWidth( deviceID, type );
}

uint32_t _2RealKinect::getImageHeight( const uint32_t deviceID, _2RealGenerator type )
{
	return m_Implementation->getImageHeight( deviceID, type );
}

void _2RealKinect::resetAllSkeletons()
{
	m_Implementation->resetAllSkeletons();
}

void _2RealKinect::resetSkeleton( const uint32_t deviceID, const uint32_t id )
{
	m_Implementation->resetSkeleton( deviceID, id );
}

uint32_t _2RealKinect::getNumberOfDevices() const
{
	return m_Implementation->getNumberOfDevices();
}

void _2RealKinect::setAlignColorDepthImage( const uint32_t deviceID, bool flag )
{
	m_Implementation->setAlignColorDepthImage( deviceID, flag );
}

bool _2RealKinect::restart()
{
	return m_Implementation->restart();
}

void _2RealKinect::convertProjectiveToWorld( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inProjective, _2RealVector3f* outWorld )
{
	m_Implementation->convertProjectiveToWorld( deviceID, coordinateCount, inProjective, outWorld );
}

void _2RealKinect::convertWorldToProjective( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inWorld, _2RealVector3f* outProjective )
{
	m_Implementation->convertWorldToProjective( deviceID, coordinateCount, inWorld, outProjective );
}

const _2RealVector3f _2RealKinect::getSkeletonJointWorldPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
{
	return m_Implementation->getJointWorldPosition( deviceID, userID, type );
}

const _2RealPositionsVector3f _2RealKinect::getSkeletonWorldPositions( const uint32_t deviceID, const uint8_t userID )
{
	return m_Implementation->getSkeletonWorldPositions( deviceID, userID );
}

const _2RealVector3f _2RealKinect::getSkeletonJointScreenPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
{
	return m_Implementation->getJointScreenPosition( deviceID, userID, type );
}

const _2RealPositionsVector3f _2RealKinect::getSkeletonScreenPositions( const uint32_t deviceID, const uint8_t userID )
{
	return m_Implementation->getSkeletonScreenPositions( deviceID, userID );
}

const _2RealMatrix3x3 _2RealKinect::getSkeletonJointWorldOrientation( const uint32_t deviceID, const uint8_t userID, _2RealJointType type )
{
	return m_Implementation->getJointWorldOrientation( deviceID, userID, type );
}

const _2RealOrientationsMatrix3x3 _2RealKinect::getSkeletonWorldOrientations( const uint32_t deviceID, const uint8_t userID )
{
	return m_Implementation->getSkeletonWorldOrientations( deviceID, userID );
}

const _2RealJointConfidence _2RealKinect::getSkeletonJointConfidence( const uint32_t deviceID, const uint8_t userID, _2RealJointType type  )
{
	return m_Implementation->getSkeletonJointConfidence( deviceID, userID, type );
}

const _2RealJointConfidences _2RealKinect::getSkeletonJointConfidences(const uint32_t deviceID, const uint8_t userID)
{
	return m_Implementation->getSkeletonJointConfidences( deviceID, userID);
}

const uint32_t _2RealKinect::getNumberOfSkeletons( const uint32_t deviceID ) const
{
	return m_Implementation->getNumberOfSkeletons( deviceID );
}

const uint32_t _2RealKinect::getNumberOfUsers( const uint32_t deviceID ) const
{
	return m_Implementation->getNumberOfUsers( deviceID );
}

const _2RealVector3f _2RealKinect::getUsersWorldCenterOfMass(const uint32_t deviceID, const uint8_t userID) 
{
	return m_Implementation->getUsersWorldCenterOfMass(deviceID, userID);
}

const _2RealVector3f _2RealKinect::getUsersScreenCenterOfMass(const uint32_t deviceID, const uint8_t userID)
{
	return m_Implementation->getUsersScreenCenterOfMass(deviceID, userID);
}

bool _2RealKinect::isJointAvailable( _2RealJointType type ) const
{
	return m_Implementation->isJointAvailable( type );
}

bool _2RealKinect::hasFeatureJointOrientation() const
{
	return m_Implementation->hasFeatureJointOrientation();
}

bool _2RealKinect::setMotorAngle(int deviceID, int& angle)
{
	return m_Implementation->setMotorAngle(deviceID, angle);
}

int _2RealKinect::getMotorAngle(int deviceID)
{
	return m_Implementation->getMotorAngle(deviceID);
}

void _2RealKinect::setLogLevel(_2RealLogLevel iLevel) 
{ 
	m_Implementation->setLogLevel(iLevel); 
};

void _2RealKinect::setLogOutputStream(std::ostream* outStream) 
{  
	m_Implementation->setLogOutputStream(outStream); 
};

}