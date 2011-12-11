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

	CADET - Center for Advances in Digital Entertainment Technologies
	 
	Authors: Robert Praxmarer, Gerlinde Emsenhuber, Robert Sommeregger
	Email: support@cadet.at
	Created: 08-09-2011
*/

// Cinder Includes
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Cinder.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rect.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"

// _2RealKinect Include
//#define TARGET_MSKINECTSDK		// use this for MS Kinect SDK, comment it or just not define it for using OpenNI
#include "_2RealKinect.h"

// Namespaces
using namespace ci;
using namespace ci::app;
using namespace std;
using namespace _2Real;


class MultipleKinectApp : public AppBasic
{
	public:
		~MultipleKinectApp();

		void	setup();
		void	draw();
		void	resize( ResizeEvent event);
		void	keyDown( KeyEvent event );
		void	prepareSettings( Settings* settings );

	private:
		void			drawKinectImages();
		void			drawSkeletons(int deviceID, ci::Rectf rect);
		void			resizeImages();
		void			mirrorImages();
		unsigned char*	getImageData( int deviceID, _2RealGenerator imageType, int& imageWidth, int& imageHeight, int& bytePerPixel );

		_2RealKinect*				m_2RealKinect;
		bool						m_bIsMirroring;
		int							m_iKinectWidth;
		int							m_iKinectHeight;
		int							m_iScreenWidth;
		int							m_iScreenHeight;
		int							m_iNumberOfDevices;
		ci::Vec2f					m_ImageSize;
		ci::Font					m_Font;
};

MultipleKinectApp::~MultipleKinectApp()
{
	m_2RealKinect->shutdown();
}

void MultipleKinectApp::prepareSettings( Settings* settings )
{
// just open windows console if compiled for windows
#ifdef _WIN32 || _WIN64							
		FILE* f;
	AllocConsole();
	freopen_s( &f, "CON", "w", stdout );
#endif

	settings->setTitle("CADET | http://www.cadet.at | MultipleKinectSample");
	settings->setWindowSize( 1024, 768 );
}

void MultipleKinectApp::setup()
{
	m_iKinectWidth = 640;
	m_iKinectHeight = 480;
	m_iScreenWidth = 1024;
	m_iScreenHeight = 768;
	m_bIsMirroring = true;	// generators are mirrored by default

	try
	{
		m_2RealKinect = _2RealKinect::getInstance();
		std::cout << "_2RealKinectWrapper Version: " << m_2RealKinect->getVersion() << std::endl;
		bool bResult = m_2RealKinect->start( COLORIMAGE | USERIMAGE | DEPTHIMAGE );
		if( bResult )
			std::cout << "\n\n_2RealKinectWrapper started successfully!...";
		
		m_iNumberOfDevices = m_2RealKinect->getNumberOfDevices();
		resizeImages();
	}
	catch ( std::exception& e )
	{
		std::cout << "\n\n_2Real: Exception occurred when trying to start:\n" << e.what() << std::endl;
		m_2RealKinect->shutdown();
		int pause = 0;
		std::cin >> pause;
	}

	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();

	m_Font = Font("Arial", 24);
}

void MultipleKinectApp::draw()
{
	try
	{
		gl::clear();
		gl::color( Color(1,1,1) );
		drawKinectImages();				//oututs all connected kinect devices generators (depth, rgb, user imgage, skeletons)			
	}
	catch( std::exception& e )
	{
		std::cout << "\nException in draw: " << e.what() << std::endl;
	}
}

void MultipleKinectApp::drawKinectImages()
{			
	unsigned char* imgRef;
	int numberChannels = 0;
	for( int i = 0; i < m_iNumberOfDevices; ++i)
	{
		ci::Rectf destinationRectangle( m_ImageSize.x * i, 0, m_ImageSize.x * (i+1), m_ImageSize.y);
		
		//rgb image
		imgRef = getImageData( i, COLORIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
		Surface8u color( imgRef, m_iKinectWidth, m_iKinectHeight, m_iKinectWidth*numberChannels, SurfaceChannelOrder::RGB );
		gl::draw( gl::Texture( color ), destinationRectangle);
		
		//depth image		
		imgRef = getImageData( i, DEPTHIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
		Channel depth( m_iKinectWidth, m_iKinectHeight, m_iKinectWidth, numberChannels, imgRef );
		destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );
		gl::draw( gl::Texture( depth ),  destinationRectangle );		
		
		//user image
#ifdef TARGET_MSKINECTSDK
		if( i == 0 )						
#endif
		{
			imgRef = getImageData( i, USERIMAGE_COLORED, m_iKinectWidth, m_iKinectHeight, numberChannels);
			Surface8u userColored( imgRef, m_iKinectWidth, m_iKinectHeight, m_iKinectWidth*3, SurfaceChannelOrder::RGB );
			destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );			
			gl::draw( gl::Texture( userColored ), destinationRectangle );
		}
		
		//skeleton		
		m_iKinectWidth = m_2RealKinect->getImageWidth( i, COLORIMAGE );		
		m_iKinectHeight = m_2RealKinect->getImageHeight( i, COLORIMAGE );
		destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );
		drawSkeletons(i, destinationRectangle );
		
		//drawing debug strings for devices
		gl::disableDepthRead();
		gl::color(Color( 1.0, 1.0, 1.0 ));	
		gl::drawString( "Device "+ toString(i), Vec2f( m_ImageSize.x * i + 20 , 0 ), Color( 1.0f, 0.0f, 0.0f ), m_Font );		
		gl::enableDepthRead();
	}
}

