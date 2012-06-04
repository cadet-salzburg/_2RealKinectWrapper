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


#include "_2RealTypes.h"

namespace _2RealKinectWrapper
{

class I_2RealImplementation;


//singleton implementation of _2Real-Wrapper for Windows-Kinect-SDK && OpenNI
class _2RealKinect
{
	public:

		/*! /brief     Returns the pointer to the singleton _2real-implementation and offers an opportunity to use main functions of kinect-device
			/return    _2RealKinect* Pointer to _2Real-Object
		!*/
		static _2RealKinect*				getInstance();

		/*! /brief     Returns the version of the _2RealKinectWrapper as string formatted major.minor.patch
			/return    std::string version number
		!*/
		std::string							getVersion();

		/*! /brief     Returns if the version of the _2RealKinectWrapper is at least the revision the user asked
			/return    bool (true if the version of the api is higher or equal to the version the user asked for)
		!*/
		bool								isAtLeastVersion(int major, int minor, int patch);

		/*! /brief     Doing all the important startup stuff like initializing context, enumeration of devices and initialization + starting of generators
					   Notice for configuration on OpenNI: No down sampling of images! Supports only 640x480 images!
			/param     uint32_t startGenerators Use _2RealGenerator enum to configure this param, default value starts GENERATOR_IMAGE | GENERATOR_DEPTH
			/param     uint32_t configureImages Use this to configure image capabilities, use _2realImages enum
			/return    bool Returns the operations success
		!*/
		void								update();
		bool								configure( const uint32_t deviceID,  uint32_t startGenerators, uint32_t configureImages );
		void								startGenerator( const uint32_t deviceID, uint32_t configureGenerators );




		bool								start( uint32_t startGenerators = CONFIG_DEFAULT, uint32_t configureImages = IMAGE_CONFIG_DEFAULT );

		/*! /brief     Shuts down all generators
			/return    bool Returns the operations success
		!*/
		bool								shutdown();

		/*! /brief	   Checks if new data is available to get from the wrapper, so you don't need to execute unnecessary calculations and data copying
			/param     const uint32_t deviceID for choosing specific device
			/param     _2RealGenerator type indicating the type of the generator to be requested, Use _2RealGenerator enum
			/return    bool returns if new data available
		!*/
		const bool							isNewData(const uint32_t deviceID, _2RealGenerator type) const;

		/*! /brief     Returns the bytes used (number of channels ) per pixel for a particular generator image
			/param     _2RealGenerator type indicating the type of the generator to be requested, Use _2RealGenerator enum
			/param     const uint8_t userId only used for GENERATOR_USERIMAGE. Ignored by other types
			/return    std::uint32_t bytes (number of channels) used per pixel
		!*/
		uint32_t							getBytesPerPixel( _2RealGenerator type ) const;


		/*! /brief     Returns the width of an particular image
			/param     const uint32_t deviceID In param for choosing specific device
			/param     _2RealGenerator type indicating the type of the generator to be requested, Use _2RealGenerator enum
			/param     const uint8_t userId only used for GENERATOR_USERIMAGE. Ignored by other types
			/return    std::uint32_t Retunrs width of the requested image
		!*/
		uint32_t							getImageWidth( const uint32_t deviceID, _2RealGenerator type );


		/*! /brief
			/param     const uint32_t deviceID for choosing specific device
			/param     _2RealGenerator type indicating the type of the generator to be requested, Use _2RealGenerator enum
			/param     const uint8_t userId only used for GENERATOR_USERIMAGE. Ignored by other types
			/return    std::uint32_t Returns heigth of the requested image
		!*/
		uint32_t							getImageHeight( const uint32_t deviceID, _2RealGenerator type );

		/*! /brief     Returns a 8bit pointer to a image buffer of a particular image
			/param     const uint32_t deviceID for choosing specific device
			/param     _2RealGenerator type indicating the type of the generator to be requested, Use _2RealGenerator enum
			/param     bool waitAndBlock indicating if waiting for changes
			/param     const uint8_t userId only used for GENERATOR_USERIMAGE. Ignored by other types
			/return    boost::shared_ptr<unsigned char> Shared Pointer to image buffer; use getImageWidth, getImageHeight, getImageBytePerPixel to obtain image specifications
		!*/
		boost::shared_array<unsigned char>	getImageData( const uint32_t deviceID, _2RealGenerator type, bool waitAndBlock=false, const uint8_t userId=0 );

		/*! /brief     Returns a 16bit pointer to depth image buffer (16 high precision depth values from depth sensor
			/param     const uint32_t deviceID for choosing specific device
			/param     bool waitAndBlock indicating if waiting for changes
			/return    boost::shared_ptr<uint16_t*> shared pointer to 16bit image buffer; use getImageWidth, getImageHeight, getImageBytePerPixel to obtain image specifications
		!*/
		boost::shared_array<uint16_t>			getImageDataDepth16Bit( const uint32_t deviceID, bool waitAndBlock=false);

