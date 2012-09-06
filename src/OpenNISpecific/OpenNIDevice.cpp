#ifndef TARGET_MSKINECTSDK
#include "OpenNIDevice.h"
#include "_2RealUtility.h"
#include "_2RealTypes.h"
#include "OpenNiUtils.hpp"
#include "_2RealConfig.h"

namespace  _2RealKinectWrapper {
	
	XnBool g_bNeedPose = FALSE;
	XnChar g_strPose[20] = "";

	unsigned char Colors[][3] =
	{
		{0,255,255},
		{0,0,255},
		{0,255,0},
		{255,255,0},
		{255,0,0},
		{255,127,0},
		{127,255,0},
		{0,127,255},
		{127,0,255},
		{255,255,127},
		{255,255,255}
	};

	void XN_CALLBACK_TYPE newUserCb( xn::UserGenerator& generator, XnUserID nId, void* pCookie )
	{
		xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
		//// New user found
		if ( g_bNeedPose )
		{
			std::cout << "Need Pose: " << nId << std::endl;
			userGenerator->GetPoseDetectionCap().StartPoseDetection( "", nId );
		}
		else
		{
			_2REAL_LOG(info) << "_2Real: New user " << nId << ", requested calibration... "<< std::endl;
			if ( userGenerator )
			{
				userGenerator->GetSkeletonCap().RequestCalibration( nId, TRUE );
			}
		}
	}

	//-----------------------------------------//
	void XN_CALLBACK_TYPE lostUserCb(xn::UserGenerator& generator, XnUserID nId, void* pCookie )
	{
		std::cout << "Lost user with ID: " << nId << std::endl;
	}

	//-----------------------------------------//

	void XN_CALLBACK_TYPE userExitCb(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
	{
		XnStatus status;
		xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
		status = userGenerator->GetSkeletonCap().Reset(nID);
		_2REAL_LOG(info) << "_2Real: User " << nID << " exited - resetting skeleton data... status: " << xnGetStatusString(status) << std::endl;
	}
	//-----------------------------------------//

	void XN_CALLBACK_TYPE userReentryCb(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie)
	{
		XnStatus status;
		xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);	
		status = userGenerator->GetSkeletonCap().RequestCalibration(nID, TRUE);
		_2REAL_LOG(info) << "_2Real: User " << nID << " reentered, request calibration... status: " << xnGetStatusString(status) << std::endl;
	}
	//-----------------------------------------//

	// Callback: Detected a pose
	void XN_CALLBACK_TYPE poseDetectedCb( xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie )
	{
		xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);
		userGenerator->GetPoseDetectionCap().StopPoseDetection(nId);
		userGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
		std::cout << "Pose detected" << std::endl;
	}

	//-----------------------------------------//

	void XN_CALLBACK_TYPE userCalibrationStartedCb(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie )
	{
		std::cout << "Calibrartion started for ID: " << nId << std::endl;
	}

	//-----------------------------------------//
	void XN_CALLBACK_TYPE userCalibrationCompletedCb(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie )
	{
		xn::UserGenerator *userGenerator = static_cast<xn::UserGenerator*>(pCookie);

		if ( eStatus == XN_CALIBRATION_STATUS_OK )
		{
			// Calibration succeeded
			std::cout << "Calibration Completed. Start Tracking User: " << nId << std::endl;
			userGenerator->GetSkeletonCap().StartTracking( nId );
		} else {
			// Calibration failed
			std::cout << "Calibration failed for user: " << nId << std::endl;

			if( eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT )
			{
				std::cout << "Manual calibration abort occured" << std::endl;
				return;
			}
			if ( g_bNeedPose )
			{
				userGenerator->GetPoseDetectionCap().StartPoseDetection( g_strPose, nId);
			}
			else
			{
				userGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
			}
		}
	}
	//-----------------------------------------//

OpenNIDevice::OpenNIDevice()
	:m_Context( xn::Context() ),
	 m_DeviceInfo( NodeInfoRef() ),
	 m_DeviceName("Device"),
	 m_DepthImage_8bit( NULL ),
	 m_InfraredImage_8bit( NULL ),
	 m_UserImage_8bit( NULL ),
	 m_UserImageColor_8bit( NULL ),
	 m_MotorInitialized( false )
{

}

OpenNIDevice::OpenNIDevice( const xn::Context context,  NodeInfoRef deviceInfo, const std::string deviceName )
	:m_Context( context ),
	 m_DeviceInfo( deviceInfo ),
	 m_DeviceName( deviceName ),
	 m_DepthImage_8bit( NULL ),
	 m_InfraredImage_8bit( NULL ),
	 m_UserImage_8bit( NULL ),
	 m_UserImageColor_8bit( NULL ),
	 m_MotorInitialized( false )
{

}

void OpenNIDevice::addDeviceToContext()
{
	xn::Device m_Device;
	m_DeviceInfo->SetInstanceName( m_DeviceName.c_str() );
	
	//ToDo: There is a verified bug in the unstable version of OpenNi that prevents adding a user generator after a device.
	// for more info see:  http://groups.google.com/group/openni-dev/browse_thread/thread/96331acca1ed7ce3?pli=1
	//For this reason, we are not forcing the creation of a device node at start, but let it be handled by openni behind the scenes,
	// when a production node of the device is requested.

	checkError( m_Context.CreateProductionTree( *m_DeviceInfo, m_Device ), " Error when creating production tree for device" );
	m_Device.AddRef();
	if ( m_DeviceMotorController.Create() == XN_STATUS_OK )
	{
		m_MotorInitialized = true;
		m_DeviceMotorController.Move( 0 );
	}

	//Get Identification data
	
	if ( m_Device.IsCapabilitySupported(XN_CAPABILITY_DEVICE_IDENTIFICATION) )
	{
		//const XnUInt32 nStringBufferSize = 200;
		//XnChar strDeviceName[nStringBufferSize];
		//XnChar strSerialNumber[nStringBufferSize];
		//XnChar strCreationInfo[nStringBufferSize];
		//XnUInt32 nLength = nStringBufferSize;
		//m_Device.GetIdentificationCap().GetDeviceName(strDeviceName, nLength); 
		//m_Device.GetIdentificationCap().GetSerialNumber(strSerialNumber,nLength);
		//unsigned short vendor_id;
  //      unsigned short product_id;
  //      unsigned char bus;
  //      unsigned char address;
		//sscanf(m_DeviceInfo->GetCreationInfo (), "%hx/%hx@%hhu/%hhu", &vendor_id,&product_id, &bus, &address);

		//unsigned short vendor_id;
  //      unsigned short product_id;
  //      unsigned char bus;
  //      unsigned char address;
  //      sscanf(m_DeviceInfo->GetCreationInfo(), "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address);
		//printf("DeviceInfo vendor_id %i product_id %i bus %i address %i \n", vendor_id, product_id, bus, address );
		//std::cout << "DI: " << m_DeviceInfo->GetCreationInfo() << std::endl;
		////printf("image: usb bus %s address %s\n", bus, address); 


		const XnProductionNodeDescription& description = m_DeviceInfo->GetDescription();
		//printf("device: vendor %s name %s, instance %s\n", description.strVendor, description.strName, m_DeviceInfo->GetInstanceName());
		//unsigned short vendor_id;
		//unsigned short product_id;
		//unsigned char bus;
		//unsigned char address;
		//sscanf( m_DeviceInfo->GetCreationInfo(), "%hx/%hx@%hhu/%hhu", &vendor_id, &product_id, &bus, &address );
		//printf("vendor_id %i product_id %i bus %i address %i connection %s\n", vendor_id, product_id, bus, address );
	}
}

