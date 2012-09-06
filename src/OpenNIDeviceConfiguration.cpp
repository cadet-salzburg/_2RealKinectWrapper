#ifndef TARGET_MSKINECTSDK
#include "OpenNIDeviceConfiguration.h"
#include "_2RealUtility.h"

using namespace _2RealKinectWrapper;


OpenNIDeviceConfiguration::OpenNIDeviceConfiguration()
	:m_GeneratorConfig2Real( 0 ),
	 m_ImageConfig2Real( 0 ),
	 m_ImageMirror( false ),
	 m_DepthMirror( false ),
	 m_UserMirror( false ),
	 m_IrMirror( false )
{

}

OpenNIDeviceConfiguration::~OpenNIDeviceConfiguration()
{

}

void OpenNIDeviceConfiguration::reset()
{
	m_GeneratorConfig2Real = m_ImageConfig2Real = 0;
	m_ImageMirror = m_DepthMirror = m_UserMirror = m_IrMirror = 0;

}

void OpenNIDeviceConfiguration::setMirror( XnPredefinedProductionNodeType nodeType, bool isMirrored )
{
	if ( nodeType == XN_NODE_TYPE_IMAGE )
	{
		m_ImageMirror = isMirrored;
	}
	else if  ( nodeType == XN_NODE_TYPE_DEPTH )
	{
		m_DepthMirror = isMirrored;
	}
	else if  ( nodeType == XN_NODE_TYPE_USER )
	{
		m_UserMirror = isMirrored;
	}
	else if  ( nodeType == XN_NODE_TYPE_IR )
	{
		m_IrMirror = isMirrored;
	}
	else
	{
		_2REAL_LOG(warn) << "_2Real: node-type supplied does not support mirroring..." << std::endl;
	}
}

void OpenNIDeviceConfiguration::setImageConfig( uint32_t imageConfig )
{
	m_ImageConfig2Real = imageConfig;
}

void OpenNIDeviceConfiguration::setGeneratorConfig( uint32_t generatorConfig )
{
	m_GeneratorConfig2Real = generatorConfig;
}

#endif
