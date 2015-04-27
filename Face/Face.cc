//
// Copyright 2005 (C) Eiichiro ITO, GHC02331@nifty.com
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
// Eiichiro ITO, 15 October 2005
// mailto: GHC02331@nifty.com
//

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include <OPENR/core_macro.h>
#include "Face.h"

#ifdef ERS210
static const char* const EAR_LOCATOR[NUM_EARS] = {
    "PRM:/r1/c1/c2/c3/e1-Joint3:j5",
    "PRM:/r1/c1/c2/c3/e2-Joint3:j6",
};
static const char* const LED_LOCATOR[NUM_LEDS] = {
    "PRM:/r1/c1/c2/c3/l1-LED2:l1",
    "PRM:/r1/c1/c2/c3/l2-LED2:l2",
    "PRM:/r1/c1/c2/c3/l3-LED2:l3",
    "PRM:/r1/c1/c2/c3/l4-LED2:l4",
    "PRM:/r1/c1/c2/c3/l5-LED2:l5",
    "PRM:/r1/c1/c2/c3/l6-LED2:l6",
};
static const char* const MODE_LOCATOR[NUM_MODES] = {
    "PRM:/r1/c1/c2/c3/l7-LED2:l7",
};
#endif // ERS210
#ifdef ERS7
static const char* const EAR_LOCATOR[NUM_EARS] = {
    "PRM:/r1/c1/c2/c3/e5-Joint4:15",
    "PRM:/r1/c1/c2/c3/e6-Joint4:16",
};
static const char* const HEAD_LOCATOR[NUM_HEADS] = {
    "PRM:/r1/c1/c2/c3/l1-LED2:l1", // Head light (color)
    "PRM:/r1/c1/c2/c3/l2-LED2:l2", // Head light (white)
};
static const char* const MODE_LOCATOR[NUM_MODES] = {
    "PRM:/r1/c1/c2/c3/l3-LED2:l3", // Mode Indicator (red)
    "PRM:/r1/c1/c2/c3/l4-LED2:l4", // Mode Indicator (green)
    "PRM:/r1/c1/c2/c3/l5-LED2:l5", // Mode Indicator (blue)
};
static const char* const LAN_LOCATOR[NUM_LANS] = {
    "PRM:/r1/c1/c2/c3/l6-LED2:l6", // Wireless light
};
static const char* const FACE_LOCATOR[NUM_FACES] = {
    "PRM:/r1/c1/c2/c3/la-LED3:la", // 1
    "PRM:/r1/c1/c2/c3/lb-LED3:lb", // 2
    "PRM:/r1/c1/c2/c3/lc-LED3:lc", // 3
    "PRM:/r1/c1/c2/c3/ld-LED3:ld", // 4
    "PRM:/r1/c1/c2/c3/le-LED3:le", // 5
    "PRM:/r1/c1/c2/c3/lf-LED3:lf", // 6
    "PRM:/r1/c1/c2/c3/lg-LED3:lg", // 7
    "PRM:/r1/c1/c2/c3/lh-LED3:lh", // 8
    "PRM:/r1/c1/c2/c3/li-LED3:li", // 9
    "PRM:/r1/c1/c2/c3/lj-LED3:lj", // 10
    "PRM:/r1/c1/c2/c3/lk-LED3:lk", // 11
    "PRM:/r1/c1/c2/c3/ll-LED3:ll", // 12
    "PRM:/r1/c1/c2/c3/lm-LED3:lm", // 13
    "PRM:/r1/c1/c2/c3/ln-LED3:ln", // 14
};
static const int NFrames_leftRight = 6;
static const int toLeftIndex[] = {
    2, 4, 11, 5, 3, -1
};
static const int toRightIndex[] = {
    3, 5, 11, 4, 2, -1
};
#endif // ERS7