unsigned char* MultipleKinectApp::getImageData( int deviceID, _2RealGenerator imageType, int& imageWidth, int& imageHeight, int& bytePerPixel )
{
	bytePerPixel = m_2RealKinect->getBytesPerPixel( imageType );
	imageWidth = m_2RealKinect->getImageWidth( deviceID, imageType );		
	imageHeight = m_2RealKinect->getImageHeight( deviceID, imageType );

	return m_2RealKinect->getImageData( deviceID, imageType);
}

void MultipleKinectApp::drawSkeletons(int deviceID, ci::Rectf rect)
{
	float fRadius = 10.0;

	glPushMatrix();
	
	glTranslatef( rect.getX1(), rect.getY1(), 0 );
	glScalef( rect.getWidth()/(float)m_iKinectWidth, rect.getHeight()/(float)m_iKinectHeight, 1);

	_2RealPositionsVector2f::iterator iter;
	int numberOfUsers = m_2RealKinect->getNumberOfUsers( deviceID );

	for( int i = 0; i < numberOfUsers; ++i)
	{		
		glColor3f( 0, 1.0, 0.0 );				
		_2RealPositionsVector2f skeletonPositions = m_2RealKinect->getSkeletonScreenPositions( deviceID, i );
		
		_2RealOrientationsMatrix3x3 skeletonOrientations;
		if(m_2RealKinect->hasFeatureJointOrientation())
			skeletonOrientations = m_2RealKinect->getSkeletonWorldOrientations( deviceID, i );

		int size = skeletonPositions.size();		
		for(int j = 0; j < size; ++j)
		{	
			gl::pushModelView();
			if( m_2RealKinect->isJointAvailable( (_2RealJointType)j ) && skeletonPositions[j].confidence>0.0)
			{
				glTranslatef(Vec3f( skeletonPositions[j].x, skeletonPositions[j].y, 0 ));

				if(m_2RealKinect->hasFeatureJointOrientation())
				{
					Matrix44<float> rotMat  = gl::getModelView();
					rotMat.m00 = skeletonOrientations[j].elements[0];
					rotMat.m01 = skeletonOrientations[j].elements[1];
					rotMat.m02 = skeletonOrientations[j].elements[2];
					rotMat.m10 = skeletonOrientations[j].elements[3];
					rotMat.m11 = skeletonOrientations[j].elements[4];
					rotMat.m12 = skeletonOrientations[j].elements[5];
					rotMat.m20 = skeletonOrientations[j].elements[6];
					rotMat.m21 = skeletonOrientations[j].elements[7];
					rotMat.m22 = skeletonOrientations[j].elements[8];
					glLoadMatrixf(rotMat);		
					gl::drawCoordinateFrame(fRadius);
				}
				else
				{
					gl::drawSolidCircle( Vec2f( 0, 0 ), fRadius);
				}
			}
			gl::popModelView();
		}
	}	
	glPopMatrix();
}

void MultipleKinectApp::resize( ResizeEvent event)
{
	m_iScreenWidth = getWindowWidth();
	m_iScreenHeight = getWindowHeight();
	resizeImages();
}

void MultipleKinectApp::resizeImages()
{
	//calculate imagesize
	int iImageHeight = (int)(m_iScreenHeight / 4.0);		// divide window height according to the number of generator outputs (rgb, depth, user, skeleton)
	int iImageWidth = (int)(iImageHeight * 4.0 / 3.0);		// keep images aspect ratio 4:3
	if(iImageWidth * m_iNumberOfDevices > m_iScreenWidth)	// aspect ratio 	
	{
		iImageWidth = m_iScreenWidth / m_iNumberOfDevices;
		iImageHeight = iImageWidth * 3 / 4;
	}
	//size of plane to draw textures on
	m_ImageSize = Vec2f( (float)iImageWidth, (float)iImageHeight );
}

void MultipleKinectApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'm' )	// mirror generators
		mirrorImages();
	if( event.getChar() == 'r' )	// restart kinects
	{
		bool bResult = m_2RealKinect->restart();
		if( bResult )
			std::cout << "\n\n_2Real started successfully!...";
		m_iNumberOfDevices = m_2RealKinect->getNumberOfDevices();
	}
	if( event.getChar() == 'u' )	// reset all calibrated users (OpenNI only)
		m_2RealKinect->resetAllSkeletons();
}

void MultipleKinectApp::mirrorImages()
{
	m_bIsMirroring = !m_bIsMirroring;	// toggleMirroring
	for( int i = 0; i < m_iNumberOfDevices; ++i)
	{
		m_2RealKinect->setMirrored( i, COLORIMAGE, m_bIsMirroring );
		m_2RealKinect->setMirrored( i, DEPTHIMAGE, m_bIsMirroring );
		m_2RealKinect->setMirrored( i, USERIMAGE, m_bIsMirroring );		// OpenNI has no capability yet to mirror the user image
	}		
}

CINDER_APP_BASIC( MultipleKinectApp, RendererGl )
