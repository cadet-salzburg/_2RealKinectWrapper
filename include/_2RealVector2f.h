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

   Authors: Robert Praxmarer, Gerlinde Emsenhuber, Robert Sommeregger, Andreas Stallinger
   Email: support@cadet.at
   Created: 08-31-2011
*/

#pragma once

namespace _2Real
{
	struct _2RealVector3f;

struct _2RealVector2f
{
	_2RealVector2f( void );
	_2RealVector2f( float X, float Y );
	_2RealVector2f( const _2RealVector2f& o );
	_2RealVector2f& operator=( const _2RealVector2f& o );
	_2RealVector2f& operator=( const _2RealVector3f& o );

	float x;
	float y;
};

}
