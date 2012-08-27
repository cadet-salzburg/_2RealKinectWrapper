#pragma once
#include <Windows.h>
#include "NuiApi.h"
#include "_2RealTypes.h"

namespace _2RealKinectWrapper
{

/*!
* Holding information about configurations of a WSDK-Generator
*/
class WSDKDeviceConfiguration
{
	struct ImageRes
	{
		ImageRes();

		NUI_IMAGE_RESOLUTION	WSDKResType;
		uint16_t				width;
		uint16_t				height;
	};

	public:
	WSDKDeviceConfiguration();
	~WSDKDeviceConfiguration();
	/*! /brief     Resets all the stored values to its default
	!*/
	void					reset();

	uint32_t				m_Generators2Real;		// 2real per bit information about generators
	uint32_t				m_GeneratorsWSDK;		// NUI per bit information about generators
	uint32_t				m_ImageConfig2Real;		// 2real per bit information about image configuration
	ImageRes				m_ImageResColor;
	ImageRes				m_ImageResDepth;
	ImageRes				m_ImageResUser;
	ImageRes				m_ImageResInfrared;
};


}