		/*! /brief     Returns the number of detected sensors
			/return    std::uint32_t Number of detected sensors
		!*/
		uint32_t							getNumberOfDevices() const;

		/*! /brief		Returns position of specific joint of specific skeleton in wold coordinates
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/param		_2RealJointType indicating the type of the joint to be requested, Use _2RealJointType enum
			/return		const _2RealVector3f
		!*/
		const _2RealVector3f				getSkeletonJointWorldPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type );

		/*! /brief		Returns all joints of the skeleton of a specific user in world coordinates
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/return		const _2RealPositionsVector3f
		!*/
		const _2RealPositionsVector3f		getSkeletonWorldPositions( const uint32_t deviceID, const uint8_t userID );

		/*! /brief		Returns position of specific joint of specific skeleton in screen coordinates
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/param		_2RealJointType type indicating the type of the joint to be requested, Use _2RealJointType enum
			/return		const _2RealVector2f
		!*/
		const _2RealVector3f getSkeletonJointScreenPosition( const uint32_t deviceID, const uint8_t userID, _2RealJointType type );

		/*! /brief		Returns all joints of the skeleton for a specific tracked user in screen coordinates
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/return		const _2RealPositionsVector2f
		!*/
		const _2RealPositionsVector3f getSkeletonScreenPositions( const uint32_t deviceID, const uint8_t userID );

		/*! /brief		Returns orientation of specific joint of specific skeleton
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/param		_2RealJointType type indicating the type of the joint to be requested, Use _2RealJointType enum
			/return		const _2RealMatrix3x3
		!*/
		const _2RealMatrix3x3				getSkeletonJointWorldOrientation( const uint32_t deviceID, const uint8_t userID, _2RealJointType type );

		/*! /brief		Returns all joint orientations of the skeleton
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/return		const _2RealOrientationsMatrix3x3
		!*/
		const _2RealOrientationsMatrix3x3	getSkeletonWorldOrientations( const uint32_t deviceID, const uint8_t userID );

		/*! /brief		Returns position and orientation confidence for a joint on a device for a specific userID
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/return		const _2RealJointConfidence
		!*/
		const _2RealJointConfidence			getSkeletonJointConfidence(const uint32_t deviceID, const uint8_t userID, _2RealJointType type);

		/*! /brief		Returns position and orientation confidence for a joint on a device for a specific userID
			/param		const uint32_t deviceID for choosing specific device
			/param		const uint8_t userID
			/return		const _2RealJointConfidences
		!*/
		const _2RealJointConfidences		getSkeletonJointConfidences(const uint32_t deviceID, const uint8_t userID);

		/*! /brief		Returns number of tracked skeletons
			/param		const uint32_t deviceID for choosing specific device
			/return		const uint32_t
		!*/
		const uint32_t						getNumberOfSkeletons( const uint32_t deviceID ) const;

		/*! /brief		Returns number of tracked (segmented from background) users
			/param		const uint32_t deviceID for choosing specific device
			/return		const uint32_t
		!*/
		const uint32_t						getNumberOfUsers( const uint32_t deviceID ) const;

		/*! /brief		Returns center of mass of a specific tracked users in world coordinates
			/param		const uint32_t deviceID for choosing specific device
			/param		uint8_t userID
			/return		const _2RealVector3f 3d vector indicating the center of mass
		!*/
		const _2RealVector3f				getUsersWorldCenterOfMass(const uint32_t deviceID, const uint8_t userID);

		/*! /brief		Returns center of mass of a specific tracked users in screen coordinates
			/param		const uint32_t deviceID for choosing specific device
			/param		uint8_t userID
			/return		const _2RealVector3f 3d vector indicating the center of mass
		!*/
		const _2RealVector3f				getUsersScreenCenterOfMass(const uint32_t deviceID, const uint8_t userID);

		/*! /brief     Indicating if a particular generator has enabled the mirror capability
			/param     const uint32_t deviceID for choosing specific device
			/param     _2RealGenerator type indicating the type of the generator to be requested, Use _2RealGenerator enum
			/return    bool Indicating if mirroring is enabled
		!*/
		bool								isMirrored( const uint32_t deviceID, _2RealGenerator type ) const;

		/*! /brief     Setting mirror capability for a specific device and generator
			/param     const uint32_t deviceID for choosing specific device
			/param     _2RealGenerator type indicating the type of the generator to be requested, Use _2RealGenerator enum
			/param     bool flag indicating if turn capability on or off
			/return    void
		!*/
		void								setMirrored( const uint32_t deviceID, _2RealGenerator type, bool flag );

