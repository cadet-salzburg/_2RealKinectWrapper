#pragma once

#include "ofMain.h"
//uncomment the following line to use Microsoft Kinect SDK instead
//#define TARGET_MSKINECTSDK
#include "_2RealKinect.h"

using namespace _2RealKinectWrapper; 

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
		boost::shared_array< unsigned char > getImageData( int deviceID, _2RealGenerator imageType, int& imageWidth, int& imageHeight, int& bytePerPixel );
		void			drawKinectImages();
		void			drawSkeletons(int deviceID, ofRectangle rect);
		void			resizeImages();
		void			mirrorImages();

		_2RealKinectWrapper::_2RealKinect*		m_2RealKinect;
		bool						m_bIsMirroring;
		int							m_iKinectWidth;
		int							m_iKinectHeight;
		int							m_iScreenWidth;
		int							m_iScreenHeight;
		int							m_iNumberOfDevices;
		ofPoint						m_ImageSize;
};
