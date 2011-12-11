#include "_2RealTypes.h"
#include "XnCppWrapper.h"
#include "XnOpenNI.h"
#include "XnTypes.h"

using namespace xn;

namespace _2Real
{


_2RealVector3f::_2RealVector3f( void ) : x(0.0f), y(0.0f), z(0.0f), confidence(0.0) {}
_2RealVector3f::_2RealVector3f( float X, float Y, float Z, float fConfidence ) : x( X ), y( Y ), z( Z ), confidence(fConfidence) {}
_2RealVector3f::_2RealVector3f( const _2RealVector3f& o ) : x( o.x ), y( o.y ), z( o.z ), confidence(o.confidence) {}
_2RealVector3f& _2RealVector3f::operator=( const _2RealVector3f& o )
{
	if( this == &o )
		return *this;
	this->x = o.x; this->y = o.y; this->z = o.z; this->confidence = o.confidence;
	return *this;
}

}
