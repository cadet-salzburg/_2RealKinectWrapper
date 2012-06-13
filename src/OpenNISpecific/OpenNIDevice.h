#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <XnCppWrapper.h>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include "_2RealTrackedUser.h"
#include "OpenNIMotorController.h"
#include <memory>


namespace _2RealKinectWrapper {

	typedef boost::shared_array< unsigned char >				ImageDataRef;
	typedef boost::shared_array< uint16_t >						ImageData16Ref;
	typedef boost::shared_ptr< xn::NodeInfo >					NodeInfoRef;
	typedef boost::shared_ptr< xn::Generator >					GeneratorRef;
	typedef boost::shared_ptr< class OpenNIDevice >				OpenNIDeviceRef;
	typedef std::vector<boost::shared_ptr<_2RealTrackedUser> >	_2RealTrackedUserVector;
	typedef boost::shared_ptr< class _2RealTrackedJoint >		_2RealTrackedJointRef;

	class OpenNIDevice
	{
	public:

		OpenNIDevice();
		OpenNIDevice( const xn::Context context,  NodeInfoRef deviceInfo, const std::string deviceName );

		void							addDeviceToContext();

		void							addGenerator( const XnPredefinedProductionNodeType &nodeType, uint32_t configureImages );
		void							removeGenerator( const XnPredefinedProductionNodeType &nodeType );
		bool							hasGenerator( const XnPredefinedProductionNodeType &nodeType ) const;
		void							startGenerator( const XnPredefinedProductionNodeType &nodeType );
		void							stopGenerator( const XnPredefinedProductionNodeType &nodeType );

		bool							hasNewData( const XnPredefinedProductionNodeType &nodeType );

		ImageDataRef					getBuffer( const XnPredefinedProductionNodeType &nodeType );
		ImageData16Ref					getBuffer16( const XnPredefinedProductionNodeType &nodeType );

		xn::NodeInfo					getNodeInfo();
		xn::NodeInfoList				getNodeInfoList( const XnPredefinedProductionNodeType &nodeType  );

		void							convertRealWorldToProjective( XnUInt32 count, 		const XnPoint3D  	aRealWorld[], XnPoint3D  	aProjective[] );
		void							convertProjectiveToRealWorld( XnUInt32 count, 		const XnPoint3D  	aProjective[], XnPoint3D  	aRealWorld[] );

		void							getExistingProductionNode( const XnPredefinedProductionNodeType &nodeType, xn::ProductionNode& productionNode ) const;
		static std::string				xnNodeTypeToString( const XnPredefinedProductionNodeType& nodeType );

		_2RealTrackedUserVector			getUsers();
		size_t							getNumberOfUsers();

		XnStatus						getUserByID( uint32_t userID, boost::shared_ptr<_2RealTrackedUser> user );

		XnSkeletonJointTransformation	getSkeletonJoint( XnUInt16 userIdx, XnSkeletonJoint jointType);
		_2RealTrackedJointRef			getUserJoint( const uint32_t userID, XnSkeletonJoint type );
		void							forceResetUser( const uint32_t id );
		void							forceResetUsers( );

		void							setMotorAngle( int angle );
		int								getMotorAngle();

	private:
		void							convertImage_16_to_8( const boost::shared_array<uint16_t> source, boost::shared_array<unsigned char> destination, uint32_t size, const int normalizing )
		{
			for( unsigned int i=0; i<size; ++i )
			{
				destination[i] = (unsigned char) ( source[i] * ( (float)( 1 << 8 ) / normalizing ) ); 	
			}
		};

		XnMapOutputMode					getRequestedOutputMode( const XnPredefinedProductionNodeType &nodeType, uint32_t configureImages );
		mutable xn::Context				m_Context;
		std::string						m_DeviceName;
		NodeInfoRef						m_DeviceInfo;

		XnCallbackHandle				userCbHandle;
		XnCallbackHandle				calibrationStartedCbHandle;
		XnCallbackHandle				calibrationCompletedCbHandle;
		XnCallbackHandle				userExitCbHandle;
		XnCallbackHandle				userReentryCbHandle;
		XnCallbackHandle				poseDetectedCbHandle;

		xn::ImageGenerator				m_ImageGenerator;
		xn::DepthGenerator				m_DepthGenerator;
		xn::UserGenerator				m_UserGenerator;
		xn::IRGenerator					m_IrGenerator;

		//image buffer obtained from kinect
		_2RealImageSource<uint8_t>				m_ColorImage;
		_2RealImageSource<uint16_t>				m_DepthImage, m_InfraredImage, m_UserImage;

		//converted image buffers
		boost::shared_array<unsigned char>	m_DepthImage_8bit;
		boost::shared_array<unsigned char>	m_InfraredImage_8bit;
		boost::shared_array<unsigned char>	m_UserImage_8bit;
		boost::shared_array<unsigned char>	m_UserImageColor_8bit;
		
		XnUInt16						m_NumUsers;
		XnUserID						*m_Users;

		void							registerUserCallbacks();
		XnStatus						getErrorState() const;
		OpenNIMotorController			m_DeviceMotorController;
		bool							m_MotorInitialized;
	};
} //namespace