Face::Face() : faceState(FS_IDLE), lastFCmd(fcmdOff)
{
    for ( int i = 0; i < NUM_EARS; i++ ) {
	earID[i] = oprimitiveID_UNDEF;
    }
#ifdef ERS210
    for ( int i = 0; i < NUM_LEDS; i++ ) {
	ledID[i] = oprimitiveID_UNDEF;
    }
    for ( int i = 0; i < NUM_MODES; i++ ) {
	modeID[i] = oprimitiveID_UNDEF;
    }
#endif // ERS210
#ifdef ERS7
    for ( int i = 0; i < NUM_HEADS; i++ ) {
	headID[i] = oprimitiveID_UNDEF;
    }
    for ( int i = 0; i < NUM_MODES; i++ ) {
	modeID[i] = oprimitiveID_UNDEF;
    }
    for ( int i = 0; i < NUM_LANS; i++ ) {
	lanID[i] = oprimitiveID_UNDEF;
    }
    for ( int i = 0; i < NUM_FACES; i++ ) {
	faceID[i] = oprimitiveID_UNDEF;
    }
#endif // ERS7
    for ( int i = 0; i < NumFaceCommands; i++ ) {
	region[i] = 0;
    }
}

OStatus
Face::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("Face::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OpenPrimitives();
    NewCommandVectorData1();

    return oSUCCESS;
}

OStatus
Face::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("Face::DoStart()\n"));

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    faceState = FS_READY;

    return oSUCCESS;
}

OStatus
Face::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("Face::DoStop()\n"));

    faceState = FS_IDLE;

    DISABLE_ALL_SUBJECT;    
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
Face::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

