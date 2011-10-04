#include "_2RealVector2f.h"
#include "_2RealTypes.h"


namespace _2Real
{

_2RealVector2f::_2RealVector2f( void ) : x( 0.0f ), y( 0.0f ) {}
_2RealVector2f::_2RealVector2f( float X, float Y ) : x( X ), y( Y ){}
_2RealVector2f::_2RealVector2f( const _2RealVector2f& o ) : x( o.x ), y( o.y ) {}
_2RealVector2f& _2RealVector2f::operator=( const _2RealVector2f& o )
{
	this->x = o.x; this->y = o.y;
	return *this;
}
_2RealVector2f& _2RealVector2f::operator=( const _2RealVector3f& o )
{
	this->x = o.x; this->y=o.y;
	return *this;
}

}