void OpenNIDevice::addGenerator( const XnPredefinedProductionNodeType &nodeType, uint32_t configureImages )
{
	if ( !hasGenerator( nodeType ) )
	{
		xn::NodeInfoList nodeList;
		xn::Query query;
		query.AddNeededNode( m_DeviceInfo->GetInstanceName() );
		checkError( m_Context.EnumerateProductionTrees( nodeType, &query, nodeList, NULL ), "Error when enumerating production trees" );
		if ( nodeList.IsEmpty() )
		{
			_2RealKinectWrapper::throwError("Requested NodeType is not supported by the device");
		}
		xn::NodeInfo node = *nodeList.Begin();
		//Give a name to the generator
		std::string nodeName =  xnNodeTypeToString( nodeType ) + "_" + m_DeviceName;
		node.SetInstanceName( nodeName.c_str() );

		if ( nodeType == XN_NODE_TYPE_IMAGE )
		{
			checkError( m_Context.CreateProductionTree( node, m_ImageGenerator ), "Error while creating production tree." );
			XnMapOutputMode mode = getRequestedOutputMode( nodeType, configureImages );
			//checkError( m_ImageGenerator.SetMapOutputMode( mode ), "_2Real: Error when setting outputmode \n" );
		} 
		else if ( nodeType == XN_NODE_TYPE_DEPTH )
		{
			checkError( m_Context.CreateProductionTree( node, m_DepthGenerator ), "Error while creating production tree." );
			XnMapOutputMode mode = getRequestedOutputMode( nodeType, configureImages );
			checkError( m_DepthGenerator.SetMapOutputMode( mode ), "_2Real: Error when setting outputmode \n" );
			if ( !m_DepthImage_8bit )
			{
				m_DepthImage_8bit = ImageDataRef( new unsigned char[mode.nXRes*mode.nYRes]);
			}
		} 
		else if ( nodeType == XN_NODE_TYPE_IR )
		{
			checkError( m_Context.CreateProductionTree( node, m_IrGenerator ), "Error while creating production tree." );
			XnMapOutputMode mode = getRequestedOutputMode( nodeType, configureImages );
			checkError( m_IrGenerator.SetMapOutputMode( mode ), "_2Real: Error when setting outputmode \n" );
			if ( !m_InfraredImage_8bit )
			{
				m_InfraredImage_8bit = ImageDataRef( new unsigned char[mode.nXRes*mode.nYRes]);
			}
		} 
		else if ( nodeType == XN_NODE_TYPE_USER )
		{
			checkError( m_Context.CreateProductionTree( node, m_UserGenerator ), "Error while creating production tree." );
			//Get mode from depth generator
			XnMapOutputMode mode = getRequestedOutputMode( XN_NODE_TYPE_DEPTH, configureImages );
			registerUserCallbacks();
			if ( !m_UserImage_8bit )
			{
				m_UserImage_8bit = ImageDataRef( new unsigned char[mode.nXRes*mode.nYRes]);
			}
			if ( !m_UserImageColor_8bit )
			{
				m_UserImageColor_8bit = ImageDataRef( new unsigned char[mode.nXRes*mode.nYRes*3]);
			}
		}

	} else
	{
		std::cout << "Generator for nodetype: " << xnNodeTypeToString(nodeType) << " already exists." << std::endl;
	}
}

bool OpenNIDevice::hasGenerator( const XnPredefinedProductionNodeType &nodeType ) const
{
	xn::Query query;
	query.AddNeededNode( m_DeviceInfo->GetInstanceName() );
	xn::NodeInfoList nodeList;
	checkError( m_Context.EnumerateExistingNodes( nodeList, nodeType ), "Error Encountered while enumerating existing nodes" );
	nodeList.FilterList( m_Context, query );
	bool result = true;
	if ( nodeList.IsEmpty() )
	{
		result = false;
	}
	return result;
}

xn::NodeInfo OpenNIDevice::getNodeInfo()
{
	return *m_DeviceInfo;
}

xn::NodeInfoList OpenNIDevice::getNodeInfoList( const XnPredefinedProductionNodeType &nodeType  )
{
	xn::NodeInfoList nodeList;
	xn::Query query;
	query.AddNeededNode( m_DeviceInfo->GetInstanceName() );
	checkError( m_Context.EnumerateProductionTrees( nodeType, &query, nodeList, NULL ), "Error when enumerating production trees" );
	if ( nodeList.IsEmpty() )
	{
		_2RealKinectWrapper::throwError("Requested NodeType is not supported by the device");
	}
	return nodeList;
}

void OpenNIDevice::startGenerator( const XnPredefinedProductionNodeType &nodeType )
{
	if ( !hasGenerator( nodeType ) )
		return;
	xn::Generator generator;
	getExistingProductionNode( nodeType, generator );
	if ( !generator.IsGenerating() )
	{
		generator.StartGenerating();
	}
}

void OpenNIDevice::stopGenerator( const XnPredefinedProductionNodeType &nodeType )
{
	if ( !hasGenerator( nodeType ) )
		return;
	xn::Generator generator;
	getExistingProductionNode( nodeType, generator );
	if ( generator.IsGenerating() )
	{
		generator.StopGenerating();
	}
}

