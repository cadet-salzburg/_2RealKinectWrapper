/*
   CADET - Center for Advances in Digital Entertainment Technologies
   Copyright 2011 University of Applied Science Salzburg / MultiMediaTechnology
	   http://www.cadet.at

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
#ifndef TARGET_MSKINECTSDK
#include "_2RealConfig.h"

#include "OpenNISpecific/OpenNIDepthGenerator.h"
#include "_2RealImageSource.h"
#include "_2RealTrackedUser.h"


namespace _2Real
{

void XN_CALLBACK_TYPE depthErrorStateChanged(xn::MapGenerator& rMapGenerator, void *pCookie)
{
	XnStatus status;
	xn::DepthGenerator *depthGenerator = static_cast<xn::DepthGenerator*>(pCookie);
	
	status = depthGenerator->GetErrorStateCap().GetErrorState();
	if (status != XN_STATUS_OK)
	{
		_2REAL_LOG(info) << "depth generator is in error state; status: " << std::string(xnGetStatusString(status)) << std::endl;
	}
}

void XN_CALLBACK_TYPE depthOutputStateChanged(xn::MapGenerator& rMapGenerator, void *pCookie)
{
	_2REAL_LOG(info) <<"depth map resolution changed\n";
}

OpenNIDepthGenerator::OpenNIDepthGenerator()
{
}

OpenNIDepthGenerator::~OpenNIDepthGenerator()
{
}

XnStatus OpenNIDepthGenerator::startGenerating()
{
	m_DepthGenerator.LockedNodeStartChanges(m_DepthLock);
	XnStatus status = m_DepthGenerator.StartGenerating();
	m_DepthGenerator.LockedNodeEndChanges(m_DepthLock);
	return (getErrorState() | status);
}

XnStatus OpenNIDepthGenerator::getTimestamp( uint64_t& time ) const
{
	time = m_DepthGenerator.GetTimestamp();
	return getErrorState();
}

XnStatus OpenNIDepthGenerator::stopGenerating()
{
	m_DepthGenerator.LockedNodeStartChanges(m_DepthLock);
	XnStatus status = m_DepthGenerator.StopGenerating();
	m_DepthGenerator.LockedNodeEndChanges(m_DepthLock);
	return (getErrorState() | status);
}

XnStatus OpenNIDepthGenerator::lockGenerator()
{
	XnStatus status = m_DepthGenerator.LockForChanges(&m_DepthLock);

	return (getErrorState() | status);
}

XnStatus OpenNIDepthGenerator::unlockGenerator()
{
	m_DepthGenerator.UnlockForChanges(m_DepthLock);	
	return getErrorState();
}

XnStatus OpenNIDepthGenerator::getErrorState() const
{
	return m_DepthGenerator.GetErrorStateCap().GetErrorState();
}

bool OpenNIDepthGenerator::isGenerating() const
{
	if( m_DepthGenerator.IsGenerating() == TRUE )
		return true;
	return false;
}

bool OpenNIDepthGenerator::isMirrored() const
{
	if( m_DepthGenerator.GetMirrorCap().IsMirrored() == TRUE )
		return true;
	return false;
}

XnStatus OpenNIDepthGenerator::setOutputMode(const XnMapOutputMode outputMode)
{
	m_DepthGenerator.LockedNodeStartChanges(m_DepthLock);
	XnStatus status = m_DepthGenerator.SetMapOutputMode(outputMode);
	m_DepthGenerator.LockedNodeEndChanges(m_DepthLock);
	return (getErrorState() | status);
}


XnStatus OpenNIDepthGenerator::registerCallbacks()
{
	XnCallbackHandle errorStateHandle;
	XnStatus status = m_DepthGenerator.RegisterToNewDataAvailable((xn::StateChangedHandler)depthErrorStateChanged, (xn::DepthGenerator *)&m_DepthGenerator, errorStateHandle);

	XnCallbackHandle outputStateHandle;
	status = m_DepthGenerator.RegisterToMapOutputModeChange((xn::StateChangedHandler)depthOutputStateChanged, (xn::DepthGenerator *)&m_DepthGenerator, outputStateHandle);
	
	return (getErrorState() | status);
}

XnStatus OpenNIDepthGenerator::getData( _2RealImageSource<uint16_t>& data ) const
{
	xn::DepthMetaData metadata;
	m_DepthGenerator.GetMetaData(metadata);

	data.setData(m_DepthGenerator.GetDepthMap());
	data.setFullResolution(metadata.FullXRes(), metadata.FullYRes());
	data.setCroppedResolution(metadata.XRes(), metadata.YRes());
	data.setCroppingOffest(metadata.XOffset(), metadata.YOffset());
	data.setTimestamp(metadata.Timestamp());
	data.setFrameID(metadata.FrameID());
	data.setBytesPerPixel(metadata.BytesPerPixel());

	if (metadata.XOffset() != 0)
	{
		data.setCropping(true);
	}
	else
	{
		data.setCropping(false);
	}

	if (m_DepthGenerator.GetMirrorCap().IsMirrored())
	{
		data.setMirroring(true);
	}
	else
	{
		data.setMirroring(false);
	}

	return getErrorState();
}

XnStatus OpenNIDepthGenerator::updateJoint( const XnSkeletonJoint type, _2RealTrackedUser& user ) const
{
	XnPoint3D worldPos;
	XnPoint3D screenPos;
	_2RealTrackedJoint* joint = const_cast<_2RealTrackedJoint*>( user.getJoint( _2RealJointType(type-1) ) );
	if( joint == NULL )
		return 0;
	_2RealVector3f wPos = joint->getWorldPosition();
	worldPos.X = wPos.x;
	worldPos.Y = wPos.y;
	worldPos.Z = wPos.z;
	XnStatus status = m_DepthGenerator.ConvertRealWorldToProjective(1, &worldPos, &screenPos);
	joint->setScreenPosition( _2RealVector2f( screenPos.X, screenPos.Y ) );

	return (getErrorState() | status);
}

XnStatus OpenNIDepthGenerator::getUserScreenPosition( _2RealTrackedUser& user ) const
{
	updateJoint(XN_SKEL_HEAD, user);
	updateJoint(XN_SKEL_NECK, user);
	updateJoint(XN_SKEL_TORSO, user);
	updateJoint(XN_SKEL_WAIST, user);
	updateJoint(XN_SKEL_LEFT_HIP, user);
	updateJoint(XN_SKEL_LEFT_KNEE, user);
	updateJoint(XN_SKEL_LEFT_ANKLE, user);
	updateJoint(XN_SKEL_LEFT_FOOT, user);
	updateJoint(XN_SKEL_RIGHT_HIP, user);
	updateJoint(XN_SKEL_RIGHT_KNEE, user);
	updateJoint(XN_SKEL_RIGHT_ANKLE, user);
	updateJoint(XN_SKEL_RIGHT_FOOT, user);
	updateJoint(XN_SKEL_LEFT_COLLAR, user);
	updateJoint(XN_SKEL_LEFT_SHOULDER, user);
	updateJoint(XN_SKEL_LEFT_ELBOW, user);
	updateJoint(XN_SKEL_LEFT_WRIST, user);
	updateJoint(XN_SKEL_LEFT_HAND, user);
	updateJoint(XN_SKEL_LEFT_FINGERTIP, user);
	updateJoint(XN_SKEL_RIGHT_COLLAR, user);
	updateJoint(XN_SKEL_RIGHT_SHOULDER, user);
	updateJoint(XN_SKEL_RIGHT_ELBOW, user);
	updateJoint(XN_SKEL_RIGHT_WRIST, user);
	updateJoint(XN_SKEL_RIGHT_HAND, user);
	updateJoint(XN_SKEL_RIGHT_FINGERTIP, user);

	return getErrorState();
}

XnStatus OpenNIDepthGenerator::getUserScreenPositions( _2RealTrackedUserVector& users ) const
{
	int size = users.size();
	for ( int i=0; i<size; ++i )
	{
		getUserScreenPosition(users[i]);
	}

	return getErrorState();
}

XnStatus OpenNIDepthGenerator::getUserScreenPositions( _2RealTrackedUser** users, const int size ) const
{
	for ( int i=0; i<size; ++i )
	{
		if( users[i] )
			getUserScreenPosition( *users[i] );
	}

	return getErrorState();
}

XnStatus OpenNIDepthGenerator::setMirroring(const bool mirror)
{
	m_DepthGenerator.LockedNodeStartChanges(m_DepthLock);
	XnStatus status = m_DepthGenerator.GetMirrorCap().SetMirror(mirror);
	m_DepthGenerator.LockedNodeEndChanges(m_DepthLock);
	return status;
}

XnStatus OpenNIDepthGenerator::getFramesPerSecond( int& fps ) const
{
	XnMapOutputMode outputMode;
	m_DepthGenerator.GetMapOutputMode(outputMode);
	fps = outputMode.nFPS;
	return getErrorState();
}

XnStatus OpenNIDepthGenerator::getMapResolution( uint32_t& x, uint32_t& y ) const
{
	XnMapOutputMode outputMode;
	m_DepthGenerator.GetMapOutputMode(outputMode);
	x = outputMode.nXRes;
	y = outputMode.nYRes;
	return getErrorState();
}

}
#endif