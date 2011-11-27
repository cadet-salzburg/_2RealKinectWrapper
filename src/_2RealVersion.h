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

#include <string>
#include <sstream>

#define API_MAJOR 0
#define API_MINOR 9
#define API_PATCH 1

namespace _2Real
{
class _2RealVersion
{
public:
	static int				getMajor()
	{
		return API_MAJOR;
	};

	static int				getMinor()
	{
		return API_MINOR;
	};

	static int				getPatch()
	{
		return API_PATCH;
	};

	static std::string		getVersion()
	{
		std::stringstream ss;
		ss << API_MAJOR << "." << API_MINOR << "." << API_PATCH;
		return ss.str();
	};

	static bool isAtLeast(int major, int minor, int patch)
	{
		return (major*100 + minor*10 + patch) <= (API_MAJOR*100 + API_MINOR*10 + API_PATCH);
	}
};
} // namespace end