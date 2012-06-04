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

#pragma once

// uncomment this to use Microsoft Kinect SDK instead
//#define TARGET_MSKINECTSDK

#include "_2RealKinect.h"
#include <windows.h>
#include <stdio.h>
#include <gl\gl.h>			
#include <gl\glu.h>	



class GlSingleKinectApp
{
	public:
		GlSingleKinectApp(void);
		~GlSingleKinectApp(void);

		void				Draw();
		bool				LoadTextures();
		void				UpdateTextures();

	private:
		_2RealKinectWrapper::_2RealKinect*			m_2Real;
		GLuint							m_Texture[2];
		bool							m_IsInitialized;
};

