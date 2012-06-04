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

#include "GlSingleKinectApp.h"
#include <iostream>

using namespace _2RealKinectWrapper;

GlSingleKinectApp::GlSingleKinectApp(void)
	: m_2Real( _2RealKinect::getInstance() ),
	m_IsInitialized( 0 )
{
	try
	{
		if( m_IsInitialized = m_2Real->start( COLORIMAGE | DEPTHIMAGE ) )
			std::cout << std::endl << "_2Real: Initialization successful!" << std::endl;
		else
			std::cout << std::endl << "_2Real: Initialization unsuccessful!" << std::endl;		
		LoadTextures();
	}
	catch( std::exception& e )
	{
		std::cout << e.what() << std::endl;
	}
}


GlSingleKinectApp::~GlSingleKinectApp(void)
{
	m_2Real->shutdown();
}

void GlSingleKinectApp::UpdateTextures()
{
	if( m_IsInitialized )
	{
		glBindTexture( GL_TEXTURE_2D, m_Texture[0] );
			glTexImage2D(	GL_TEXTURE_2D,
							0,
							3,
							m_2Real->getImageWidth( 0, COLORIMAGE ),
							m_2Real->getImageHeight( 0, COLORIMAGE ),
							0,
							GL_RGB,
							GL_UNSIGNED_BYTE,
							m_2Real->getImageData( 0, COLORIMAGE, 0 ).get() );

		glBindTexture( GL_TEXTURE_2D, m_Texture[1] );
			glTexImage2D(	GL_TEXTURE_2D,
				0,
				1,
				m_2Real->getImageWidth( 0, DEPTHIMAGE ),
				m_2Real->getImageHeight( 0, DEPTHIMAGE ),
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				m_2Real->getImageData( 0, DEPTHIMAGE, 0 ).get() );
	}
}

void GlSingleKinectApp::Draw()
{
	UpdateTextures();

	glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
	glScalef( 1, -1, 1 );
	glTranslatef(-2.0f,-2.0f,-5.0f);
	glBegin(GL_QUADS);
		// Front Face
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 2.0f, 0.0f,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.0f,  2.0f,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f,  2.0f,  0.0f);
	glEnd();
	
	glBindTexture( GL_TEXTURE_2D, m_Texture[1] );
	glTranslatef(2.0f,0.0f,0.0f);
	glBegin(GL_QUADS);
	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f,  0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 2.0f, 0.0f,  0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.0f,  2.0f,  0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f,  2.0f,  0.0f);
	glEnd();
}

bool GlSingleKinectApp::LoadTextures()
{
	if( m_IsInitialized )
	{
		glGenTextures( 1, &m_Texture[0] );					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture( GL_TEXTURE_2D, m_Texture[0] );
		glTexImage2D(	GL_TEXTURE_2D,
						0,
						3,
						m_2Real->getImageWidth( 0, COLORIMAGE ),
						m_2Real->getImageHeight( 0, COLORIMAGE ),
						0,
						GL_RGB,
						GL_UNSIGNED_BYTE,
						m_2Real->getImageData( 0, COLORIMAGE, 0 ).get() );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glGenTextures( 1, &m_Texture[1] );					// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture( GL_TEXTURE_2D, m_Texture[1] );
		glTexImage2D(	GL_TEXTURE_2D,
			0,
			1,
			m_2Real->getImageWidth( 0, DEPTHIMAGE ),
			m_2Real->getImageHeight( 0, DEPTHIMAGE ),
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			m_2Real->getImageData( 0, DEPTHIMAGE, 0 ).get() );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	return true;
}
