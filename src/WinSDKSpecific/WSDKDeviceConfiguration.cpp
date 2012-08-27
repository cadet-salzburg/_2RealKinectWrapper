#ifdef TARGET_MSKINECTSDK
#include "WSDKDeviceConfiguration.h"

using namespace _2RealKinectWrapper;

WSDKDeviceConfiguration::ImageRes::ImageRes()
	: WSDKResType( NUI_IMAGE_RESOLUTION_INVALID ),
	  width( 0 ),
	  height( 0 )
{

}


WSDKDeviceConfiguration::WSDKDeviceConfiguration()
	: m_Generators2Real( IMAGE_CONFIG_INVALID ),
	  m_GeneratorsWSDK( IMAGE_CONFIG_INVALID ),
	  m_ImageConfig2Real( IMAGE_CONFIG_INVALID )
{

}

WSDKDeviceConfiguration::~WSDKDeviceConfiguration()
{

}

void WSDKDeviceConfiguration::reset()
{
	m_Generators2Real = m_GeneratorsWSDK = m_ImageConfig2Real = 0;
	m_ImageResColor = m_ImageResDepth = m_ImageResUser = m_ImageResInfrared = ImageRes();
}
#endif
