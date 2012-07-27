/*
	CADET - Center for Advances in Digital Entertainment Technologies
	Copy01right 2011 University of Applied Science Salzburg / MultiMediaTechnology

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
using namespace _2RealKinectWrapper;


class MultipleKinectApp : public AppBasic
{
	public:
		MultipleKinectApp();
		~MultipleKinectApp();

		void	setup();
		void	update();
		void	draw();
		void	resize( ResizeEvent event);
		void	keyDown( KeyEvent event );
		void	prepareSettings( Settings* settings );

	private:
		void								drawKinectImages();
		void								drawSkeletons(int deviceID, ci::Rectf rect);
		void								drawCenterOfMasses(int deviceID, ci::Rectf destRect);
		void								resizeImages();
		void								mirrorImages();
		boost::shared_array<unsigned char>	getImageData( int deviceID, _2RealGenerator imageType, int& imageWidth, int& imageHeight, int& bytePerPixel );

		_2RealKinect*				m_2RealKinect;
		bool						m_bIsMirroring;
		int							m_iKinectWidth;
		int							m_iKinectHeight;
		int							m_iScreenWidth;
		int							m_iScreenHeight;
		int							m_iNumberOfDevices;
		ci::Vec2f					m_ImageSize;
		ci::Font					m_Font;
		int							m_iMotorValue;
		bool						m_Align;
};

MultipleKinectApp::MultipleKinectApp() : m_iScreenWidth(1280), m_iScreenHeight(1024), m_Align(false)
{

}

MultipleKinectApp::~MultipleKinectApp()
{
	m_2RealKinect->shutdown();
}

void MultipleKinectApp::prepareSettings( Settings* settings )
{
// just open windows console if compiled for windows
#if defined(_WIN32) || defined(_WIN64)				
	FILE* f;
	AllocConsole();
	freopen_s( &f, "CON", "w", stdout );
#endif

	settings->setTitle("CADET | http://www.cadet.at | MultipleKinectSample");
	settings->setWindowSize( m_iScreenWidth, m_iScreenHeight );
}

void MultipleKinectApp::setup()
{
	m_iKinectWidth = 640;
	m_iKinectHeight = 480;
	m_bIsMirroring = true;	
	m_iMotorValue = 0;

	try
	{
		m_2RealKinect = _2RealKinect::getInstance();
		std::cout << "_2RealKinectWrapper Version: " << m_2RealKinect->getVersion() << std::endl;
		bool bResult = false;
		//m_iNumberOfDevices = m_2RealKinect->getNumberOfDevices();
		m_iNumberOfDevices = 1;
		for ( int devIdx=0; devIdx < m_iNumberOfDevices ; ++devIdx )
		{
			//bResult = m_2RealKinect->configure( devIdx,  COLORIMAGE, IMAGE_COLOR_1280X1024 );
			bResult = m_2RealKinect->configure( devIdx,  COLORIMAGE, IMAGE_COLOR_640X480 );
			if( bResult )
			{
				std::cout << "_2RealKinectWrapper Device " << devIdx << " started successfully!..." << std::endl;
			}
			//m_iMotorValue = m_2RealKinect->getMotorAngle( devIdx );	// just make motor device 0 controllable
			m_2RealKinect->startGenerator( devIdx,  COLORIMAGE );
			//m_2RealKinect->startGenerator( devIdx,  COLORIMAGE );
			try
			{
				m_2RealKinect->setMirrored( devIdx, COLORIMAGE, true );
			}
			catch (...)
			{
				std::cout << "Failed to mirror" << std::endl;
			}
		}
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

void MultipleKinectApp::update()
{

}

void MultipleKinectApp::draw()
{
	try
	{
		gl::clear();
		gl::color( Color(1,1,1) );
		drawKinectImages();		
	}
	catch( std::exception& e )
	{
		std::cout << "\nException in draw: " << e.what() << std::endl;
	}
}

void MultipleKinectApp::drawKinectImages()
{			
	boost::shared_array<unsigned char> imgRef;
	int numberChannels = 0;



 	for( int i = 0; i < m_iNumberOfDevices; ++i)
	{
		//---------------IR Image---------------------//
		ci::Rectf destinationRectangle( m_ImageSize.x * i, 0, m_ImageSize.x * (i+1), m_ImageSize.y);
//		m_iKinectWidth  = m_2RealKinect->getImageWidth(i, COLORIMAGE );
//		m_iKinectHeight = m_2RealKinect->getImageHeight(i, COLORIMAGE );
//		imgRef = getImageData( i, COLORIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
//		Channel depth( m_iKinectWidth, m_iKinectHeight, m_iKinectWidth, numberChannels, imgRef.get() );
//		destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );
//		gl::draw( gl::Texture( depth ),  destinationRectangle );

		//---------------Color Image---------------------//
		m_iKinectWidth = m_2RealKinect->getImageWidth(i, COLORIMAGE );
		m_iKinectHeight = m_2RealKinect->getImageHeight(i, COLORIMAGE );
		m_iKinectWidth = 640;
		m_iKinectHeight = 480;
	//	ci::Rectf destinationRectangle( m_ImageSize.x * i, 0, m_ImageSize.x * (i+1), m_ImageSize.y);
		imgRef = getImageData( i, COLORIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
		Surface8u color( imgRef.get(), m_iKinectWidth, m_iKinectHeight, m_iKinectWidth*numberChannels, SurfaceChannelOrder::RGB );
		gl::draw( gl::Texture( color ), destinationRectangle );
//
//		//---------------Depth Image---------------------//
//		m_iKinectWidth = m_2RealKinect->getImageWidth(i, DEPTHIMAGE );
//		m_iKinectHeight = m_2RealKinect->getImageHeight(i, DEPTHIMAGE );
//		imgRef = getImageData( i, DEPTHIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
//		Channel depth( m_iKinectWidth, m_iKinectHeight, m_iKinectWidth, numberChannels, imgRef.get() );
//		destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );
//		gl::draw( gl::Texture( depth ),  destinationRectangle );
//
//		//---------------User Image---------------------//
//#ifdef TARGET_MSKINECTSDK
//		if( i == 0 )						
//#endif
//		{
//			m_iKinectWidth = m_2RealKinect->getImageWidth(i, USERIMAGE );
//			m_iKinectHeight = m_2RealKinect->getImageHeight(i, USERIMAGE );
//			imgRef = getImageData( i, USERIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
//			if( imgRef )
//			{
//				Surface8u userColored( imgRef.get(), m_iKinectWidth, m_iKinectHeight, m_iKinectWidth*3, SurfaceChannelOrder::RGB );
//				destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );			
//				gl::draw( gl::Texture( userColored ), destinationRectangle );
//#ifndef		TARGET_MSKINECTSDK
//				drawCenterOfMasses(i, destinationRectangle);
//#endif
//			}
//			// draw nrOfUsers with font
//			gl::disableDepthRead();
//			gl::drawString( "Users: "+ toString(m_2RealKinect->getNumberOfUsers(i)), Vec2f( destinationRectangle.x1 + 20 , destinationRectangle.y1 ), Color( 1.0f, 0.0f, 0.0f ), m_Font );	
//			gl::enableDepthRead();
//		}
//		//---------------Skeletons---------------------//	
//		destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );
//		drawSkeletons(i, destinationRectangle );
//
//		gl::disableDepthRead();
//		gl::drawString( "Skeletons: "+ toString(m_2RealKinect->getNumberOfSkeletons(i)), Vec2f( destinationRectangle.x1 + 20 , destinationRectangle.y1 ), Color( 1.0f, 0.0f, 0.0f ), m_Font );	
//		gl::enableDepthRead();
//
//		//drawing debug strings for devices
//
//		gl::disableDepthRead();
//		gl::color(Color( 1.0, 1.0, 1.0 ));	
//		gl::drawString( "Device "+ toString(i), Vec2f( m_ImageSize.x * i + 20 , 0 ), Color( 1.0f, 0.0f, 0.0f ), m_Font );		
//		//draw fps
//		gl::drawString( "fps: " + toString(getAverageFps()), Vec2f( float(getWindowWidth() - 120), 10.0 ), Color(1,0,0), m_Font);	
//		gl::enableDepthRead();
	}
}

boost::shared_array<unsigned char> MultipleKinectApp::getImageData( int deviceID, _2RealGenerator imageType, int& imageWidth, int& imageHeight, int& bytePerPixel )
{
	bytePerPixel = m_2RealKinect->getBytesPerPixel( imageType );
	imageWidth = m_2RealKinect->getImageWidth( deviceID, imageType );		
	imageHeight = m_2RealKinect->getImageHeight( deviceID, imageType );
	return m_2RealKinect->getImageData( deviceID, imageType );
}

void MultipleKinectApp::drawSkeletons(int deviceID, ci::Rectf rect)
{
	float fRadius = 10.0;

	gl::pushMatrices();
	try
	{
		glLineWidth(2.0);

		glTranslatef( rect.getX1(), rect.getY1(), 0 );
		glScalef( rect.getWidth()/(float)m_2RealKinect->getImageWidth(deviceID, DEPTHIMAGE), rect.getHeight()/(float)m_2RealKinect->getImageHeight(deviceID, DEPTHIMAGE), 1);

		_2RealPositionsVector2f::iterator iter;

		for( unsigned int i = 0; i < m_2RealKinect->getNumberOfSkeletons( deviceID ); ++i)
		{
			glColor3f( 0, 1.0, 0.0 );
			_2RealPositionsVector3f skeletonPositions = m_2RealKinect->getSkeletonScreenPositions( deviceID, i );

			_2RealOrientationsMatrix3x3 skeletonOrientations;
			if(m_2RealKinect->hasFeatureJointOrientation())
				skeletonOrientations = m_2RealKinect->getSkeletonWorldOrientations( deviceID, i );

			int size = skeletonPositions.size();
			for( int j = 0; j < size; ++j )
			{	
				_2RealJointConfidence jointConfidence = m_2RealKinect->getSkeletonJointConfidence(deviceID, i, _2RealJointType(j));
				gl::pushModelView();
				if( m_2RealKinect->isJointAvailable( (_2RealJointType)j ) && jointConfidence.positionConfidence > 0.0)
				{
					glTranslatef(Vec3f( skeletonPositions[j].x, skeletonPositions[j].y, 0 ));

					if(m_2RealKinect->hasFeatureJointOrientation() && jointConfidence.orientationConfidence > 0.0)
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
		glLineWidth(1.0);
	}
	catch(...)
	{
		
	}
	gl::popMatrices();
}

void MultipleKinectApp::drawCenterOfMasses(int deviceID, ci::Rectf destRect)
{
	int nrOfUsers = m_2RealKinect->getNumberOfUsers(deviceID);
	//std::cout << "The number of users is: " << nrOfUsers << std::endl;
	if ( nrOfUsers > 0 )
	{
		gl::color(1,0,0);
		for ( int i=0; i<nrOfUsers; i++ )
		{
			_2RealKinectWrapper::_2RealVector3f center = m_2RealKinect->getUsersScreenCenterOfMass(deviceID, i);	
			center.x = (center.x / (float)m_iKinectWidth) * (destRect.x2 - destRect.x1) + destRect.x1;
			center.y = (center.y / (float)m_iKinectHeight) * (destRect.y2 - destRect.y1) + destRect.y1;
			gl::disableDepthRead();
			gl::drawStrokedRect(Rectf(center.x, center.y, center.x + 5, center.y + 5));
			gl::enableDepthRead();
		}
		gl::color(1,1,1);
	}
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
	int iImageHeight = (int)(m_iScreenHeight);		// divide window height according to the number of generator outputs (rgb, depth, user, skeleton)
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
	{
		mirrorImages();
	}
	if( event.getChar() == 'r' )	// restart kinects
	{
		bool bResult = m_2RealKinect->restart();
		if( bResult )
			std::cout << "\n\n_2Real started successfully!..." << std::endl;
		m_iNumberOfDevices = m_2RealKinect->getNumberOfDevices();
	}

	if( event.getChar() == 'u' )	// reset all calibrated users (OpenNI only)
	{
		m_2RealKinect->resetAllSkeletons();
	}

	if ( event.getChar() == 'w' )
	{
		m_Align = !m_Align;
		if ( !m_2RealKinect->depthIsAlignedToColor( 0 ) )
		try
		{
			m_2RealKinect->alignDepthToColor( 0, m_Align );

		}
		catch ( ... )
		{
			std::cout << "Something" << std::endl;
		}
	}
	if ( event.getChar() == 'l' )
	{
		m_2RealKinect->stopGenerator( 0,  DEPTHIMAGE | COLORIMAGE | USERIMAGE );
	}
	if ( event.getChar() == 'o' )
	{
		m_2RealKinect->startGenerator( 0,  DEPTHIMAGE | COLORIMAGE | USERIMAGE );
	}

	if( event.getCode() == ci::app::KeyEvent::KEY_UP )	
	{
		m_iMotorValue+=3;
		m_2RealKinect->setMotorAngle(0, m_iMotorValue);
//		if(m_2RealKinect->setMotorAngle(0, m_iMotorValue))
		//	m_iMotorValue = m_2RealKinect->getMotorAngle(0);
		//else
		//	m_iMotorValue-=3;
	}
	if( event.getCode() == ci::app::KeyEvent::KEY_DOWN )	
	{
		m_iMotorValue-=3;
		m_2RealKinect->setMotorAngle(0, m_iMotorValue);
		//if(m_2RealKinect->setMotorAngle(0, m_iMotorValue))
		//	m_iMotorValue = m_2RealKinect->getMotorAngle(0);
		//else
		//	m_iMotorValue+=3;
	}
}

void MultipleKinectApp::mirrorImages()
{
	//m_2RealKinect->setResolution( 0, COLORIMAGE, 320,241);


	//if ( !m_bIsMirroring )
	//{
	//	std::cout << "Switch to 1280X1024" << std::endl;
	//	try
	//	{
	//		m_2RealKinect->stopGenerator( 0, COLORIMAGE );
	//		if ( m_2RealKinect->generatorIsActive( 0, COLORIMAGE ))
	//		{
	//			std::cout << "Its active" <<std::endl;
	//		} else 
	//		{
	//			std::cout << "Its not active" <<std::endl;
	//		}
	//		//m_2RealKinect->removeGenerator( 0, COLORIMAGE );
	//		//m_2RealKinect->addGenerator( 0, COLORIMAGE, IMAGE_COLOR_320X240 );
	//		//m_2RealKinect->startGenerator( 0, COLORIMAGE );
	//		if ( m_2RealKinect->generatorIsActive( 0, COLORIMAGE ))
	//		{
	//			std::cout << "Its active" <<std::endl;
	//		} else 
	//		{
	//			std::cout << "Its not active" <<std::endl;
	//		}

	//	//	m_2RealKinect->setResolution( 0, COLORIMAGE, 1290, 1024 );
	//	}
	//	catch (...)
	//	{
	//		std::cout << "Couldn't switch to 1280X1024" << std::endl;
	//	}
	//}
	//else
	//{
	//	std::cout << "Switch to 640X480" << std::endl;
	//	try
	//	{
	//		//m_2RealKinect->stopGenerator( 0, COLORIMAGE );
	//		//m_2RealKinect->removeGenerator( 0, COLORIMAGE );
	//		//m_2RealKinect->addGenerator( 0, COLORIMAGE, IMAGE_COLOR_1280X1024 );
	//		//m_2RealKinect->startGenerator( 0, COLORIMAGE );
	//	//	m_2RealKinectb->setResolution( 0, COLORIMAGE, 640, 480 );
	//		
	//	}
	//	catch (...)
	//	{
	//		std::cout << "Couldn't switch to 320X240" << std::endl;
	//	}
	//}
	

	for ( int i = 0; i < m_iNumberOfDevices; ++i )
	{
		
//		m_2RealKinect->setMirrored( i, DEPTHIMAGE, m_bIsMirroring );
//		m_2RealKinect->setMirrored( i, USERIMAGE, m_bIsMirroring );		// OpenNI has no capability yet to mirror the user image

		try
		{
			m_2RealKinect->setMirrored( i, COLORIMAGE, m_bIsMirroring );
		}
		catch (...)
		{
			std::cout << "Failed to mirror" << std::endl;
		}
	}
	m_bIsMirroring = !m_bIsMirroring;	// toggleMirroring
}

CINDER_APP_BASIC( MultipleKinectApp, RendererGl )