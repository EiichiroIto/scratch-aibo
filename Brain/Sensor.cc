//
// Copyright 2003 (C) Eiichiro ITO, GHC02331@nifty.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Eiichiro ITO, 15 August 2003
// mailto: GHC02331@nifty.com
#include "Sensor.h"

#ifndef MAIN
#include <OPENR/OPENRAPI.h>
#include <OPENR/OObject.h>
#endif // MAIN

#define min(x,y) ((x)<(y)?(x):(y))

static const int SensorThreshold = 5;
static const int scodePSD = 8;
static const int scodeAccFrontRear = 9;
static const int scodeAccLeftRight = 10;
static const int scodeAccUpDown = 11;

#ifdef ERS210
static const char* const COMMON_SENSOR_LOCATOR[] = {
    "PRM:/r6/s1-Sensor:s1",             // BACK SWITCH
    "PRM:/r1/c1/c2/c3/f2-Sensor:f2",    // HEAD TACTILE SENSOR (FRONT)
    "PRM:/r1/c1/c2/c3/f1-Sensor:f1",    // HEAD TACTILE SENSOR (REAR)
    "PRM:/r1/c1/c2/c3/c4/s5-Sensor:s5", // TIN SWITCH
    "PRM:/r4/c1/c2/c3/c4-Sensor:s4",    // RFLEG SW
    "PRM:/r2/c1/c2/c3/c4-Sensor:s4",    // LFLEG SW
    "PRM:/r5/c1/c2/c3/c4-Sensor:s4",    // RRLEG SW
    "PRM:/r3/c1/c2/c3/c4-Sensor:s4",    // LRLEG SW
    "PRM:/r1/c1/c2/c3/p1-Sensor:p1",	// Head PSD Sensor
    "PRM:/a1-Sensor:a1",		// Acceleration Sensor FR
    "PRM:/a2-Sensor:a2",		// Acceleration Sensor LR
    "PRM:/a3-Sensor:a3",		// Acceleration Sensor UD
};
#endif // ERS210
#ifdef ERS7
static const char* const COMMON_SENSOR_LOCATOR[] = {
    "PRM:/t4-Sensor:t4",                // BACK SWITCH (FRONT)
    "PRM:/r1/c1/c2/c3/t1-Sensor:t1",    // HEAD TACTILE SENSOR
    "PRM:/t2-Sensor:t2",                // BACK SWITCH (REAR)
    "PRM:/r1/c1/c2/c3/c4/s5-Sensor:s5", // TIN SWITCH
    "PRM:/r4/c1/c2/c3/c4-Sensor:44",    // RFLEG SW
    "PRM:/r2/c1/c2/c3/c4-Sensor:24",    // LFLEG SW
    "PRM:/r5/c1/c2/c3/c4-Sensor:54",    // RRLEG SW
    "PRM:/r3/c1/c2/c3/c4-Sensor:34",    // LRLEG SW
    "PRM:/r1/c1/c2/c3/p2-Sensor:p2",	// Head PSD Sensor(far)
    "PRM:/a1-Sensor:a1",		// Acceleration Sensor FR
    "PRM:/a2-Sensor:a2",		// Acceleration Sensor LR
    "PRM:/a3-Sensor:a3",		// Acceleration Sensor UD
};
#endif // ERS7

Sensor::Sensor() : initSensor(false)

{
    for ( int i = 0; i < MaxSensors; i += 1 ) {
	commonidx[ i ] = -1;
    }
    Initialize();
}

void
Sensor::Initialize()
{
    for ( int i = 0; i < MaxSensors; i += 1 ) {
	value[ i ] = 0;
	count[ i ] = 0;
    }
}

void
Sensor::SetSensor( OSensorFrameVectorData* sensorVec )
{
    for ( int i = 0; i < MaxSensors; i += 1 ) {
	if ( commonidx[ i ] < 0 ) {
	    continue;
	}
	for ( int j = 0; j < 4; j += 1 ) {
	    SetSensor( i, sensorVec->GetData(commonidx[i])->frame[j].value );
	}
    }
}

void
Sensor::SetSensor( int sensorCode, slongword _value )
{
    if ( !IsValidSensorCode( sensorCode ) ) {
	return;
    }
#if 0
    if ( sensorCode == scodeAccFrontRear ) {
	if ( (_value / 10000) > 10 ) {
	    OSYSPRINT(("Sensor::SetSensor AccFR=%d\n", _value / 10000 ));
	}
	return;
    }
    if ( sensorCode == scodeAccLeftRight ) {
	if ( (_value / 10000) > 10 ) {
	    OSYSPRINT(("Sensor::SetSensor AccLR=%d\n", _value / 10000 ));
	}
	return;
    }
    if ( sensorCode == scodeAccUpDown ) {
	if ( (_value / 10000) > 10 ) {
	    OSYSPRINT(("Sensor::SetSensor AccUD=%d\n", _value / 10000 ));
	}
	return;
    }
#endif
    if ( value[ sensorCode ] == _value ) {
	count[ sensorCode ] += 1;
    } else {
	count[ sensorCode ] = 1;
	value[ sensorCode ] = _value;
    }
}

bool
Sensor::IsPressed( int sensorCode )
{
    if ( !IsValidSensorCode( sensorCode ) ) {
	return false;
    }
    if ( value[ sensorCode ] == oswitchON
	 && count[ sensorCode ] > SensorThreshold ) {
	count[ sensorCode ] = 0;
	return true;
    }
    return false;
}

bool
Sensor::IsValidSensorCode( int sensorCode ) const
{
    return 0 <= sensorCode && sensorCode < MaxSensors;
}

void
Sensor::Print() const
{
    OSYSPRINT(( " Sensor:" ));
    for ( int i = 0; i < MaxSensors; i ++ ) {
	if ( commonidx[i] >= 0 ) {
	    OSYSPRINT(( "(%d,%d)", value[ i ], count[ i ] ));
	}
    }
    OSYSPRINT(( "\n" ));
}

void
Sensor::InitCommonSensorIndex(OSensorFrameVectorData* sensorVec)
{
    OStatus result;
    OPrimitiveID sensorID;

    if ( initSensor ) {
	return;
    }
    initSensor = true;
    for (int i = 0; i < MaxSensors; i++) {
	if ( !*COMMON_SENSOR_LOCATOR[i] ) {
	    continue;
	}
        result = OPENR::OpenPrimitive(COMMON_SENSOR_LOCATOR[i], &sensorID);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "SensorObserver::InitCommonSensorIndex()",
                      "OPENR::OpenPrimitive() FAILED", result));
            continue;
        }

        for (int j = 0; j < sensorVec->vectorInfo.numData; j++) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
            if (info->primitiveID == sensorID) {
                commonidx[i] = j;
                OSYSPRINT(("%s %s=%2d\n",
			   "Sensor::InitCommonSensorIndex",
                           COMMON_SENSOR_LOCATOR[i], commonidx[i] ));
                break;
            }
        }
    }
}

