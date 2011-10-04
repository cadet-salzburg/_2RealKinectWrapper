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
#include "OpenNISpecific/OpenNIColorGenerator.h"
#include "_2RealImageSource.h"

namespace _2Real
{

void XN_CALLBACK_TYPE colorErrorStateChanged(xn::MapGenerator& rMapGenerator, void *pCookie)
{
	XnStatus status;
	xn::DepthGenerator *depthGenerator = static_cast<xn::DepthGenerator*>(pCookie);
	
	status = depthGenerator->GetErrorStateCap().GetErrorState();
	if (status != XN_STATUS_OK)
	{
		_2REAL_LOG(warn) << "color generator is in error state; status: " << std::string(xnGetStatusString(status)) << std::endl;
	}
}

void XN_CALLBACK_TYPE colorOutputStateChanged(xn::MapGenerator& rMapGenerator, void *pCookie)
{
	_2REAL_LOG(info) << "color map resolution was changed\n";
}

OpenNIColorGenerator::OpenNIColorGenerator()
{
}

OpenNIColorGenerator::~OpenNIColorGenerator()
{
}

XnStatus OpenNIColorGenerator::startGenerating()
{
	m_ColorGenerator.LockedNodeStartChanges(m_ColorLock);
	XnStatus status = m_ColorGenerator.StartGenerating();
	m_ColorGenerator.LockedNodeEndChanges(m_ColorLock);
	return (getErrorState() | status);
}

XnStatus OpenNIColorGenerator::getTimestamp( uint64_t& time ) const
{
	time = m_ColorGenerator.GetTimestamp();
	return getErrorState();
}

XnStatus OpenNIColorGenerator::stopGenerating()
{
	m_ColorGenerator.LockedNodeStartChanges(m_ColorLock);
	XnStatus status = m_ColorGenerator.StopGenerating();
	m_ColorGenerator.LockedNodeEndChanges(m_ColorLock);
	return (getErrorState() | status);
}

XnStatus OpenNIColorGenerator::lockGenerator()
{
	XnStatus status = m_ColorGenerator.LockForChanges(&m_ColorLock);

	return (getErrorState() | status);
}

XnStatus OpenNIColorGenerator::unlockGenerator()
{
	m_ColorGenerator.UnlockForChanges(m_ColorLock);	

	return getErrorState();
}

XnStatus OpenNIColorGenerator::getErrorState() const
{
	return m_ColorGenerator.GetErrorStateCap().GetErrorState();
}

bool OpenNIColorGenerator::isGenerating() const
{
	if( m_ColorGenerator.IsGenerating() == TRUE )
		return true;
	return false;
}

bool OpenNIColorGenerator::isMirrored() const
{
	if( m_ColorGenerator.GetMirrorCap().IsMirrored() == TRUE )
		return true;
	return false;
}

XnStatus OpenNIColorGenerator::setOutputMode(const XnMapOutputMode outputMode)
{
	m_ColorGenerator.LockedNodeStartChanges(m_ColorLock);
	XnStatus status = m_ColorGenerator.SetMapOutputMode(outputMode);
	m_ColorGenerator.LockedNodeEndChanges(m_ColorLock);
	return (getErrorState() | status);
}

XnStatus OpenNIColorGenerator::registerCallbacks()
{	
	XnCallbackHandle errorStateHandle;
	XnStatus status = m_ColorGenerator.RegisterToNewDataAvailable((xn::StateChangedHandler)colorErrorStateChanged, (xn::ImageGenerator *)&m_ColorGenerator, errorStateHandle);

	XnCallbackHandle outputStateHandle;
	status = m_ColorGenerator.RegisterToMapOutputModeChange((xn::StateChangedHandler)colorOutputStateChanged, (xn::ImageGenerator *)&m_ColorGenerator, outputStateHandle);
	
	return (getErrorState() | status);
}

XnStatus OpenNIColorGenerator::getData( _2RealImageSource<uint8_t>& data ) const
{
	xn::ImageMetaData metadata;
	m_ColorGenerator.GetMetaData(metadata);

	data.setData(m_ColorGenerator.GetImageMap());
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

	if (m_ColorGenerator.GetMirrorCap().IsMirrored())
	{
		data.setMirroring(true);
	}
	else
	{
		data.setMirroring(false);
	}

	return getErrorState();
}

XnStatus OpenNIColorGenerator::setMirroring(const bool mirror)
{
	m_ColorGenerator.LockedNodeStartChanges(m_ColorLock);
	XnStatus status = m_ColorGenerator.GetMirrorCap().SetMirror(mirror);
	m_ColorGenerator.LockedNodeEndChanges(m_ColorLock);
	return status;
}

XnStatus OpenNIColorGenerator::getFramesPerSecond( int &fps ) const
{
	XnMapOutputMode outputMode;
	m_ColorGenerator.GetMapOutputMode(outputMode);
	fps = outputMode.nFPS;
	return getErrorState();
}

XnStatus OpenNIColorGenerator::getMapResolution( uint32_t& x, uint32_t& y ) const
{
	XnMapOutputMode outputMode;
	m_ColorGenerator.GetMapOutputMode(outputMode);
	x = outputMode.nXRes;
	y = outputMode.nYRes;
	return getErrorState();
}

}
#endif