		/*! /brief
		 /param     _2RealJointType type indicating the type of the joint to be requested, Use _2RealJointType enum
		 /return    bool
		!*/
		virtual bool						isJointAvailable( _2RealJointType type ) const;

		/*! /brief  returns if the used SDK (OpenNI, MS SDK) has the joint orientation feature
		 /param     _2RealJointType type indicating the type of the joint to be requested, Use _2RealJointType enum
		 /return    bool
		!*/
		virtual bool						hasFeatureJointOrientation() const;
		
		/*! /brief     Resetting a particular calibrated user (skeleton) of a particular device (OpenNI only) (this is needed as OpenNI takes up to 15 seconds to delete a lost skeleton)
			/param     const uint32_t deviceID for choosing specific device
			/param     const uint32_t id id of the user to be resetted
			/return    void
		!*/
		void								resetSkeleton( const uint32_t deviceID, const uint32_t id );


		/*! /brief     Resetting all calibrated users (skeleton) of all sensors, OpenNI only (this is needed as OpenNI takes up to 15 seconds to delete a lost skeleton)
			/return    void
		!*/
		void								resetAllSkeletons();

		/*! /brief     Sets alignment of the sensors
			/param     const uint32_t deviceID for choosing specific device
			/param     bool flag indicating if turn capability on or off
			/return    void
		!*/
		void								setAlignColorDepthImage( const uint32_t deviceID, bool flag );

		/*! /brief     Shuts the system down and restarts it with flags provided in start()
			/return    bool
		!*/
		bool								restart();

		void								convertProjectiveToWorld( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inProjective, _2RealVector3f* outWorld );
		void								convertWorldToProjective( const uint32_t deviceID, const uint32_t coordinateCount, const _2RealVector3f* inWorld, _2RealVector3f* outProjective );


		/*! /brief     Sets elevation of camera via built in motor
			/param	   device number
			/param     angle between -27 and 27 for kinect device, 0 is the middle and normal position
			/return    could angle value be set ?
		!*/
		bool								setMotorAngle(int deviceID, int angle);

		/*! /brief     Sets elevation of camera via built in motor
			/param	   device number
			/return    angle between -27 and 27 for kinect device, 0 is the middle and normal position
		!*/
		int									getMotorAngle(int deviceID);

		/*! /brief     Sets level of verbosity of the integrated logger, ranging from fully verbose (iLevel = debug) to nonverbose (iLevel = none)
			/param     _2RealLogLevel level enum for loglevel to be set
			/return    void
		!*/
		void								setLogLevel(_2RealLogLevel level);

		/*! /brief     Sets custom output stream for logging (default is cout), you can log into filestreams, cerr, etc.
			/param     std::ostream* outStream address of output stream
			/return    void
		!*/
		void								setLogOutputStream(std::ostream* outStream);

	private:

		_2RealKinect();
		~_2RealKinect();
		_2RealKinect( const _2RealKinect& o );
		_2RealKinect& operator=( const _2RealKinect& o );

		boost::shared_ptr<I_2RealImplementation>		m_Implementation;
		static _2RealKinect*							m_Instance;
};

}	// namespace _2RealKinectWrapper


// This is used for conditional linking of libraries, you as a user don't have to set the libs in your project setting, however you have to set the paths in your IDEs settings
#ifndef UNIX
    #ifndef TARGET_MSKINECTSDK
        #pragma comment( lib, "openNI.lib" )
        #ifdef _DEBUG
            #ifdef _DLL
                #pragma comment( lib, "_2RealKinectOpenNI_mtd_d.lib")		// Debug multi threaded dll for OpenNI
            #elif _MT
                #pragma comment( lib, "_2RealKinectOpenNI_d.lib")		// Debug OpenNI
            #endif
        #else
            #ifdef _DLL
                #pragma comment( lib, "_2RealKinectOpenNI_mtd.lib")		// Release multi threaded dll for OpenNI
            #elif _MT
                #pragma comment( lib, "_2RealKinectOpenNI.lib")			// Release OpenNI
            #endif
        #endif
    #else
        #pragma comment( lib, "Kinect10.lib" )
        #ifdef _DEBUG
            #ifdef _DLL
                #pragma comment( lib, "_2RealKinectMicrosoftSDK_mtd_d.lib")		// Debug MS SDK
            #elif _MT
                #pragma comment( lib, "_2RealKinectMicrosoftSDK_d.lib")			// Debug MS SDK
            #endif
        #else
            #ifdef _DLL
                #pragma comment( lib, "_2RealKinectMicrosoftSDK_mtd.lib")		// Release MS SDK
            #elif _MT
                #pragma comment( lib, "_2RealKinectMicrosoftSDK.lib")			// Release MS SDK
            #endif
        #endif
    #endif
#endif
