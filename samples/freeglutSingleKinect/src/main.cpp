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

#include <stdio.h>			// Header File For Standard Input/Output

#ifndef __APPLE__
#include "GL/freeglut.h"
#else
#include "GLUT/glut.h"
#endif

#include "FreeGlutSingleKinectApp.h"

FreeGlutSingleKinectApp* _2RealApp = NULL;

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity() ;

	_2RealApp->Draw();
	glutSwapBuffers();
}

static void
key(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27 :
    case 'Q':
    case 'q': glutLeaveMainLoop () ;      break;

    default:
        break;
    }
}

// Load Bitmaps And Convert To Textures
int LoadGLTextures()									
{
	int Status=FALSE;								
	Status = _2RealApp->LoadTextures();
	return Status;									
}


int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitWindowSize(640,480);
    glutInitWindowPosition(40,40);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow("FreeGlut Single Kinect");
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
	glutIdleFunc(display);

	glClearColor(0,0,0,1);
	glEnable(GL_TEXTURE_2D);					

	_2RealApp = new FreeGlutSingleKinectApp();

	if (!LoadGLTextures())							
	{
		std::cout << "textures not loaded";								
	}

    glutMainLoop();

#ifdef _MSC_VER
    /* DUMP MEMORY LEAK INFORMATION */
    _CrtDumpMemoryLeaks () ;
#endif

    return EXIT_SUCCESS;
}
