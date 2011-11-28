#include "ofMultipleKinectApp.h"


using namespace _2Real;

ofMultipleKinectApp::~ofMultipleKinectApp()
{
	m_2RealKinect->shutdown();
}

//--------------------------------------------------------------
void ofMultipleKinectApp::setup(){

	ofSetWindowTitle("CADET| http://www.cadet.at | MultipleKinectSample");	

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
			std::cout << "\n\n_2Real started successfully!...";
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
}

//--------------------------------------------------------------
void ofMultipleKinectApp::update(){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::draw(){
	try
	{
		ofClear( 1.0f, 1.0f, 1.0f, 1.0f);		
		drawKinectImages();				//oututs all connected kinect devices generators (depth, rgb, user imgage, skeletons)			
	}
	catch( std::exception& e )
	{
		std::cout << "\nException in draw: " << e.what() << std::endl;
	}
}

void ofMultipleKinectApp::drawKinectImages()
{
	for( int i = 0; i < m_iNumberOfDevices; ++i)
	{
		//rgb image
		uint8_t* imgRef = (uint8_t*)m_2RealKinect->getImageData( i, COLORIMAGE );
		int numberChannels = m_2RealKinect->getBytesPerPixel( COLORIMAGE );
		int m_iKinectWidth = m_2RealKinect->getImageWidth( i, COLORIMAGE );		
		int m_iKinectHeight = m_2RealKinect->getImageHeight( i, COLORIMAGE );
		ofImage colorImage;
		colorImage.setFromPixels(imgRef, m_iKinectWidth, m_iKinectHeight, OF_IMAGE_COLOR, true);
		colorImage.draw(i * m_ImageSize.x, 0, m_ImageSize.x*(i+1) , m_ImageSize.y);

		//depth image
		numberChannels = m_2RealKinect->getBytesPerPixel( DEPTHIMAGE );
		m_iKinectWidth = m_2RealKinect->getImageWidth( i, DEPTHIMAGE );		
		m_iKinectHeight = m_2RealKinect->getImageHeight( i, DEPTHIMAGE );
		ofImage depthImage;
		depthImage.setFromPixels(m_2RealKinect->getImageData( i, DEPTHIMAGE), m_iKinectWidth, m_iKinectHeight, OF_IMAGE_GRAYSCALE, true);
		depthImage.draw(i * m_ImageSize.x, m_ImageSize.y, m_ImageSize.x*(i+1), m_ImageSize.y);

		//user image
#ifdef	TARGET_MSKINECTSDK
		if( i == 0 )						
#endif
		{
			m_iKinectWidth = m_2RealKinect->getImageWidth( i, USERIMAGE_COLORED );		
			m_iKinectHeight = m_2RealKinect->getImageHeight( i, USERIMAGE_COLORED );
			uint8_t* imgRef = (uint8_t*)m_2RealKinect->getImageData( i, USERIMAGE_COLORED, false, 0 );
			ofImage colorImage;
			colorImage.setFromPixels(imgRef, m_iKinectWidth, m_iKinectHeight, OF_IMAGE_COLOR, true);
			colorImage.draw(i * m_ImageSize.x,  m_ImageSize.y*2, m_ImageSize.x*(i+1) , m_ImageSize.y);
						
		}

		m_iKinectWidth = m_2RealKinect->getImageWidth( i, COLORIMAGE );		
		m_iKinectHeight = m_2RealKinect->getImageHeight( i, COLORIMAGE );
		//skeleton		
		drawSkeletons(i, ofRectangle( i * m_ImageSize.x, m_ImageSize.y *3 , m_ImageSize.x, m_ImageSize.y));

	}
}

void ofMultipleKinectApp::drawSkeletons(int deviceID, ofRectangle rect)
{
	float fRadius = 14.0;

	glPushMatrix();

	glTranslatef( rect.x, rect.y, 0 );
	glScalef( rect.width/(float)m_iKinectWidth, rect.height/(float)m_iKinectHeight, 1);

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
			glPushMatrix();
			if( m_2RealKinect->isJointAvailable( (_2RealJointType)j ) )
			{
				glTranslatef(skeletonPositions[j].x, skeletonPositions[j].y, 0 );

				if(m_2RealKinect->hasFeatureJointOrientation())
				{
					float modelview[16];
					glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

					modelview[0] = skeletonOrientations[j].elements[0];
					modelview[1] = skeletonOrientations[j].elements[3];
					modelview[2] = skeletonOrientations[j].elements[6];
					modelview[4] = skeletonOrientations[j].elements[1];
					modelview[5] = skeletonOrientations[j].elements[4];
					modelview[6] = skeletonOrientations[j].elements[7];
					modelview[8] = skeletonOrientations[j].elements[2];
					modelview[9] = skeletonOrientations[j].elements[5];
					modelview[10] = skeletonOrientations[j].elements[8];

					glLoadMatrixf(modelview);		
					ofDrawAxis(fRadius);
				}
				else
				{
					ofCircle(  0, 0, fRadius);
				}
			}
			
			glPopMatrix();
		}
	}	
	glPopMatrix();
	
	glColor3f( 1.0, 1.0, 1.0 );	// reset vertex color to white
}


void ofMultipleKinectApp::resizeImages()
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
	m_ImageSize = ofPoint( iImageWidth, iImageHeight );
}

void ofMultipleKinectApp::mirrorImages()
{
	m_bIsMirroring = !m_bIsMirroring;	// toggleMirroring
	for( int i = 0; i < m_iNumberOfDevices; ++i)
	{
		m_2RealKinect->setMirrored( i, COLORIMAGE, m_bIsMirroring );
		m_2RealKinect->setMirrored( i, DEPTHIMAGE, m_bIsMirroring );
		m_2RealKinect->setMirrored( i, USERIMAGE, m_bIsMirroring );		// OpenNI has no capability yet to mirror the user image
	}
}

//--------------------------------------------------------------
void ofMultipleKinectApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::windowResized(int w, int h){
	m_iScreenWidth = ofGetWindowWidth();
	m_iScreenHeight = ofGetWindowHeight();
	resizeImages();
}

//--------------------------------------------------------------
void ofMultipleKinectApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofMultipleKinectApp::dragEvent(ofDragInfo dragInfo){ 

}