void OpenNIDevice::removeGenerator( const XnPredefinedProductionNodeType &nodeType )
{
	if ( hasGenerator( nodeType ) )
	{
		if ( nodeType == XN_NODE_TYPE_IMAGE )
		{
			m_ImageGenerator.Release();
		} 
		else if ( nodeType == XN_NODE_TYPE_DEPTH )
		{
			m_DepthGenerator.Release();
		}
		else if ( nodeType == XN_NODE_TYPE_USER )
		{
			m_UserGenerator.Release();
		}
		else if ( nodeType == XN_NODE_TYPE_IR )
		{
			m_IrGenerator.Release();
		}
	}
}

bool OpenNIDevice::generatorIsActive( const XnPredefinedProductionNodeType &nodeType )
{
	if ( !hasGenerator(nodeType) )
		throwError("Requested generator has not been added");
	xn::Generator gen;
	getExistingProductionNode( nodeType, gen );
	return gen.IsGenerating() ? true : false;
}

void OpenNIDevice::setGeneratorResolution( const XnPredefinedProductionNodeType &nodeType, unsigned int hRes, unsigned int vRes )
{
	if ( nodeType != XN_NODE_TYPE_DEPTH && nodeType != XN_NODE_TYPE_USER && nodeType != XN_NODE_TYPE_IMAGE &&  nodeType != XN_NODE_TYPE_IR )
		return;
	xn::MapGenerator node;
	XnMapOutputMode mode = getClosestOutputMode( nodeType, hRes, vRes );
	getExistingProductionNode( nodeType, node );
	checkError( node.SetMapOutputMode( mode ), "_2Real: Error when setting outputmode \n" );
}

XnMapOutputMode	OpenNIDevice::getClosestOutputMode( const XnPredefinedProductionNodeType &nodeType, unsigned int hRes, unsigned int vRes )
{
	boost::uint32_t configureImages = 0;
	if ( nodeType == XN_NODE_TYPE_DEPTH )
	{
		if ( hRes == 640 || vRes == 480 )
		{
			configureImages = IMAGE_USER_DEPTH_640X480;
		} 
		else if ( hRes == 320 || vRes == 240 )
		{
			configureImages = IMAGE_USER_DEPTH_640X480;
		} 
		else if ( hRes == 80 || vRes == 60 )
		{
			configureImages = IMAGE_USER_DEPTH_640X480;
		} 
		else
		{
			configureImages = IMAGE_USER_DEPTH_640X480;
		}
	} else if ( nodeType == XN_NODE_TYPE_IMAGE )
	{
		if ( hRes == 1280 || vRes == 1024 )
		{
			configureImages = IMAGE_COLOR_1280X1024;
		} 
		else if ( hRes == 640 || vRes == 480 )
		{
			configureImages = IMAGE_COLOR_640X480;
		} 
		else if ( hRes == 320 || vRes == 240 )
		{
			configureImages = IMAGE_COLOR_320X240;
		} else 
		{
			configureImages = IMAGE_COLOR_640X480;
		}
	} else if ( nodeType == XN_NODE_TYPE_USER )
	{
		if ( hRes == 640 || vRes == 480 )
		{
			configureImages = IMAGE_USER_DEPTH_640X480;
		}
		else if ( hRes == 320 || vRes == 240 )
		{
			configureImages = IMAGE_USER_DEPTH_320X240;
		}
		else if ( hRes == 80 || vRes == 60  )
		{
			configureImages = IMAGE_USER_DEPTH_80X60;
		}
		else //default
		{
			configureImages = IMAGE_USER_DEPTH_640X480;
		}
	} else if ( nodeType == XN_NODE_TYPE_IR )
	{
		if (  hRes == 640 || vRes == 480 )
		{
			configureImages = IMAGE_INFRARED_640X480;
		}
		else if ( hRes == 320 || vRes == 240 )
		{
			configureImages = IMAGE_INFRARED_320X240;
		}
		else //default
		{
			configureImages = IMAGE_INFRARED_640X480;
		}
	}
	return getRequestedOutputMode( nodeType, configureImages );
}
boost::uint32_t		OpenNIDevice::getImageConfig2Real( const XnPredefinedProductionNodeType &nodeType, const XnMapOutputMode& mode  )
{
	boost::uint32_t imageConfig2Real;
	if ( nodeType == XN_NODE_TYPE_DEPTH )
	{
		//configuring image size
		if ( mode.nXRes == 640 && mode.nYRes == 480 )
		{
			imageConfig2Real = IMAGE_USER_DEPTH_640X480;
		}
		else if ( mode.nXRes == 320 && mode.nYRes == 240 )
		{
			imageConfig2Real = IMAGE_USER_DEPTH_320X240;
		}
		else if ( mode.nXRes == 80 && mode.nYRes == 60 )
		{
			imageConfig2Real = IMAGE_USER_DEPTH_80X60;
		}
		else //default
		{
			imageConfig2Real = IMAGE_USER_DEPTH_640X480;
		}
		std::cout << "Depth " << std::endl;
	} else if ( nodeType == XN_NODE_TYPE_IMAGE )
	{
		if( mode.nXRes == 1280 && mode.nYRes == 1024 )
		{
			imageConfig2Real = IMAGE_COLOR_1280X1024;
		}
		else if( mode.nXRes == 640 && mode.nYRes == 480 )
		{
			imageConfig2Real = IMAGE_COLOR_640X480;
		}
		else if( mode.nXRes == 320 && mode.nYRes == 240 )
		{
			imageConfig2Real = IMAGE_COLOR_320X240;
		}
		else //default
		{
			imageConfig2Real = IMAGE_COLOR_640X480;
		}
				std::cout << "Image " << std::endl;
	} else if ( nodeType == XN_NODE_TYPE_USER )
	{
	//configuring image size
		if ( mode.nXRes == 640 && mode.nYRes == 480 )
		{
			imageConfig2Real = IMAGE_USER_DEPTH_640X480;
		}
		else if ( mode.nXRes == 320 && mode.nYRes == 240 )
		{
			imageConfig2Real = IMAGE_USER_DEPTH_320X240;
		}
		else if ( mode.nXRes == 80 && mode.nYRes == 60 )
		{
			imageConfig2Real = IMAGE_USER_DEPTH_80X60;
		}
		else //default
		{
			imageConfig2Real = IMAGE_USER_DEPTH_640X480;
		}
				std::cout << "User " << std::endl;
	} else if ( nodeType == XN_NODE_TYPE_IR )
	{
		if ( mode.nXRes == 640 && mode.nYRes == 480 )
		{
			imageConfig2Real = IMAGE_INFRARED_640X480;
		}
		else if ( mode.nXRes == 320 && mode.nYRes == 240 )
		{
			imageConfig2Real = IMAGE_INFRARED_320X240;
		}
		else //default
		{
			imageConfig2Real = IMAGE_INFRARED_640X480;
		}
				std::cout << "IR " << std::endl;
	} else {
		throwError(" Requested node type does not support an output mode ");
	}
	return imageConfig2Real;
}

