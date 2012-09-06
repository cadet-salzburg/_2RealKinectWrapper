#pragma once
#include "XnOpenNI.h"
#include <boost/integer.hpp>   

namespace _2RealKinectWrapper
{

/*!
* Holding information about configurations of a WSDK-Generator
*/
class OpenNIDeviceConfiguration
{
public:
	OpenNIDeviceConfiguration();
	~OpenNIDeviceConfiguration();
	
	void setMirror( XnPredefinedProductionNodeType nodeType, bool isMirrored );
	void setImageConfig( boost::uint32_t imageConfig );
	void setGeneratorConfig( boost::uint32_t generatorConfig) ;

	/*! /brief     Resets all the stored values to its default
	!*/
	void					reset();

	uint32_t				m_GeneratorConfig2Real;		// 2real per bit information about generators
	uint32_t				m_ImageConfig2Real;				// 2real per bit information about image configuration
	bool					m_ImageMirror;
	bool					m_DepthMirror;
	bool					m_UserMirror;
	bool					m_IrMirror;
};
}