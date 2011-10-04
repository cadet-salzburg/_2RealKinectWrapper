#include "_2RealLogger.h"

namespace _2Real
{
	std::ostream* _2RealLogger::m_pOutStream = &std::cout;
	_2RealLogLevel _2RealLogger::m_iLogLevel = debug;				// default for logger is full verbosity
	_2RealLogLevel _2RealLogger::m_iLogLevelMsg = debug;			// init loglevel for msg
	boost::mutex _2RealLogger::m_Mutex;

_2RealLogger& _2RealLogger::getInstance()
{
	static _2RealLogger instance;
    return instance;
}

void _2RealLogger::setLogLevel(_2RealLogLevel iLevel)
{
	boost::mutex::scoped_lock lock(m_Mutex);	// make logging thread safe
	m_iLogLevel = iLevel;
}

void _2RealLogger::setLogLevelForMsg(_2RealLogLevel iLevel)		
{
	boost::mutex::scoped_lock lock(m_Mutex);	// make logging thread safe
	m_iLogLevelMsg = iLevel;
}

void _2RealLogger::setLogOutputStream(std::ostream* outStream)
{
	boost::mutex::scoped_lock lock(m_Mutex);	// make logging thread safe
	m_pOutStream = outStream;
}

} // namespace end