XnMapOutputMode OpenNIDevice::getRequestedOutputMode( const XnPredefinedProductionNodeType &nodeType, boost::uint32_t configureImages )
{
   XnMapOutputMode mode;
   mode.nFPS = 30;

   if ( nodeType == XN_NODE_TYPE_DEPTH )
   {
	   //configuring image size
	   if ( configureImages & IMAGE_USER_DEPTH_640X480 )
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }
	   else if ( configureImages & IMAGE_USER_DEPTH_320X240 )
	   {
		   mode.nXRes = 320;
		   mode.nYRes = 240;
	   }
	   else if ( configureImages & IMAGE_USER_DEPTH_80X60 )
	   {
		   mode.nXRes = 80;
		   mode.nYRes = 60;
	   }
	   else //default
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }

   } else if ( nodeType == XN_NODE_TYPE_IMAGE )
   {
	   if( configureImages & IMAGE_COLOR_1280X1024 )
	   {
		  /*
			XN_RES_QVGA
			XN_RES_VGA
			XN_RES_SXGA
			XN_RES_UXGA
			XnMapOutputMode Mode;
			pGenerator->GetMapOutputMode(Mode);
			Mode.nXRes = Resolution((XnResolution)XN_RES_UXGA).GetXResolution();
			Mode.nYRes = Resolution((XnResolution)XN_RES_UXGA).GetYResolution();
			XnStatus nRetVal = pGenerator->SetMapOutputMode(Mode);
		  */
		   mode.nXRes = 1280;
		   mode.nYRes = 1024;
	   }
	   else if( configureImages & IMAGE_COLOR_640X480 )
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }
	   else if( configureImages & IMAGE_COLOR_320X240 )
	   {
		   mode.nXRes = 320;
		   mode.nYRes = 240;
	   }
	   else //default
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }

   } else if ( nodeType == XN_NODE_TYPE_USER )
   {
	   if ( configureImages & IMAGE_USER_DEPTH_640X480 )
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }
	   else if ( configureImages & IMAGE_USER_DEPTH_320X240 )
	   {
		   mode.nXRes = 320;
		   mode.nYRes = 240;
	   }
	   else if ( configureImages & IMAGE_USER_DEPTH_80X60 )
	   {
		   mode.nXRes = 80;
		   mode.nYRes = 60;
	   }
	   else //default
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }

   } else if ( nodeType == XN_NODE_TYPE_IR )
   {
	   if ( configureImages & IMAGE_INFRARED_640X480 )
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }
	   else if ( configureImages & IMAGE_INFRARED_320X240 )
	   {
		   mode.nXRes = 320;
		   mode.nYRes = 240;
	   }
	   else //default
	   {
		   mode.nXRes = 640;
		   mode.nYRes = 480;
	   }
   } else {

	   throwError(" Requested node type does not support an output mode ");
   }

   return mode;
}

void OpenNIDevice::getExistingProductionNode( const XnPredefinedProductionNodeType &nodeType, xn::ProductionNode& productionNode ) const
{
	std::string nodeName =  xnNodeTypeToString( nodeType ) + "_" + m_DeviceName;
	checkError( m_Context.GetProductionNodeByName(nodeName.c_str(), productionNode), " Requested production node has not been created" );
}

bool OpenNIDevice::hasNewData( const XnPredefinedProductionNodeType &nodeType )
{
	xn::Generator generator;
	getExistingProductionNode( nodeType, generator );
	XnBool newData = generator.IsDataNew();
	if ( newData )
		return true;
	return false;
}

ImageDataRef OpenNIDevice::getBuffer( const XnPredefinedProductionNodeType &nodeType )
{
	if ( nodeType == XN_NODE_TYPE_IMAGE && hasGenerator( XN_NODE_TYPE_IMAGE ) )
	{
		xn::ImageMetaData imgMeta;
		m_ImageGenerator.GetMetaData( imgMeta );

		m_ColorImage.setData( (uint8_t*)m_ImageGenerator.GetImageMap());
		m_ColorImage.setFullResolution( imgMeta.FullXRes(), imgMeta.FullYRes() );
		m_ColorImage.setCroppedResolution( imgMeta.XRes(), imgMeta.YRes() );
		m_ColorImage.setCroppingOffest( imgMeta.XOffset(), imgMeta.YOffset() );
		m_ColorImage.setTimestamp( imgMeta.Timestamp() );
		m_ColorImage.setFrameID( imgMeta.FrameID() );

		m_ColorImage.setBytesPerPixel(imgMeta.BytesPerPixel());

		imgMeta.XOffset() ? m_ColorImage.setCropping(true) : m_ColorImage.setCropping(false);
		m_ImageGenerator.GetMirrorCap().IsMirrored() ? m_ColorImage.setMirroring(true) : m_ColorImage.setMirroring(false);

		return m_ColorImage.getData();
	}
	else if ( nodeType == XN_NODE_TYPE_DEPTH && hasGenerator( XN_NODE_TYPE_DEPTH ) )
	{
		ImageData16Ref buffer16	  = getBuffer16( XN_NODE_TYPE_DEPTH );
		_2RealVector2f dimensions = m_DepthImage.getCroppedResolution();
		unsigned int numPixels	  = dimensions.x * dimensions.y;
		convertImage_16_to_8( buffer16, m_DepthImage_8bit, numPixels, _2REAL_OPENNI_DEPTH_NORMALIZATION_16_TO_8 );

		return m_DepthImage_8bit;
	}
	else if ( nodeType == XN_NODE_TYPE_IR && hasGenerator( XN_NODE_TYPE_IR ) )
	{
		ImageData16Ref buffer16	  = getBuffer16( XN_NODE_TYPE_IR );
		_2RealVector2f dimensions = m_InfraredImage.getCroppedResolution();
		unsigned int numPixels	  = dimensions.x * dimensions.y;
		convertImage_16_to_8( buffer16, m_InfraredImage_8bit, numPixels, 255 );

		return m_InfraredImage_8bit;
	} 
	else if ( nodeType == XN_NODE_TYPE_USER && hasGenerator( XN_NODE_TYPE_USER ) )
	{
		ImageData16Ref buffer16 = getBuffer16( XN_NODE_TYPE_USER );
		_2RealVector2f dims = m_UserImage.getCroppedResolution();
		size_t numPixels = dims.x * dims.y;
		for ( size_t idx = 0; idx < numPixels; ++idx )
		{
			m_UserImageColor_8bit[idx*3+0] = 0;
			m_UserImageColor_8bit[idx*3+1] = 0;
			m_UserImageColor_8bit[idx*3+2] = 0;

			if ( (unsigned char)  buffer16[idx]  > 0 )
			{
				int colorIdx = buffer16[idx]%MAX_USERS;
				m_UserImageColor_8bit[idx*3+0] = Colors[colorIdx][0];
				m_UserImageColor_8bit[idx*3+1] = Colors[colorIdx][1];
				m_UserImageColor_8bit[idx*3+2] = Colors[colorIdx][2];
			}
		}
		return m_UserImageColor_8bit;
	}
	else 
	{
		throwError("_2Real: Requested node type does not produce image data or doesn't exist ");
	}
}

