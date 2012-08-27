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


#define _2REAL_NUMBER_OF_JOINTS 24

//sensor reaches up to 10 meters in OpenNI
#define _2REAL_OPENNI_DEPTH_NORMALIZATION_16_TO_8 10000
#define _2REAL_WSDK_DEPTH_NORMALIZATION_16_TO_8 3500 	/*!< 3.5m for kinect */



//WSDK skeleton smoothing
#define _2REAL_WSDK_CORRECTION 0.0
#define _2REAL_WSDK_JITTER_RATIUS 0.03
#define _2REAL_WSDK_MAX_DEVIATION_RADIUS 0.03
#define _2REAL_WSDK_PREDICTION 0.0
#define _2REAL_WSDK_SMOOTHING 0.03
