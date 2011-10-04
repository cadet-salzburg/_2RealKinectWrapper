#pragma once

#include "ofMain.h"
#define TARGET_MSKINECTSDK
#include "_2RealKinect.h"

class ofMultipleKinectApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		~ofMultipleKinectApp();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	private:
		void			drawKinectImages();
		void			drawSkeletons(int deviceID);
		void			resizeImages();
		void			mirrorImages();
		
		_2Real::_2RealKinect*		m_2RealKinect;
		bool						m_bIsMirroring;
		int							m_iKinectWidth;
		int							m_iKinectHeight;
		int							m_iScreenWidth;
		int							m_iScreenHeight;
		int							m_iNumberOfDevices;
		ofPoint						m_ImageSize;
};
