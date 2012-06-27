License
-------

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
		funded by the Austrian Research Promotion Agency (http://www.ffg.at)

	Authors: Robert Praxmarer, Nikolas Psaroudakis, Gerlinde Emsenhuber, Robert Sommeregger, Andreas Stallinger
	Email: support@cadet.at
	Created: 08-09-2011

   
Version
-------

     0.9.4α ( this branch is experimental and currently in alpha ). 
     It includes *Breaking* changes to the interface and works on OS X as well.
   
   Bug reports, feature request or inquiries to:
	support@cadet.at
	
   General info about CADET
     info@cadet.at
    
   Website:
     http://www.cadet.at
	 

 
Version History
---------------
### 0.9.4α Update release

* A major update on the OpenNI version is underway. This branch is experimental and is not production ready.

### 0.9.3    Update release

* Updated to newest MS kinect SDK 1.0 (microsoft changed their api quite a bit, it is tested so far with single kinect and all works fine)
* Updated to newest boost library 1.48.0 (be aware of that when linking against our wrapper, the necessary libs and header are to be found in the external libs folders)
* Still todo, add new highres mode to wrapper 1280x960

### 0.9.2 Update release

* ATTENTION MS SDK Users (the new installer of the MS Kinect SDK sets the environment variable wrong with a double backslash in the folder structure and this renders it unuseable for MS Visual Studio)
* Changed OpenNI to work with AutoCalibration so no need for the awkward calibration pose anymore.

### 0.9.1 Update release

* Small name changes in the interface class
* Added joint orientation methods (just works for OpenNI), use hasFeatureJointOrientation() to check
* Updated to the newest unstable versions of OpenNI (1.4.0.2), NITE (1.5.0.2), Avin2 Driver (5.0.5.1)
* Updated to the newest MS Kinect SDK lib (1.0beta2) 

Info
----
_2RealKinectWrapper is an API built as a static library which simplifies the usage of multiple Kinect sensors (PrimeSense, Microsoft) for C++ programmers. 

It supports both major SDKs (OpenNI and Microsoft's Kinect SDK) with one easy to use programming interface. It is multithreaded for maximum performance.

Simple examples for libCinder, OpenFrameworks and plain OpenGL accompanie the release to demonstrate it's capabilities and usage. 
The programming interface shouldn't change in the future in terms of breaking your code, new functions nevertheless might be added.
We tried to test the software as good as possible. However if you find bugs or issues we are happy if you send us a report or a solution to support@cadet.at, or use github ...
You can acquire different images (Depth, RGB, IR,...) of different Kinects at the same time. 
However skeleton and user detection on multiple devices is not available for Microsoft's SDK.
OpenNI in principal supports this feature at least for 2 devices, our solution has still a flaw as OpenNI is handling multiple user nodes really strange, but we are working on fixing it as soon as possible ...

Watch the demo video here: http://www.vimeo.com/29949750

If you find that software helpful for your projects, comment on the vimeo and like it, as we are funded by a research grant we have to proove we are doing something useful ;-)
	
Supported Platforms
-------------------

* Win7 (OpenNI, Microsoft Kinect)
* MacOS (just OpenNI
* Linux support is planned

Prerequisites
--------------

### Install device driver

* The Kinect comes with it's own microsoft driver which is installed automatically.
* OpenNI installs the PrimeSense driver as default. 

If you want to use a Kinect with OpenNI be sure to install the patched Kinect driver from:
			
https://github.com/avin2/SensorKinect  ( the patch comes in the form of an executable that can be found in the bin folder )

If you want to use the OpenNI SDK you need to:

1. Install the newest unstable OpenNI lib  ( http://openni.org/ )
2. Install the newest unstable NITE SDK ( http://openni.org/ )

If you want to use the Microsoft Kinect SDK you need to:

1. install the newest MS Kinect SDK  ( http://www.microsoft.com/en-us/kinectforwindows/develop/ )
		
If you want to use both Microsoft and OpenNI SDKs on your system (Win7), you need to install all of the above libs and switch the driver accordingly in the windows device manager.

Build
-----

Add environment variables:

You have to set the following environment variables in your system 
( http://www.windows7hacker.com/index.php/2010/05/how-to-addedit-environment-variables-in-windows-7/ ) :
No need anymore with the new MS Kinect SDK to set a manual environment variable. 
The installer automaically sets KINECTSDK10_DIR as env var (be aware that before it was KINECTSDK_DIR)
			
If you want to build the cinder sample add:

* CINDER_DIR		your drive\your base path where you installed cinder (e.g. c:\sdk\cinder)

If you want to build the openframeworks sample add:

* OPENFRAMEWORKS_DIR your drive\your base path where you installed cinder (e.g. c:\sdk\of_preRelease_v007_vs2010)


To build _2RealKinect go to _2RealKinectWrapper/build/vc10
	
In Visual Studio go to menu build/batch build ...
Check all targets and press rebuild ... 

	Note: if you just installed OpenNI or MSKinect uncheck the targets you don't need 

Now you find all necessary libs in the _2RealKinectWrapper/lib folder 
(debug, release for MSKinect and OpenNI using dynamic standard library or static)

Sample
------
	
There are 3 samples accompanying the wrapper (Openframeworks, Cinder, Glut)

The glut sample is very simple and should work out of the box, it just shows the use of one kinect and just outputs rgb and depth image.
For the others you must have installed Cinder and/or openframeworks and set the environment variables as stated above.
The samples show the output (rgb, depth, user, skeleton) for all connected and by the windows detected devices

Switching the used Kinect SDK in the samples or your own project
----------------------------------------------------------------
	
Default is OpenNI (you don't have to change anything for that configuration)

When you want to use MS Kinect SDK just add a define before the include like this and rebuild (or define the preprocessor define in your visual studio settings):

	#define TARGET_MSKINECTSDK
	#include "_2RealKinect.h"

If you include "_2RealKinect.h" more often in your project be sure to always add the target before or just put the target define at the top of the "_2RealKinect.h" file itself


How to switch driver in the device manager of Win7:

	Open device manager.
	Click on Kinect in the device list, right click on Kinect Camera, choose update driver, choose search on computer / choose manually, choose "choose from a list", now you should see 2 devices of which you can choose.
	Choose the one that is not active and press enter. You can distinguish the drivers as the Microsoft driver is signed has a icon showing this...
	
Dependencies
------------

	OpenNI (not bundled with the library, please download and install)
	MS Kinect SDK (not bundled with the library, please download and install)
	Subset of Boost library (it's delivered with this lib in the externalLibs folder)

Known issues
------------

### Both SDKs

* Multiple Kinects only work if they are connected to separated USB busses (note on laptops all the usb connections are often on a single bus)
* You can't use RGB and IR image at the same time this is a limitation by both SDKs
* Right now all found devices will start with the nodes you entered in the start routine, if there is a use we will make an overloaded start for starting different nodes on different devices...
		
### OpenNI

* Hand and feet have orientation confidence of value 0.0 always (so OpenNI doesn't supply you with orientations for hand and feet) 
* Multi device user segmentation and skeletonizations are not working correctly yet (it mixes up the different devices, openni is strange)
* OpenNI with Kinect sensor only supports 640x480 resolution, Primesense sensor supports different resolutions
		
### Microsoft SDK
		
* MS SDK just supports user and skeleton tracking for a single device, depth and rgb go for multiple devices
* Color IDs of users can be different at start up (problem seems to be in the Microsoft Kinect SDK, which doesn't seem to do the ids right)
* Mirroring seems to be somehow different at startup (sometimes) 
* No support for IR images yet
* If you switched the driver and used the openni sdk before and don't unplug and replug the kinect the depth image is mirrored the wrong way in the MS Kinect SDK

Todo
-----

* Change implementation to shared_ptr
* Integrate Hand detector of OpenNI
* Linux (codeblocks)
* Tutorial
* High Level Recording and Tracking Library 
* Multi Device User Tracking
  
Additional notes on the implementation
--------------------------------------
	
	In our API we make heavy use of the PIMPL idiom to encapsulate the necessary SDK specific, implementation details that shouldn't bother the user. 
	The API should be easily extensible to other depth camera SDKs, e.g Panasonic Imager etc
	
Please help us improve this wrapper, spread the word and maybe write a nice comment on vimeo, thx!!!