ImageData16Ref OpenNIDevice::getBuffer16( const XnPredefinedProductionNodeType &nodeType )
{
	if ( nodeType == XN_NODE_TYPE_DEPTH && hasGenerator( XN_NODE_TYPE_DEPTH ) )
	{
		xn::DepthMetaData depthMeta;
		m_DepthGenerator.GetMetaData( depthMeta );

		m_DepthImage.setData( (uint16_t*)m_DepthGenerator.GetDepthMap() );
		m_DepthImage.setFullResolution( depthMeta.FullXRes(), depthMeta.FullYRes() );
		m_DepthImage.setCroppedResolution( depthMeta.XRes(), depthMeta.YRes() );
		m_DepthImage.setCroppingOffest( depthMeta.XOffset(), depthMeta.YOffset() );
		m_DepthImage.setTimestamp( depthMeta.Timestamp() );
		m_DepthImage.setFrameID( depthMeta.FrameID() );
		m_DepthImage.setBytesPerPixel( depthMeta.BytesPerPixel() );

		depthMeta.XOffset() ? m_DepthImage.setCropping( true ) : m_DepthImage.setCropping( false );
		m_DepthGenerator.GetMirrorCap().IsMirrored() ? m_DepthImage.setMirroring(true) : m_DepthImage.setMirroring(false);

		return m_DepthImage.getData();
	}
	else if ( nodeType == XN_NODE_TYPE_IR && hasGenerator( XN_NODE_TYPE_IR ) )
	{
		xn::IRMetaData irMeta;
		m_IrGenerator.GetMetaData( irMeta );

		m_InfraredImage.setData( (uint16_t*)m_IrGenerator.GetIRMap() );
		m_InfraredImage.setFullResolution( irMeta.FullXRes(), irMeta.FullYRes() );
		m_InfraredImage.setCroppedResolution( irMeta.XRes(), irMeta.YRes() );
		m_InfraredImage.setCroppingOffest( irMeta.XOffset(), irMeta.YOffset() );
		m_InfraredImage.setTimestamp( irMeta.Timestamp() );
		m_InfraredImage.setFrameID( irMeta.FrameID() );
		m_InfraredImage.setBytesPerPixel( irMeta.BytesPerPixel() );

		irMeta.XOffset() ? m_InfraredImage.setCropping( true ) : m_InfraredImage.setCropping( false );
		m_IrGenerator.GetMirrorCap().IsMirrored() ? m_InfraredImage.setMirroring(true) : m_InfraredImage.setMirroring(false);

		return m_InfraredImage.getData();
	}
	else if ( nodeType == XN_NODE_TYPE_USER && hasGenerator( XN_NODE_TYPE_USER ) )
	{
		xn::SceneMetaData sceneMeta;
		m_UserGenerator.GetUserPixels( 0, sceneMeta );
		m_UserImage.setData( (uint16_t*)sceneMeta.Data() );
		m_UserImage.setFullResolution( sceneMeta.FullXRes(), sceneMeta.FullYRes() );
		m_UserImage.setCroppedResolution( sceneMeta.XRes(), sceneMeta.YRes() );
		m_UserImage.setCroppingOffest( sceneMeta.XOffset(), sceneMeta.YOffset() );
		m_UserImage.setTimestamp( sceneMeta.Timestamp() );
		m_UserImage.setFrameID( sceneMeta.FrameID() );
		m_UserImage.setBytesPerPixel( sceneMeta.BytesPerPixel() );

		sceneMeta.XOffset() ? m_UserImage.setCropping( true ) : m_UserImage.setCropping( false );
		m_UserGenerator.GetMirrorCap().IsMirrored() ? m_UserImage.setMirroring(true) : m_UserImage.setMirroring(false);

		return m_UserImage.getData();
	}
	else 
	{
		throwError("_2Real: Requested node type does not produce 16bit image data or doesn't exist ");
	}
}

void OpenNIDevice::convertRealWorldToProjective( XnUInt32 count, 		const XnPoint3D  	aRealWorld[], XnPoint3D  	aProjective[] )
{
	if ( !hasGenerator(XN_NODE_TYPE_DEPTH) )
	{
		throwError("A depth generator is needed for the conversions");
	} else {
		//Lets use the existing depthGenerator
		xn::DepthGenerator depthGen;
		getExistingProductionNode( XN_NODE_TYPE_DEPTH, depthGen );
		depthGen.ConvertRealWorldToProjective( count, aRealWorld, aProjective );
	}
}

void OpenNIDevice::convertProjectiveToRealWorld( XnUInt32 count, const XnPoint3D  	aProjective[], XnPoint3D  	aRealWorld[] )
{
	if ( !hasGenerator(XN_NODE_TYPE_DEPTH) )
	{
		throwError("A depth generator is needed for the conversions");
	} else {
		//Lets use the already existing one
		xn::DepthGenerator depthGen;
		getExistingProductionNode( XN_NODE_TYPE_DEPTH, depthGen );
		depthGen.ConvertProjectiveToRealWorld( count, aProjective, aRealWorld );
	}
}

