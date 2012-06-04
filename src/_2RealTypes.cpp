#include "_2RealTypes.h"

namespace _2RealKinectWrapper
{


_2RealVector3f::_2RealVector3f( void ) : x(0.0f), y(0.0f), z(0.0f) {}
_2RealVector3f::_2RealVector3f( float X, float Y, float Z ) : x( X ), y( Y ), z( Z ) {}
_2RealVector3f::_2RealVector3f( const _2RealVector3f& o ) : x( o.x ), y( o.y ), z( o.z ) {}
_2RealVector3f& _2RealVector3f::operator=( const _2RealVector3f& o )
{
	if( this == &o )
		return *this;
	this->x = o.x; this->y = o.y; this->z = o.z; 
	return *this;
}

}
