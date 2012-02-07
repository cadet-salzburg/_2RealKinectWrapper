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
#include "OpenNISpecific/OpenNIUserGenerator.h"
#include "_2RealTrackedUser.h"
#include "_2RealImageSource.h"
#include "XnTypes.h"


namespace _2Real
{

void XN_CALLBACK_TYPE newUserCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	status = userGenerator->GetSkeletonCap().RequestCalibration(nID, TRUE);
	_2REAL_LOG(info) << "_2Real: New user " << nID << ", requested calibration... status: " << xnGetStatusString(status) << std::endl;
}

void XN_CALLBACK_TYPE lostUserCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	status = userGenerator->GetSkeletonCap().StopTracking(nID);
	status = userGenerator->GetSkeletonCap().Reset(nID);
	_2REAL_LOG(info) << "_2Real: Stopped tracking of user " << nID << "... status: " << xnGetStatusString(status) << std::endl;
}

void XN_CALLBACK_TYPE userExitCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	status = userGenerator->GetSkeletonCap().Reset(nID);
	_2REAL_LOG(info) << "_2Real: User " << nID << " exited - resetting skeleton data... status: " << xnGetStatusString(status) << std::endl;
}

void XN_CALLBACK_TYPE userReentryCallback(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	
	status = userGenerator->GetSkeletonCap().RequestCalibration(nID, TRUE);
	_2REAL_LOG(info) << "_2Real: User " << nID << " reentered, request calibration... status: " << xnGetStatusString(status) << std::endl;
}


void XN_CALLBACK_TYPE calibrationStartCallback(xn::SkeletonCapability& rCapability, XnUserID nID, void* pCookie)
{
	_2REAL_LOG(info) << "_2Real: Starting calibration of user " << nID << std::endl;
}

void XN_CALLBACK_TYPE calibrationCompleteCallback(xn::SkeletonCapability& rCapability, XnUserID nID, XnCalibrationStatus calibrationError, void* pCookie)
{
	XnStatus status;
	xn::UserGenerator* userGenerator = static_cast<xn::UserGenerator*>(pCookie);
	
	if (calibrationError == XN_STATUS_OK)
	{
		status = userGenerator->GetSkeletonCap().StartTracking(nID);
		_2REAL_LOG(info) << "_2Real: Calibration success, beginning to track user " << nID << " now... status: " << xnGetStatusString(status) << std::endl;
	}
	else
	{
		XnChar calibrationPose[20];
		status = userGenerator->GetSkeletonCap().GetCalibrationPose(calibrationPose);
		status = userGenerator->GetPoseDetectionCap().StartPoseDetection(calibrationPose, nID);
	}
}

XnStatus OpenNIUserGenerator::setSkeletonProfile( const XnSkeletonProfile profile )
{
	m_UserGenerator.LockedNodeStartChanges(m_UserLock);
	XnStatus status = m_UserGenerator.GetSkeletonCap().SetSkeletonProfile(profile);
	m_UserGenerator.LockedNodeEndChanges(m_UserLock);
	return (getErrorState() | status);
}

XnStatus OpenNIUserGenerator::startGenerating()
{
	m_UserGenerator.LockedNodeStartChanges(m_UserLock);
	XnStatus status = m_UserGenerator.StartGenerating();
	m_UserGenerator.LockedNodeEndChanges(m_UserLock);
	return (getErrorState() | status);
}

XnStatus OpenNIUserGenerator::getTimestamp( uint64_t& time ) const
{
	time = m_UserGenerator.GetTimestamp();
	return getErrorState();
}

XnStatus OpenNIUserGenerator::stopGenerating()
{
	m_UserGenerator.LockedNodeStartChanges(m_UserLock);
	XnStatus status = m_UserGenerator.StopGenerating();
	m_UserGenerator.LockedNodeEndChanges(m_UserLock);
	return (getErrorState() | status);
}

XnStatus OpenNIUserGenerator::lockGenerator()
{
	XnStatus status = m_UserGenerator.LockForChanges( &m_UserLock );

	return (getErrorState() | status);
}

XnStatus OpenNIUserGenerator::unlockGenerator()
{
	m_UserGenerator.UnlockForChanges(m_UserLock);	

	return getErrorState();
}

XnStatus OpenNIUserGenerator::getErrorState() const
{
	return m_UserGenerator.GetErrorStateCap().GetErrorState();
}

bool OpenNIUserGenerator::isGenerating() const
{
	if( m_UserGenerator.IsGenerating() == TRUE )
		return true;
	return false;
}

bool OpenNIUserGenerator::isMirrored() const
{
	if( m_UserGenerator.GetMirrorCap().IsMirrored() == TRUE )
		return true;
	return false;
}