void OpenNIDevice::alignDepthToColor( bool flag )
{
	if ( hasGenerator(XN_NODE_TYPE_DEPTH) && hasGenerator(XN_NODE_TYPE_IMAGE) )
	{
		if ( m_DepthGenerator.IsCapabilitySupported("AlternativeViewPoint") )
		{
			if ( flag )
			{
				checkError( m_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint( m_ImageGenerator ), "Error while setting alternate viewpoint");
			} else
			{
				checkError( m_DepthGenerator.GetAlternativeViewPointCap().ResetViewPoint( ), "Error while resetting alternate viewpoint");
			}
		}

	}
}

bool OpenNIDevice::depthIsAlignedToColor()
{
	if ( hasGenerator(XN_NODE_TYPE_DEPTH) && hasGenerator(XN_NODE_TYPE_IMAGE) )
	{
		if ( m_DepthGenerator.IsCapabilitySupported("AlternativeViewPoint") )
		{
			XnBool check = m_DepthGenerator.GetAlternativeViewPointCap().IsViewPointAs( m_ImageGenerator );
			return !!check;
		}
	} else 
	{
		_2REAL_LOG(info) << "Depth and Image generators are needed for alignment" << std::endl;
	}
	return false;
}


std::string OpenNIDevice::xnNodeTypeToString( const XnPredefinedProductionNodeType& nodeType )
{
	std::string nodeTypeString;
	switch( nodeType ) {
	case XN_NODE_TYPE_INVALID:
		nodeTypeString = "XN_NODE_TYPE_INVALID";
		break;
	case XN_NODE_TYPE_DEVICE:
		nodeTypeString = "XN_NODE_TYPE_DEVICE";
		break;
	case XN_NODE_TYPE_DEPTH:
		nodeTypeString = "XN_NODE_TYPE_DEPTH";
		break;
	case XN_NODE_TYPE_IMAGE:
		nodeTypeString = "XN_NODE_TYPE_IMAGE";
		break;
	case XN_NODE_TYPE_AUDIO:
		nodeTypeString = "XN_NODE_TYPE_AUDIO";
		break;
	case XN_NODE_TYPE_IR:
		nodeTypeString = "XN_NODE_TYPE_IR";
		break;
	case XN_NODE_TYPE_USER:
		nodeTypeString = "XN_NODE_TYPE_USER";
		break;
	case XN_NODE_TYPE_RECORDER:
		nodeTypeString = "XN_NODE_TYPE_RECORDER";
		break;
	case XN_NODE_TYPE_PLAYER:
		nodeTypeString = "XN_NODE_TYPE_PLAYER";
		break;
	case XN_NODE_TYPE_GESTURE:
		nodeTypeString = "XN_NODE_TYPE_GESTURE";
		break;
	case XN_NODE_TYPE_SCENE:
		nodeTypeString = "XN_NODE_TYPE_SCENE";
		break;
	case XN_NODE_TYPE_HANDS:
		nodeTypeString = "XN_NODE_TYPE_HANDS";
		break;
	case XN_NODE_TYPE_CODEC:
		nodeTypeString = "XN_NODE_TYPE_CODEC";
		break;
	case XN_NODE_TYPE_PRODUCTION_NODE:
		nodeTypeString = "XN_NODE_TYPE_PRODUCTION_NODE";
		break;
	case XN_NODE_TYPE_GENERATOR:
		nodeTypeString = "XN_NODE_TYPE_GENERATOR";
		break;
	case XN_NODE_TYPE_MAP_GENERATOR:
		nodeTypeString = "XN_NODE_TYPE_MAP_GENERATOR";
		break;
	case XN_NODE_TYPE_SCRIPT:
		nodeTypeString = "XN_NODE_TYPE_SCRIPT";
		break;
	default: 
		throw std::logic_error(__FILE__ ": enum En out of range");
		break;
	}
	return nodeTypeString;
}

XnSkeletonJointTransformation OpenNIDevice::getSkeletonJoint( XnUInt16 userIdx, XnSkeletonJoint jointType  )
{

	try
	{
		XnSkeletonJointTransformation requestedJoint;
		m_UserGenerator.GetSkeletonCap().GetSkeletonJoint( m_Users[userIdx], jointType, requestedJoint );
		return requestedJoint;
	}
	catch ( ... )
	{
		std::cout << "Could not get skeleton joint." << std::endl;
	}
}

_2RealTrackedJointRef OpenNIDevice::getUserJoint( const uint32_t userID, XnSkeletonJoint type )
{
	XnSkeletonJointTransformation joint;
	XnStatus status = m_UserGenerator.GetSkeletonCap().GetSkeletonJoint( userID, type, joint );

	//set position of joint
	_2RealVector3f worldPos = _2RealVector3f( joint.position.position.X, joint.position.position.Y, joint.position.position.Z);
	XnPoint3D screenPos;

	xn::UserGenerator depthGen;
	getExistingProductionNode( XN_NODE_TYPE_DEPTH, depthGen );	
	xnConvertRealWorldToProjective( depthGen.GetHandle(), 1, &joint.position.position, &screenPos ); 
	_2RealMatrix3x3 mat;

	//for( int i=0; i < 9; ++i )
	//	mat.elements[i] = joint.orientation.orientation.elements[i];

	mat.m11 = joint.orientation.orientation.elements[0];
	mat.m12 = joint.orientation.orientation.elements[1];
	mat.m13 = joint.orientation.orientation.elements[2];
	mat.m21 = joint.orientation.orientation.elements[3];
	mat.m22 = joint.orientation.orientation.elements[4];
	mat.m23 = joint.orientation.orientation.elements[5];
	mat.m31 = joint.orientation.orientation.elements[6];
	mat.m32 = joint.orientation.orientation.elements[7];
	mat.m33 = joint.orientation.orientation.elements[8];


	_2RealJointConfidence confidence((float)joint.position.fConfidence, (float)joint.orientation.fConfidence);

	return boost::shared_ptr<_2RealTrackedJoint>( new _2RealTrackedJoint(  (_2RealJointType) type,
		_2RealVector3f( screenPos.X, screenPos.Y, screenPos.Z ),
		worldPos,
		mat,
		confidence));
}


