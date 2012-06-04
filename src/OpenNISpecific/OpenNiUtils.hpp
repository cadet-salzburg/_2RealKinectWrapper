#ifndef ONI_UTILS
#define ONI_UTILS

#include <XnCppWrapper.h>
#include <iostream>

namespace {
	
	void checkError( const XnStatus &status, const std::string &strError )
	{
		if ( status != XN_STATUS_OK )
		{
			std::cout << strError << " " << xnGetStatusString( status ) << std::endl;
		}
	};
	
	struct the_null_deleter
	{
		void operator()(void const *) const
		{
			
		}
	};
}
#endif