XnStatus OpenNIUserGenerator::registerCallbacks()
{
	XnCallbackHandle userHandle;
	
	XnCallbackHandle userExitHandle;
	XnCallbackHandle userReentryHandle;

	XnCallbackHandle calibrationStartHandle;
	XnCallbackHandle calibrationCompleteHandle;
	
	m_UserGenerator.RegisterUserCallbacks(newUserCallback, lostUserCallback, (xn::UserGenerator *)&m_UserGenerator, userHandle);
	
	m_UserGenerator.RegisterToUserExit(userExitCallback, (xn::UserGenerator *)&m_UserGenerator, userExitHandle);
	m_UserGenerator.RegisterToUserReEnter(userReentryCallback, (xn::UserGenerator *)&m_UserGenerator, userReentryHandle);
		
	m_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(calibrationStartCallback, (xn::UserGenerator *)&m_UserGenerator, calibrationStartHandle);
	m_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(calibrationCompleteCallback, (xn::UserGenerator *)&m_UserGenerator, calibrationCompleteHandle);

	return getErrorState();
}

XnStatus OpenNIUserGenerator::getUserByID( uint32_t userID, _2RealTrackedUser& user )
{

	if (m_UserGenerator.GetSkeletonCap().IsTracking(userID))
	{
		user.setJoint( JOINT_HEAD, getUserJoint( userID, XN_SKEL_HEAD ) );
		user.setJoint( JOINT_NECK, getUserJoint( userID, XN_SKEL_NECK ) );
		user.setJoint( JOINT_TORSO, getUserJoint( userID, XN_SKEL_TORSO ) );
		user.setJoint( JOINT_WAIST, getUserJoint( userID, XN_SKEL_WAIST ) );
		
		user.setJoint( JOINT_LEFT_COLLAR, getUserJoint( userID, XN_SKEL_LEFT_COLLAR ) );
		user.setJoint( JOINT_LEFT_SHOULDER, getUserJoint( userID, XN_SKEL_LEFT_SHOULDER ) );
		user.setJoint( JOINT_LEFT_ELBOW, getUserJoint( userID, XN_SKEL_LEFT_ELBOW ) );
		user.setJoint( JOINT_LEFT_WRIST, getUserJoint( userID, XN_SKEL_LEFT_WRIST ) );
		user.setJoint( JOINT_LEFT_HAND, getUserJoint( userID, XN_SKEL_LEFT_HAND ) );
		user.setJoint( JOINT_LEFT_FINGERTIP, getUserJoint( userID, XN_SKEL_LEFT_FINGERTIP ) );

		user.setJoint( JOINT_RIGHT_COLLAR, getUserJoint( userID, XN_SKEL_RIGHT_COLLAR ) );
		user.setJoint( JOINT_RIGHT_SHOULDER, getUserJoint( userID, XN_SKEL_RIGHT_SHOULDER ) );
		user.setJoint( JOINT_RIGHT_ELBOW, getUserJoint( userID, XN_SKEL_RIGHT_ELBOW ) );
		user.setJoint( JOINT_RIGHT_WRIST, getUserJoint( userID, XN_SKEL_RIGHT_WRIST ) );
		user.setJoint( JOINT_RIGHT_HAND, getUserJoint( userID, XN_SKEL_RIGHT_HAND ) );
		user.setJoint( JOINT_RIGHT_FINGERTIP, getUserJoint( userID, XN_SKEL_RIGHT_FINGERTIP ) );

		user.setJoint( JOINT_LEFT_HIP, getUserJoint( userID, XN_SKEL_LEFT_HIP ) );
		user.setJoint( JOINT_LEFT_KNEE, getUserJoint( userID, XN_SKEL_LEFT_KNEE ) );
		user.setJoint( JOINT_LEFT_ANKLE, getUserJoint( userID, XN_SKEL_LEFT_ANKLE ) );
		user.setJoint( JOINT_LEFT_FOOT, getUserJoint( userID, XN_SKEL_LEFT_FOOT ) );

		user.setJoint( JOINT_RIGHT_HIP, getUserJoint( userID, XN_SKEL_RIGHT_HIP ) );
		user.setJoint( JOINT_RIGHT_KNEE, getUserJoint( userID, XN_SKEL_RIGHT_KNEE ) );
		user.setJoint( JOINT_RIGHT_ANKLE, getUserJoint( userID, XN_SKEL_RIGHT_ANKLE ) );
		user.setJoint( JOINT_RIGHT_FOOT, getUserJoint( userID, XN_SKEL_RIGHT_FOOT ) );
	}

	return getErrorState();
}

uint32_t OpenNIUserGenerator::getNrOfUsers() const
{
	return m_UserGenerator.GetNumberOfUsers();
}

