#ifdef TARGET_MSKINECTSDK
#include "WSDKDeviceConfiguration.h"


WSDKDeviceConfiguration::ImageRes::ImageRes()
	: WSDKResType( NUI_IMAGE_RESOLUTION_INVALID ),
	  width( 0 ),
	  height( 0 )
{

}


WSDKDeviceConfiguration::WSDKDeviceConfiguration()
	: m_Generators2Real( 0 ),
	  m_GeneratorsWSDK( 0 ),
	  m_ImageConfig2Real( 0 )
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