XnStatus OpenNIDevice::getUserByID( uint32_t userID, boost::shared_ptr<_2RealTrackedUser> user )
{
	if ( m_UserGenerator.GetSkeletonCap().IsTracking(userID) )
	{
		user->setJoint( JOINT_HEAD, getUserJoint( userID, XN_SKEL_HEAD ) );
		user->setJoint( JOINT_NECK, getUserJoint( userID, XN_SKEL_NECK ) );
		user->setJoint( JOINT_TORSO, getUserJoint( userID, XN_SKEL_TORSO ) );
		user->setJoint( JOINT_WAIST, getUserJoint( userID, XN_SKEL_WAIST ) );

		user->setJoint( JOINT_LEFT_COLLAR, getUserJoint( userID, XN_SKEL_LEFT_COLLAR ) );
		user->setJoint( JOINT_LEFT_SHOULDER, getUserJoint( userID, XN_SKEL_LEFT_SHOULDER ) );
		user->setJoint( JOINT_LEFT_ELBOW, getUserJoint( userID, XN_SKEL_LEFT_ELBOW ) );
		user->setJoint( JOINT_LEFT_WRIST, getUserJoint( userID, XN_SKEL_LEFT_WRIST ) );
		user->setJoint( JOINT_LEFT_HAND, getUserJoint( userID, XN_SKEL_LEFT_HAND ) );
		user->setJoint( JOINT_LEFT_FINGERTIP, getUserJoint( userID, XN_SKEL_LEFT_FINGERTIP ) );

		user->setJoint( JOINT_RIGHT_COLLAR, getUserJoint( userID, XN_SKEL_RIGHT_COLLAR ) );
		user->setJoint( JOINT_RIGHT_SHOULDER, getUserJoint( userID, XN_SKEL_RIGHT_SHOULDER ) );
		user->setJoint( JOINT_RIGHT_ELBOW, getUserJoint( userID, XN_SKEL_RIGHT_ELBOW ) );
		user->setJoint( JOINT_RIGHT_WRIST, getUserJoint( userID, XN_SKEL_RIGHT_WRIST ) );
		user->setJoint( JOINT_RIGHT_HAND, getUserJoint( userID, XN_SKEL_RIGHT_HAND ) );
		user->setJoint( JOINT_RIGHT_FINGERTIP, getUserJoint( userID, XN_SKEL_RIGHT_FINGERTIP ) );

		user->setJoint( JOINT_LEFT_HIP, getUserJoint( userID, XN_SKEL_LEFT_HIP ) );
		user->setJoint( JOINT_LEFT_KNEE, getUserJoint( userID, XN_SKEL_LEFT_KNEE ) );
		user->setJoint( JOINT_LEFT_ANKLE, getUserJoint( userID, XN_SKEL_LEFT_ANKLE ) );
		user->setJoint( JOINT_LEFT_FOOT, getUserJoint( userID, XN_SKEL_LEFT_FOOT ) );

		user->setJoint( JOINT_RIGHT_HIP, getUserJoint( userID, XN_SKEL_RIGHT_HIP ) );
		user->setJoint( JOINT_RIGHT_KNEE, getUserJoint( userID, XN_SKEL_RIGHT_KNEE ) );
		user->setJoint( JOINT_RIGHT_ANKLE, getUserJoint( userID, XN_SKEL_RIGHT_ANKLE ) );
		user->setJoint( JOINT_RIGHT_FOOT, getUserJoint( userID, XN_SKEL_RIGHT_FOOT ) );
	}

	return getErrorState();
}

_2RealTrackedUserVector OpenNIDevice::getUsers()
{
	_2RealTrackedUserVector trackedUsers;
	trackedUsers.clear();
	XnUInt16 numberOfUsers = MAX_USERS;	// is used as an input parameter for GetUser, means how many users to retrieve 
	XnUserID currentUsers[ MAX_USERS];

	checkError( m_UserGenerator.GetUsers( currentUsers, numberOfUsers ) | getErrorState(), "_2Real: OpenNIDevice  Error while attempting to get the users." );

	for (int i= 0; i<(int)numberOfUsers; i++)
	{
		boost::shared_ptr<_2RealTrackedUser> user = boost::shared_ptr<_2RealTrackedUser>( new _2RealTrackedUser(currentUsers[i]) );
		if ( m_UserGenerator.GetSkeletonCap().IsTracking( currentUsers[i] ) )
		{
			getUserByID( currentUsers[i], user );
			trackedUsers.push_back( user );
		}
	}
	return trackedUsers;
}

size_t OpenNIDevice::getNumberOfUsers()
{
	return m_UserGenerator.GetNumberOfUsers();
}

XnStatus OpenNIDevice::getErrorState() const
{
	return m_UserGenerator.GetErrorStateCap().GetErrorState();
}

void  OpenNIDevice::forceResetUser( const uint32_t id )
{
	XnChar calibrationPose[20];
	xn::SkeletonCapability cap = m_UserGenerator.GetSkeletonCap();
	if ( cap.IsTracking( id ) )
	{
		m_UserGenerator.GetSkeletonCap().Reset( id );
		m_UserGenerator.GetSkeletonCap().GetCalibrationPose( calibrationPose );
		m_UserGenerator.GetPoseDetectionCap().StartPoseDetection( calibrationPose, id );
	}
}

void  OpenNIDevice::forceResetUsers( )
{
	XnUInt16 numberOfUsers = MAX_USERS;
	XnUserID *currentUsers = new XnUserID[MAX_USERS];
	XnChar calibrationPose[20];

	checkError( m_UserGenerator.GetUsers( currentUsers, numberOfUsers ), "_2Real:OpenNIDevice Error when getting users \n" );
	xn::SkeletonCapability cap = m_UserGenerator.GetSkeletonCap();
	for (unsigned int i= 0; i<numberOfUsers; ++i)
	{
		XnUserID id = currentUsers[i];
		if ( cap.IsTracking( currentUsers[i] ) )
		{
			m_UserGenerator.GetSkeletonCap().Reset( id );
			m_UserGenerator.GetSkeletonCap().GetCalibrationPose( calibrationPose );
			m_UserGenerator.GetPoseDetectionCap().StartPoseDetection( calibrationPose, id );
		}
	}
	delete [] currentUsers;
}

void OpenNIDevice::setMotorAngle( int angle )
{
	if ( !m_MotorInitialized )
		return;
	m_DeviceMotorController.Move( angle );
}

