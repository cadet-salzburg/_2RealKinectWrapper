#pragma once
#include <XnUSB.h>
#include <XnTypes.h>


namespace _2RealKinectWrapper {

	enum LED_STATUS
	{
		LED_OFF					= 0,
		LED_GREEN				= 1,
		LED_RED					= 2,
		LED_YELLOW				= 3,
		LED_BLINK_YELLOW		= 4,
		LED_BLINK_GREEN			= 5,
		LED_BLINK_RED_YELLOW	= 6
	};

	enum MOTOR_STATUS
	{
		MOTOR_STOPPED	= 0x00,
		MOTOR_LIMIT		= 0x01,
		MOTOR_MOVING	= 0x04,
		MOTOR_UNKNOWN	= 0x08
	};

	class OpenNIMotorController
	{
	public:
		OpenNIMotorController();
		~OpenNIMotorController();
		XnStatus Create();
		void Release();
		XnStatus SetLED( LED_STATUS eStatus );
		XnStatus Move( int angle );
		int GetAngle() const;
		MOTOR_STATUS GetMotorStatus() const;
		XnVector3D GetAccelerometer() const;
		XnStatus GetInformation( int& rAngle, MOTOR_STATUS& rMotorStatus, XnVector3D& rVec ) const;
	private:
		bool				m_bOpened;
		XN_USB_DEV_HANDLE	m_xDevice;
	};
};