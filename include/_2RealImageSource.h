/*
   CADET - Center for Advances in Digital Entertainment Technologies
   Copyright 2011 University of Applied Science Salzburg / MultiMediaTechnology
	   
	   http://www.cadet.at
	   http://multimediatechnology.at/

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Author: Robert Praxmarer, Gerlinde Emsenhuber, Robert Sommeregger
   Email: support@cadet.at
   Created: 08-31-2011
*/

#pragma once
#include <stdint.h>
#include "_2RealVector2f.h"

namespace _2RealKinectWrapper		// this null deleter is needed so the shared pointer doesn't delete the memory allocated by openni, otherwise we will segfault
{
struct null_deleter
{
	void operator()(void const *) const
	{
	}
};

template<typename T>
class _2RealImageSource
{
	public:

		_2RealImageSource();
		~_2RealImageSource();
	
		/*! /brief returns the image data
			/return T* a pointer to the image data
		*/
		boost::shared_array<T>		getData() const;

		/*! /brief returns the frames' full (uncropped) resolution
			/param x a reference to a 32 bit integer where the uncropped width (in pixel) will be stored
			/param y a reference to a 32 bit integer where the uncropped height (in pixel) will be stored
		*/
		_2RealVector2f				getFullResolution() const;

		/*! /brief returns the frames' cropped resolution (same as the full resolution if cropping is disabled)
			x - the width (in pixel)
			y - the height (in pixel)
		*/
		_2RealVector2f				getCroppedResolution() const;

		/*! /brief returns the frames' cropping offset
			x - the horizontal offset, starting from the left
			y - the vertical offset, starting from the top
		*/
		_2RealVector2f				getCroppingOffest() const;

		/*! /brief returns the frames' timestamp
		*/
		uint64_t					getTimestamp() const;

		/*! /brief returns the frame id 
		*/
		uint32_t					getFrameID() const;

		/*! /brief returns the number of bytes per pixel
		*/
		uint32_t					getBytesPerPixel() const;

		/*! /brief returns if the image is mirrored along the vertical axis
		*/
		bool						isMirrored() const;

		/*! /brief returns if the image is cropped
		*/
		bool						isCropped() const;

	private:
	
		// pointer to the image data
		boost::shared_array<T>							m_pData;
		// timestamp as returned by the device
		uint64_t										m_iTimestamp;
		// frame id as returned by the device
		uint32_t										m_iFrameID;
		// whether the image was mirrored
		bool											m_bIsMirrored;
		// whether the image was cropped
		bool											m_bIsCropped;
		// uncropped resolution of the image
		uint32_t										m_iFullResolutionX;
		uint32_t										m_iFullResolutionY;
		// cropped resolution 
		uint32_t										m_iCroppedResolutionX;
		uint32_t										m_iCroppedResolutionY;
		// cropping offset from the top left corner
		uint32_t										m_iCroppingOffsetX;
		uint32_t										m_iCroppingOffsetY;
		// bytes per pixel
		uint32_t										m_iBytesPerPixel;

		void setData( T* data );
		void setFullResolution( const uint32_t x, const uint32_t y );
		void setCroppedResolution( const uint32_t x, const uint32_t y );
		void setCroppingOffest( const uint32_t x, const uint32_t y );
		void setTimestamp( const uint64_t time );
		void setFrameID( const uint32_t id );
		void setBytesPerPixel( const uint32_t bytes );
		void setMirroring( const bool mirrored );
		void setCropping( const bool cropped );

		#ifndef TARGET_MSKINECTSDK
		friend class OpenNIDevice;
		#else
		friend class WSDKDevice;
		#endif
};


template <typename T> 
_2RealImageSource<T>::_2RealImageSource() : m_pData(boost::shared_array<T>()), m_iTimestamp(0), m_iFrameID(0), m_iBytesPerPixel(0), m_bIsCropped(false), m_bIsMirrored(false), m_iCroppingOffsetX(0), 
	m_iCroppingOffsetY(0), m_iFullResolutionX(0),  m_iFullResolutionY(0), m_iCroppedResolutionX(0), m_iCroppedResolutionY(0)
{
}

template <typename T>
_2RealImageSource<T>::~_2RealImageSource()
{
	m_pData = boost::shared_array<T>();
}

template <typename T>
void _2RealImageSource<T>::setData(T* data)
{
	m_pData = boost::shared_array<T>(data, null_deleter()) ;	// null deleter is needed so the shared pointer doesn't delete the memory allocated by openni, otherwise we will segfault
}

template <typename T>
boost::shared_array<T> _2RealImageSource<T>::getData() const
{
	return m_pData;
}

template <typename T>
void _2RealImageSource<T>::setCroppedResolution(const uint32_t x, const uint32_t y)
{
	m_iCroppedResolutionX = x;
	m_iCroppedResolutionY = y;
}

template <typename T>
_2RealVector2f _2RealImageSource<T>::getCroppedResolution() const
{
	return _2RealVector2f( (float)m_iCroppedResolutionX, (float)m_iCroppedResolutionY );
}

template <typename T>
void _2RealImageSource<T>::setFullResolution(const uint32_t x, const uint32_t y)
{
	m_iFullResolutionX = x;
	m_iFullResolutionY = y;
}

template <typename T>
_2RealVector2f _2RealImageSource<T>::getFullResolution() const
{
	return _2RealVector2f( (float)m_iFullResolutionX, (float)m_iFullResolutionY );
}

template <typename T>
void _2RealImageSource<T>::setCroppingOffest(const uint32_t x, const uint32_t y)
{
	m_iCroppingOffsetX = x;
	m_iCroppingOffsetY = y;
}

template <typename T>
_2RealVector2f _2RealImageSource<T>::getCroppingOffest() const
{
	return _2RealVector2f( m_iFullResolutionX, m_iFullResolutionY );
}

template <typename T>
void _2RealImageSource<T>::setTimestamp(const uint64_t timestamp)
{
	m_iTimestamp = timestamp;
}

template <typename T>
uint64_t _2RealImageSource<T>::getTimestamp() const
{
	return m_iTimestamp;
}

template <typename T>
void _2RealImageSource<T>::setFrameID(const uint32_t id)
{
	m_iFrameID = id;
}

template <typename T>
uint32_t _2RealImageSource<T>::getFrameID() const
{
	return m_iFrameID;
}

template <typename T>
bool _2RealImageSource<T>::isMirrored() const
{
	return m_bIsMirrored;
}

template <typename T>
void _2RealImageSource<T>::setMirroring( bool mirrored )
{
	m_bIsMirrored = mirrored;
}

template <typename T>
bool _2RealImageSource<T>::isCropped() const
{
	return m_bIsCropped;
}

template <typename T>
void _2RealImageSource<T>::setCropping( bool cropped )
{
	m_bIsCropped = cropped;
}

template <typename T>
uint32_t _2RealImageSource<T>::getBytesPerPixel() const
{
	return m_iBytesPerPixel;
}

template <typename T>
void _2RealImageSource<T>::setBytesPerPixel(const uint32_t bytes)
{
	m_iBytesPerPixel = bytes;
}

}