int	OpenNIDevice::getMotorAngle()
{
	if ( !m_MotorInitialized )
		return 0;
	return m_DeviceMotorController.GetAngle();
}
OpenNIDeviceConfiguration OpenNIDevice::getDeviceConfiguration()
{
	OpenNIDeviceConfiguration  devConf;
	boost::uint32_t _2RealImageConfig  = 0;
	boost::uint32_t _2RealGeneratorConfig  = 0;
	//Check for color image
	if ( hasGenerator(XN_NODE_TYPE_IMAGE ) )
	{
		xn::ProductionNode imageNode;
		getExistingProductionNode( XN_NODE_TYPE_IMAGE, imageNode );
		//Get resolution
		xn::MapGenerator mapGenerator = static_cast<xn::MapGenerator>(imageNode);
		XnMapOutputMode imageOutputMode;
		checkError( mapGenerator.GetMapOutputMode( imageOutputMode ), "Error while trying to get image output mode");
		boost::uint32_t _2RealImageMode = getImageConfig2Real( XN_NODE_TYPE_IMAGE, imageOutputMode );
		_2RealImageConfig = _2RealImageConfig | _2RealImageMode;
		//Get mirrored
		devConf.setMirror( XN_NODE_TYPE_IMAGE, mapGenerator.GetMirrorCap().IsMirrored() );
		_2RealGeneratorConfig = _2RealGeneratorConfig | COLORIMAGE;
		std::cout << "Has COLOR " << std::endl;
	}
	//Check for depth image
	if ( hasGenerator(XN_NODE_TYPE_DEPTH ) )
	{
		xn::ProductionNode depthNode;
		getExistingProductionNode( XN_NODE_TYPE_DEPTH, depthNode );
		//Get resolution
		xn::MapGenerator mapGenerator = static_cast<xn::MapGenerator>(depthNode);
		XnMapOutputMode depthOutputMode;
		checkError( mapGenerator.GetMapOutputMode( depthOutputMode ), "Error while trying to get depth output mode");
		boost::uint32_t _2RealImageMode = getImageConfig2Real( XN_NODE_TYPE_DEPTH, depthOutputMode );
		_2RealImageConfig = _2RealImageConfig | _2RealImageMode;
		//Get mirrored
		devConf.setMirror( XN_NODE_TYPE_DEPTH, mapGenerator.GetMirrorCap().IsMirrored() );
		_2RealGeneratorConfig = _2RealGeneratorConfig | DEPTHIMAGE;
		std::cout << "Has DEPTH " << std::endl;
	}
	//Check for user image
	if ( hasGenerator(XN_NODE_TYPE_USER ) )
	{
		xn::ProductionNode userNode;
		getExistingProductionNode( XN_NODE_TYPE_DEPTH, userNode );
		//Get resolution
		xn::MapGenerator mapGenerator = static_cast<xn::MapGenerator>(userNode);
		XnMapOutputMode userOutputMode;
		checkError( mapGenerator.GetMapOutputMode( userOutputMode ), "Error while trying to get depth output mode");
		boost::uint32_t _2RealImageMode = getImageConfig2Real( XN_NODE_TYPE_USER, userOutputMode );
		_2RealImageConfig = _2RealImageConfig | _2RealImageMode;
		//Get mirrored
		devConf.setMirror( XN_NODE_TYPE_USER, mapGenerator.GetMirrorCap().IsMirrored() );
		_2RealGeneratorConfig = _2RealGeneratorConfig | USERIMAGE;
		std::cout << "Has USER " << std::endl;
	}
	//check for ir image
	if ( hasGenerator(XN_NODE_TYPE_IR ) )
	{
		xn::ProductionNode irNode;
		getExistingProductionNode( XN_NODE_TYPE_IR, irNode );
		//Get resolution
		xn::MapGenerator mapGenerator = static_cast<xn::MapGenerator>(irNode);
		XnMapOutputMode irOutputMode;
		checkError( mapGenerator.GetMapOutputMode( irOutputMode ), "Error while trying to get depth output mode");
		boost::uint32_t _2RealImageMode = getImageConfig2Real( XN_NODE_TYPE_IR, irOutputMode );
		_2RealImageConfig = _2RealImageConfig | _2RealImageMode;
		//Get mirrored
		devConf.setMirror( XN_NODE_TYPE_IR, mapGenerator.GetMirrorCap().IsMirrored() );
		_2RealGeneratorConfig = _2RealGeneratorConfig | INFRAREDIMAGE;
		std::cout << "Has IR " << std::endl;
	}
	devConf.setImageConfig( _2RealImageConfig );
	devConf.setGeneratorConfig( _2RealGeneratorConfig );
	return devConf;
}

void OpenNIDevice::registerUserCallbacks()
{
	if (!m_UserGenerator.IsCapabilitySupported( XN_CAPABILITY_SKELETON ) )
	{
		_2REAL_LOG(info) << "\n_2Real: Skeleton Capability Is Not Supprted " << std::endl;
		return;
	}

	checkError( m_UserGenerator.RegisterUserCallbacks( newUserCb, lostUserCb, (xn::UserGenerator *) &m_UserGenerator, userCbHandle ), "Cannot Register User Callbacks" );
	checkError( m_UserGenerator.RegisterToUserExit( userExitCb, (xn::UserGenerator *) &m_UserGenerator, userExitCbHandle), "Cannot Register UserExit Callback" );
	checkError( m_UserGenerator.RegisterToUserReEnter( userReentryCb, (xn::UserGenerator *)&m_UserGenerator, userReentryCbHandle), "Cannot Register UserExit Callback" );
	checkError( m_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart( userCalibrationStartedCb,  (xn::UserGenerator *)&m_UserGenerator, calibrationStartedCbHandle ) , "Cannot Register User Calibration Started Callback ");
	checkError( m_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete( userCalibrationCompletedCb,  (xn::UserGenerator *)&m_UserGenerator, calibrationCompletedCbHandle ), "Cannot Register User Calibration Completed Callback");

	if ( m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration() )
	{
		g_bNeedPose = TRUE;
		checkError( m_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected( poseDetectedCb, NULL, poseDetectedCbHandle), "Cannot register pose detected callback");
		m_UserGenerator.GetSkeletonCap().GetCalibrationPose( g_strPose );
	}
	m_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
}

} //namespace
#endif
