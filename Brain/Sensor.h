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

#ifndef Sensor_h_DEFINED
#define Sensor_h_DEFINED
#include <OPENR/ODataFormats.h>
#include "command.h"

class Sensor {
  private:
    bool initSensor;
    static const int MaxSensors = 12;
    int commonidx[ MaxSensors ];
    slongword value[ MaxSensors ];
    int count[ MaxSensors ];

  public:
    Sensor();
    void Initialize();
    void SetSensor( OSensorFrameVectorData* sensorVec );
    bool IsPressed( int sensorCode );
    void Print() const;
    void InitCommonSensorIndex( OSensorFrameVectorData* sensorVec );

  private:
    bool IsValidSensorCode( int sensorCode ) const;
    void SetSensor( int sensorCode, slongword value );
};
#endif // Sensor_h_DEFINED