void
Face::OpenPrimitives()
{
    for (int i = 0; i < NUM_EARS; i++) {
        OStatus result = OPENR::OpenPrimitive(EAR_LOCATOR[i], &earID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Face::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
#ifdef ERS210
    for (int i = 0; i < NUM_LEDS; i++) {
        OStatus result = OPENR::OpenPrimitive(LED_LOCATOR[i], &ledID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Face::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
    for (int i = 0; i < NUM_MODES; i++) {
        OStatus result = OPENR::OpenPrimitive(MODE_LOCATOR[i], &modeID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Face::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
#endif // ERS210
#ifdef ERS7
    for (int i = 0; i < NUM_HEADS; i++) {
        OStatus result = OPENR::OpenPrimitive(HEAD_LOCATOR[i], &headID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Face::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
    for (int i = 0; i < NUM_MODES; i++) {
        OStatus result = OPENR::OpenPrimitive(MODE_LOCATOR[i], &modeID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Face::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
    for (int i = 0; i < NUM_LANS; i++) {
        OStatus result = OPENR::OpenPrimitive(LAN_LOCATOR[i], &lanID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Face::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
    for (int i = 0; i < NUM_FACES; i++) {
        OStatus result = OPENR::OpenPrimitive(FACE_LOCATOR[i], &faceID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "Face::OpenPrimitives()",
                      "OPENR::OpenPrimitive() FAILED", result));
        }
    }
#endif // ERS7
}

bool
Face::NewCommandVectorData2( int index, int numData,
			     OCommandVectorData** cmdVecDataPtr)
{
    OStatus result;

    MemoryRegionID cmdVecDataID;
    result = OPENR::NewCommandVectorData( numData,
					  &cmdVecDataID, cmdVecDataPtr ) ;
    if (result != oSUCCESS) {
	OSYSLOG1((osyslogERROR, "%s : %s %d",
		  "Face::NewCommandVectorData2()",
		  "OPENR::NewCommandVectorData() FAILED", result));
	return false;
    }
    OCommandVectorData* cmdVecData = *cmdVecDataPtr;
    region[index] = new RCRegion( cmdVecData->vectorInfo.memRegionID,
				  cmdVecData->vectorInfo.offset,
				  (void*)cmdVecData,
				  cmdVecData->vectorInfo.totalSize);
    cmdVecData->SetNumData( numData );
    return true;
}

#ifdef ERS210
void
Face::NewCommandVectorData1()
{
    OCommandVectorData* cmdVecData;

    for ( int i = 0; i < NumFaceCommands; i ++ ) {
	if ( i == fcmdEarMove1 ) {
	    if ( !NewCommandVectorData2( i, NUM_EARS, &cmdVecData ) ) {
		continue;
	    }
	    for ( int j = 0; j < NUM_EARS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataJOINT_COMMAND3, earID[j], 3 );
		OCommandData* data = cmdVecData->GetData(j);
		OJointCommandValue3* val = (OJointCommandValue3*)data->value;
		val[0].value = ojoint3_STATE1;
		val[1].value = ojoint3_STATE1;
		val[2].value = ojoint3_STATE0;
	    }
	} else if ( i == fcmdEarMove2 ) {
	    if ( !NewCommandVectorData2( i, NUM_EARS, &cmdVecData ) ) {
		continue;
	    }
	    for ( int j = 0; j < NUM_EARS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataJOINT_COMMAND3, earID[j], 7 );
		OCommandData* data = cmdVecData->GetData(j);
		OJointCommandValue3* val = (OJointCommandValue3*)data->value;
		val[0].value = ojoint3_STATE1;
		val[1].value = ojoint3_STATE1;
		val[2].value = ojoint3_STATE0;
		val[3].value = ojoint3_STATE0;
		val[4].value = ojoint3_STATE1;
		val[5].value = ojoint3_STATE1;
		val[6].value = ojoint3_STATE0;
	    }
	} else if ( i == fcmdEarUp || i == fcmdEarDown ) {
	    if ( !NewCommandVectorData2( i, NUM_EARS, &cmdVecData ) ) {
		continue;
	    }
	    for ( int j = 0; j < NUM_EARS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataJOINT_COMMAND3, earID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OJointCommandValue3* val = (OJointCommandValue3*)data->value;
		val[0].value
		    = (i == fcmdEarUp ? ojoint3_STATE1 : ojoint3_STATE0 );
	    }
	} else if ( i == fcmdOff || i == fcmdAll ||
		    i == fcmdSerious || i == fcmdLaugh || i == fcmdCry ) {
	    if ( !NewCommandVectorData2( i, NUM_LEDS, &cmdVecData ) ) {
		continue;
	    }
	    for ( int j = 0; j < NUM_LEDS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND2, ledID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
		if ( i == fcmdSerious ) {
		    val[0].led = (j == 2 || j == 5) ? oledON : oledOFF;
		} else if ( i == fcmdLaugh ) {
		    val[0].led = (j == 1 || j == 4) ? oledON : oledOFF;
		} else if ( i == fcmdCry ) {
		    val[0].led = (j == 0 || j == 3) ? oledON : oledOFF;
		} else if ( i == fcmdAll ) {
		    val[0].led = oledON;
		} else { // fcmdOff
		    val[0].led = oledOFF;
		}
		val[0].period = 1; // 8ms * 1 = 8ms
            }
	} else if ( i == fcmdModeOn || i == fcmdModeOff ) {
	    if ( !NewCommandVectorData2( i, NUM_MODES, &cmdVecData ) ) {
		continue;
	    }
	    for ( int j = 0; j < NUM_MODES; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND2, modeID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
		val[0].led = (i == fcmdModeOn) ? oledON : oledOFF;
		val[0].period = 1; // 8ms * 1 = 8ms
            }
	} else if ( i == fcmdULeft || i == fcmdURight ||
		    i == fcmdMLeft || i == fcmdMRight ||
		    i == fcmdLLeft || i == fcmdLRight ) {
	    if ( !NewCommandVectorData2( i, NUM_LEDS, &cmdVecData ) ) {
		continue;
	    }
	    for ( int j = 0; j < NUM_LEDS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND2, ledID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
		if ( i == fcmdULeft ) {
		    val[0].led = j == 2 ? oledON : oledOFF;
		} else if ( i == fcmdURight ) {
		    val[0].led = j == 5 ? oledON : oledOFF;
		} else if ( i == fcmdMLeft ) {
		    val[0].led = j == 1 ? oledON : oledOFF;
		} else if ( i == fcmdMRight ) {
		    val[0].led = j == 4 ? oledON : oledOFF;
		} else if ( i == fcmdLLeft ) {
		    val[0].led = j == 0 ? oledON : oledOFF;
		} else if ( i == fcmdLRight ) {
		    val[0].led = j == 3 ? oledON : oledOFF;
		}
		val[0].period = 1; // 8ms * 1 = 8ms
            }
	}
    }
}
#endif // ERS210
#ifdef ERS7
void
Face::NewCommandVectorData1()
{
    OCommandVectorData* cmdVecData = NULL;

    for ( int i = 0; i < NumFaceCommands; i ++ ) {
	if ( i == fcmdEarMove1 ) {
	    if ( !NewCommandVectorData2( i, NUM_EARS, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err1\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_EARS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataJOINT_COMMAND4, earID[j], 3 );
		OCommandData* data = cmdVecData->GetData(j);
		OJointCommandValue4* val = (OJointCommandValue4*)data->value;
		val[0].value = ojoint4_STATE1; val[0].period = 16;
		val[1].value = ojoint4_STATE1; val[1].period = 16;
		val[2].value = ojoint4_STATE0; val[2].period = 16;
	    }
	} else if ( i == fcmdEarMove2 ) {
	    if ( !NewCommandVectorData2( i, NUM_EARS, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err2\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_EARS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataJOINT_COMMAND4, earID[j], 7 );
		OCommandData* data = cmdVecData->GetData(j);
		OJointCommandValue4* val = (OJointCommandValue4*)data->value;
		val[0].value = ojoint4_STATE1; val[0].period = 16;
		val[1].value = ojoint4_STATE1; val[1].period = 16;
		val[2].value = ojoint4_STATE0; val[2].period = 16;
		val[3].value = ojoint4_STATE0; val[3].period = 16;
		val[4].value = ojoint4_STATE1; val[4].period = 16;
		val[5].value = ojoint4_STATE1; val[5].period = 16;
		val[6].value = ojoint4_STATE0; val[6].period = 16;
	    }
	} else if ( i == fcmdEarUp || i == fcmdEarDown ) {
	    if ( !NewCommandVectorData2( i, NUM_EARS, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err3\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_EARS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataJOINT_COMMAND4, earID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OJointCommandValue4* val = (OJointCommandValue4*)data->value;
		val[0].value
		    = (i == fcmdEarUp ? ojoint4_STATE1 : ojoint4_STATE0 );
                val[0].period = 16;
	    }
	} else if ( i == fcmdSerious || i == fcmdLaugh || i == fcmdCry ) {
	    if ( !NewCommandVectorData2( i, NUM_FACES, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err4\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_FACES; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND3, faceID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue3* val = (OLEDCommandValue3*)data->value;
		if ( i == fcmdSerious ) {
		    val[0].intensity = (j == 2 || j == 3) ? 255 : 0;
		    val[0].mode = oled3_MODE_B;
		} else if ( i == fcmdLaugh ) {
		    val[0].intensity = (j == 6 || j == 7) ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else if ( i == fcmdCry ) {
		    val[0].intensity = (j == 4 || j == 5) ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		}
		val[0].period = 1; // 8ms * 1 = 8ms
            }
	} else if ( i == fcmdLanOn || i == fcmdLanOff ) {
	    if ( !NewCommandVectorData2( i, NUM_LANS, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err5\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_LANS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND2, lanID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
		val[0].led = (i == fcmdLanOn) ? oledON : oledOFF;
		val[0].period = 1; // 8ms * 1 = 8ms
            }
	} else if ( i == fcmdToLeft || i == fcmdToRight ) {
	    if ( !NewCommandVectorData2( i, NUM_FACES, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err6\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_FACES; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND3, faceID[j], NFrames_leftRight );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue3* val = (OLEDCommandValue3*)data->value;
		for ( int k = 0; k < NFrames_leftRight; k ++ ) {
		    if ( i == fcmdToLeft ) {
			val[k].intensity = (j == toLeftIndex[k]) ? 255 : 0;
		    }
		    if ( i == fcmdToRight ) {
			val[k].intensity = (j == toRightIndex[k]) ? 255 : 0;
		    }
		    val[k].mode = oled3_MODE_A;
		    val[k].period = 20;
		}
            }
	} else if ( i == fcmdULeft || i == fcmdURight ||
		    i == fcmdMLeft || i == fcmdMRight ||
		    i == fcmdLLeft || i == fcmdLRight ||
		    i == fcmdAll || i == fcmdOff ) {
	    if ( !NewCommandVectorData2( i, NUM_FACES, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err7\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_FACES; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND3, faceID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue3* val = (OLEDCommandValue3*)data->value;
		if ( i == fcmdULeft ) {
		    val[0].intensity = j == 7 ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else if ( i == fcmdURight ) {
		    val[0].intensity = j == 6 ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else if ( i == fcmdMLeft ) {
		    val[0].intensity = j == 3 ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else if ( i == fcmdMRight ) {
		    val[0].intensity = j == 2 ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else if ( i == fcmdLLeft ) {
		    val[0].intensity = j == 1 ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else if ( i == fcmdLRight ) {
		    val[0].intensity = j == 0 ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else if ( i == fcmdAll ) {
		    val[0].intensity = j <= 11 ? 255 : 0;
		    val[0].mode = oled3_MODE_A;
		} else {
		    val[0].intensity = 0;
		    val[0].mode = oled3_MODE_A;
		}
		val[0].period = 1; // 8ms * 1 = 8ms
            }
	} else if ( i == fcmdHeadColor || i == fcmdHeadWhite
		    || i == fcmdHeadOff ) {
	    if ( !NewCommandVectorData2( i, NUM_MODES, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err8\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_HEADS; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND2, headID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
		if ( i == fcmdHeadColor ) {
		    val[0].led = (j == 0) ? oledON : oledOFF;
		} else if ( i == fcmdHeadWhite ) {
		    val[0].led = (j == 1) ? oledON : oledOFF;
		} else {
		    val[0].led = oledOFF;
		}
		val[0].period = 1;
	    }
	} else if ( i == fcmdModeRed || i == fcmdModeGreen
		    || i == fcmdModeBlue || i == fcmdModeOff ) {
	    if ( !NewCommandVectorData2( i, NUM_MODES, &cmdVecData ) ) {
		continue;
	    }
	    if ( !cmdVecData ) {
		OSYSPRINT(("err9\n"));
		continue;
	    }
	    for ( int j = 0; j < NUM_MODES; j ++ ) {
		OCommandInfo* info = cmdVecData->GetInfo(j);
		info->Set( odataLED_COMMAND2, modeID[j], 1 );
		OCommandData* data = cmdVecData->GetData(j);
		OLEDCommandValue2* val = (OLEDCommandValue2*)data->value;
		if ( i == fcmdModeRed ) {
		    val[0].led = (j == 0) ? oledON : oledOFF;
		} else if ( i == fcmdModeGreen ) {
		    val[0].led = (j == 1) ? oledON : oledOFF;
		} else if ( i == fcmdModeBlue ) {
		    val[0].led = (j == 2) ? oledON : oledOFF;
		} else {
		    val[0].led = oledOFF;
		}
		val[0].period = 1;
	    }
	}
    }
}
#endif // ERS7

void
Face::Ready(const OReadyEvent& event)
{
    if ( faceState == FS_IDLE ) {
	// do nothing
	return;
    }
    OSYSPRINT(( "Face::Ready\n" ));
    faceState = FS_READY;
}

void
Face::Notify(const ONotifyEvent& event)
{
    if ( faceState == FS_IDLE ) {
	// do nothing
	return;
    }
    if ( faceState == FS_BUSY ) {
	OSYSPRINT(("Face::Notify command ignored in busy mode\n" ));
	observer[event.ObsIndex()]->AssertReady();
	return;
    }
    int fcmd = *(int*) event.Data(0);

    if ( fcmd != lastFCmd ) {
	if ( fcmd >= 0 && fcmd < NumFaceCommands ) {
	    if ( fcmd == fcmdEarMove1 || fcmd == fcmdEarMove2
#ifdef ERS7
		 || fcmd == fcmdToLeft || fcmd == fcmdToRight
#endif // ERS7
		) {
		lastFCmd = fcmdNone;
	    } else {
		lastFCmd = fcmd;
	    }
	    faceState = FS_BUSY;
	    subject[sbjCommand]->SetData(region[fcmd]);
	    subject[sbjCommand]->NotifyObservers();
	} else {
	    OSYSPRINT(("Face::Invalid fcmd=%d\n", fcmd ));
	}
    }
    observer[event.ObsIndex()]->AssertReady();
}