XnStatus OpenNIUserGenerator::getTrackedUsers( _2RealTrackedUserVector& users )
{
	users.clear();
	XnUInt16 numberOfUsers = 100;
	XnUserID* currentUsers = new XnUserID[100];

	XnStatus status = m_UserGenerator.GetUsers( currentUsers, numberOfUsers );
	for (int i= 0; i<(int)numberOfUsers; i++)
	{
		_2RealTrackedUser user( currentUsers[i] );
		if ( m_UserGenerator.GetSkeletonCap().IsTracking( currentUsers[i] ) )
		{
			getUserByID( currentUsers[i], user );
			users.push_back( user );
		}
	}

	delete [] currentUsers;
	return ( getErrorState() | status );
}

XnStatus OpenNIUserGenerator::getTrackedUsers( _2RealTrackedUser** userArray, const int size )
{
	//clearing array
	for( int i=0; i < size; ++i )
	{
		if( userArray[i] )
			delete userArray[i];
		userArray[i] = NULL;
	}

	XnUInt16 numberOfUsers = 100;
	XnUserID* currentUsers = new XnUserID[100];

	XnStatus status = m_UserGenerator.GetUsers( currentUsers, numberOfUsers );
	for (int i= 0; i<(int)numberOfUsers; i++)
	{
		_2RealTrackedUser* user = new _2RealTrackedUser( currentUsers[i] );
		if ( m_UserGenerator.GetSkeletonCap().IsTracking( currentUsers[i] ) )
		{
			getUserByID( currentUsers[i], *user );
			userArray[i] = user;
		}
	}

	delete [] currentUsers;
	return ( getErrorState() | status );
}

_2RealTrackedJoint OpenNIUserGenerator::getUserJoint( const uint32_t userID, XnSkeletonJoint type ) const
{
	XnSkeletonJointTransformation joint;
	XnStatus status = m_UserGenerator.GetSkeletonCap().GetSkeletonJoint( userID, type, joint );

	//set position of joint
	_2RealVector3f worldPos = _2RealVector3f( joint.position.position.X, joint.position.position.Y, joint.position.position.Z);
	XnPoint3D screenPos;
	xnConvertRealWorldToProjective( m_UserGenerator.GetHandle(), 1, &joint.position.position, &screenPos ); //convert world to screen positon
	
	//set orienation of joint
	_2RealMatrix3x3 mat;
	for( int i=0; i < 9; ++i )
		mat.elements[i] = joint.orientation.orientation.elements[i];

	_2RealConfidence confidence((float)joint.position.fConfidence, (float)joint.orientation.fConfidence);

	return _2RealTrackedJoint(  (_2RealJointType) type,
								_2RealVector2f( screenPos.X, screenPos.Y ),
								worldPos,
								mat,
								confidence);
}

XnStatus OpenNIUserGenerator::getUserData( int iID, _2RealImageSource<uint16_t>& data ) const
{
	xn::SceneMetaData metadata;
	XnStatus status = m_UserGenerator.GetUserPixels((XnUserID)iID, metadata);

	data.setData(metadata.Data());
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

	if (m_UserGenerator.GetMirrorCap().IsMirrored())
	{
		data.setMirroring(true);
	}
	else
	{
		data.setMirroring(false);
	}

	return (getErrorState() | status);
}

XnStatus OpenNIUserGenerator::setMirroring(const bool mirror)
{
	m_UserGenerator.LockedNodeStartChanges( m_UserLock);
	XnStatus status = m_UserGenerator.GetMirrorCap().SetMirror( mirror );
	m_UserGenerator.LockedNodeEndChanges( m_UserLock );
	return status;
}

void OpenNIUserGenerator::forceResetUser(const uint32_t id)
{
	XnChar calibrationPose[20];
	xn::SkeletonCapability cap = m_UserGenerator.GetSkeletonCap();
	if ( cap.IsTracking( id ) )
	{
		m_UserGenerator.GetSkeletonCap().Reset( id );
		m_UserGenerator.GetSkeletonCap().GetCalibrationPose( calibrationPose );
		m_UserGenerator.GetPoseDetectionCap().StartPoseDetection( calibrationPose, id );
	}
}

void OpenNIUserGenerator::forceResetUsers()
{
	xn::SkeletonCapability cap = m_UserGenerator.GetSkeletonCap();

	XnUInt16 numberOfUsers = 100;
	XnUserID *currentUsers = new XnUserID[100];
	XnChar calibrationPose[20];
	XnStatus status = m_UserGenerator.GetUsers( currentUsers, numberOfUsers );

	for (unsigned int i= 0; i<numberOfUsers; i++)
	{
		XnUserID id = currentUsers[i];
		if ( cap.IsTracking( currentUsers[i] ) )
		{
			m_UserGenerator.GetSkeletonCap().Reset( id );
			m_UserGenerator.GetSkeletonCap().GetCalibrationPose( calibrationPose );
			m_UserGenerator.GetPoseDetectionCap().StartPoseDetection( calibrationPose, id );
		}
	}
}

}
#endif
