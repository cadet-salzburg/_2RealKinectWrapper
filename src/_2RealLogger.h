/*
	CADET - Center for Advances in Digital Entertainment Technologies
	Copyright 2011 Fachhochschule Salzburg GmbH

	http://www.cadet.at

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

	Authors: Robert Praxmarer, Gerlinde Emsenhuber, Robert Sommeregger
	Email: info@cadet.at
	Created: 08-09-2011
*/

#pragma once

//#include <boost/log/core.hpp>
//#include <boost/log/trivial.hpp>
//#include <boost/log/filters.hpp> 

#include <ostream>
#include <iostream>
#include "_2RealTypes.h"
#include <boost/thread/mutex.hpp>


namespace _2RealKinectWrapper
{
	#define _2REAL_LOG(level)	_2RealLogger::getInstance().setLogLevelForMsg(level); _2RealLogger::getInstance() 

class _2RealLogger			// implemented as singleton
{
public:
    static _2RealLogger&	getInstance();
	void					setLogLevel(_2RealLogLevel level);				// sets verbosity (external use for the user api)
	void					setLogLevelForMsg(_2RealLogLevel level);				// sets level of current stream (internal use in framework)
	void					setLogOutputStream(std::ostream* outStream);
	

	static std::ostream* m_pOutStream;

	// operator overloads
	template <typename T>
	friend _2RealLogger& operator <<(_2RealLogger& logger, T const& msg)	
	{
		boost::mutex::scoped_lock lock(m_Mutex);	// make logging thread safe
		if(m_iLogLevelMsg <= m_iLogLevel)			// just log to stream if the level of the message is under the level of the verbosity level
		{
			(*m_pOutStream) << msg;
			(*m_pOutStream).flush();
	}
		return logger;
	}

	friend _2RealLogger& operator <<( _2RealLogger& logger, std::ostream& (*pf)(std::ostream&) )	// overload for endl
	{
		boost::mutex::scoped_lock lock(m_Mutex);	// make logging thread safe
		if(m_iLogLevelMsg <= m_iLogLevel)			// just log to stream if the level of the message is under the level of the verbosity level
		{
			(*m_pOutStream) << pf;
			(*m_pOutStream).flush();
		}
		return logger;
	}

private:
	_2RealLogger( ) {};	// private constructor (set log level to debug means fully verbose)
	_2RealLogger( const _2RealLogger& );		// copy constructor

	static _2RealLogLevel m_iLogLevel;
	static _2RealLogLevel m_iLogLevelMsg;
	static boost::mutex   m_Mutex;
};
	
}	// namespace end