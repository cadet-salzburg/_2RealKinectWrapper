#ifndef TARGET_MSKINECTSDK
#include "OpenNIMotorController.h"
#include <iostream>

namespace _2RealKinectWrapper {
	OpenNIMotorController::OpenNIMotorController()
	{
		m_bOpened = false;
	}

	OpenNIMotorController::~OpenNIMotorController()
	{
		Release();
	}
	
	XnStatus OpenNIMotorController::Create()
	{
		const XnUSBConnectionString *paths;
		XnUInt32 count;
		XnStatus res;
	
		// Init OpenNI USB
		res = xnUSBInit();
		//std::cout << xnGetStatusString(res) << " "  <<  res << std::endl;
		if( res != XN_STATUS_OK && res != 131142 /** USB alreay initialized **/ )
			return res;
	
		// list all "Kinect motor" USB devices
		res = xnUSBEnumerateDevices( 0x45e, 0x02b0, &paths, &count );
		if( res != XN_STATUS_OK )
			return res;
		
		const XnChar* pUSBtoUse = paths[0];
		if( count > 0 )
		{

			res = xnUSBOpenDeviceByPath( pUSBtoUse, &m_xDevice );
			for( int i=0; i < count; ++i )
			{
				unsigned short vendor_id;
				unsigned short product_id;
				unsigned char bus;
				unsigned char address;
				sscanf(paths[i], "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address);
				//printf("MotorInfo vendor_id %i product_id %i bus %i address %i \n", vendor_id, product_id, bus, address );
				//std::cout << "MI: " << paths[i] << std::endl;
			}
			if( res != XN_STATUS_OK )
				return res;

			// Init motors
			XnUChar buf;
			res = xnUSBSendControl( m_xDevice, (XnUSBControlType) 0xc0, 0x10, 0x00, 0x00, &buf, sizeof(buf), 0 );
			if( res != XN_STATUS_OK )
			{
				Release();
				return res;
			}
			
			res = xnUSBSendControl( m_xDevice, XN_USB_CONTROL_TYPE_VENDOR, 0x06, 0x01, 0x00, NULL, 0, 0);
			if( res != XN_STATUS_OK )
			{
				Release();
				return res;
			}
			
			m_bOpened = true;
			
			std::cout << "\n The motor controller was initialized correctly" << std::endl;
			return XN_STATUS_OK;
		}

		return XN_STATUS_OS_FILE_OPEN_FAILED;
	}

	void OpenNIMotorController::Release()
	{
		if( m_bOpened )
		{
			SetLED( LED_BLINK_GREEN );
			xnUSBCloseDevice( m_xDevice );
			m_bOpened = false;
		}
	}

	XnStatus OpenNIMotorController::SetLED( LED_STATUS eStatus )
	{
		if( !m_bOpened )
			return XN_STATUS_OS_EVENT_OPEN_FAILED;

		XnStatus res = xnUSBSendControl( m_xDevice, XN_USB_CONTROL_TYPE_VENDOR, 0x06, eStatus, 0x00, NULL, 0, 0 );
		if( res != XN_STATUS_OK )
			return res;
		return XN_STATUS_OK;
	}

	XnStatus OpenNIMotorController::Move( int angle )
	{
		if( !m_bOpened )
			return XN_STATUS_OS_EVENT_OPEN_FAILED;

		XnStatus res = xnUSBSendControl( m_xDevice, XN_USB_CONTROL_TYPE_VENDOR, 0x31, 2 * angle, 0x00, NULL, 0, 0 );
		if( res != XN_STATUS_OK )
			return res;
		return XN_STATUS_OK;
	}

	int OpenNIMotorController::GetAngle() const
	{
		int iA;
		MOTOR_STATUS eStatus;
		XnVector3D vVec;
		GetInformation( iA, eStatus, vVec );
		return iA;
	}

	MOTOR_STATUS OpenNIMotorController::GetMotorStatus() const
	{
		int iA;
		MOTOR_STATUS eStatus;
		XnVector3D vVec;
		GetInformation( iA, eStatus, vVec );
		return eStatus;
	}

	XnVector3D OpenNIMotorController::GetAccelerometer() const
	{
		int iA;
		MOTOR_STATUS eStatus;
		XnVector3D vVec;
		GetInformation( iA, eStatus, vVec );
		return vVec;
	}

	XnStatus OpenNIMotorController::GetInformation( int& rAngle, MOTOR_STATUS& rMotorStatus, XnVector3D& rVec ) const
	{
		XnUChar aData[10];
		XnUInt32 uSize;
		XnStatus res = xnUSBReceiveControl( m_xDevice, XN_USB_CONTROL_TYPE_VENDOR, 0x32, 0x00, 0x00, aData, 10, &uSize, 0 );
		if( res == XN_STATUS_OK )
		{
			rAngle = aData[8];
			if( rAngle > 128 )
				rAngle = -0.5 * ( 255 - rAngle );
			else
				rAngle /= 2;

			if( aData[9] == 0x00 )
				rMotorStatus = MOTOR_STOPPED;
			else if( aData[9] == 0x01 )
				rMotorStatus = MOTOR_LIMIT;
			else if( aData[9] == 0x04 )
				rMotorStatus = MOTOR_MOVING;
			else
				rMotorStatus = MOTOR_UNKNOWN;

			rVec.X = (float)( ((XnUInt16)aData[2] << 8) | aData[3] );
			rVec.Y = (float)( ((XnUInt16)aData[4] << 8) | aData[5] );
			rVec.Z = (float)( ((XnUInt16)aData[6] << 8) | aData[7] );
		}
		return res;
	}
}; //end namespace
#endif
