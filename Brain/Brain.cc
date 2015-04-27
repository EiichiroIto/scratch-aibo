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
#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/core_macro.h>
#include <ControlInfo.h>
#include "Brain.h"

static const char* const FBK_LOCATOR = "PRM:/r1/c1/c2/c3/i1-FbkImageSensor:F1";

Brain::Brain () : brainState(BRAIN_IDLE),
		  fbkID(oprimitiveID_UNDEF),
		  commandQueue(),
		  waitingResult(false),
		  verboseMode(7),
		  stnHolder( this, &perception, &sensor ),
		  hcmdSeqno(0),
		  camWhiteBalance(1), camShutterSpeed(1), camGain(1)
{
    ClearLastCmds();
}

OStatus
Brain::DoInit(const OSystemEvent& event)
{
    camWhiteBalance = 1;
    camShutterSpeed = 1;
    camGain = 1;

    OSYSDEBUG(("Brain::DoInit()\n"));
    sensor.Initialize();

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    OStatus st = OPENR::SetMotorPower(opowerON);
    OSYSDEBUG(("OPENR::SetMotorPower(opowerON) %d\n", st));

    OSYSPRINT(( "1(%d,%d,%d)\n",
		camWhiteBalance, camShutterSpeed, camGain ));

    OpenPrimitive();
    ReadDefartConfig( DEFART_CONFIG );

    OSYSPRINT(( "2(%d,%d,%d)\n",
		camWhiteBalance, camShutterSpeed, camGain ));

    logging.Allocate();

    OSYSPRINT(( "3(%d,%d,%d)\n",
		camWhiteBalance, camShutterSpeed, camGain ));

    cdtfile.Read( CDT_CONFIG );
    cdtfile.SetCdtVectorData( fbkID );
    //cdtfile.Print();
    stnHolder.ReadAll();

    OSYSPRINT(( "4(%d,%d,%d)\n",
		camWhiteBalance, camShutterSpeed, camGain ));

    SetCamCPCPrimitives();

    return oSUCCESS;
}

OStatus
Brain::DoStart(const OSystemEvent& event)
{
    OSYSDEBUG(("Brain::DoStart()\n"));

    if (subject[sbjCommand]->IsReady() == true) {
        MonetExecuteDirect(SLEEP2SLEEP_NULL);
        brainState = BRAIN_WAITING_RESULT;
    } else {
        brainState = BRAIN_START;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
Brain::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("Brain::DoStop()\n"));

    brainState = BRAIN_IDLE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
Brain::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;

    logging.DeAllocate();

    return oSUCCESS;
}

void
Brain::OpenPrimitive()
{
    OStatus result;

    result = OPENR::OpenPrimitive(FBK_LOCATOR, &fbkID);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "Brain::OpenPrimitive()",
                  "OPENR::OpenPrimitive() FAILED", result));
    }
}

void
Brain::SetCamCPCPrimitives()
{
    OSYSPRINT(( "Brain::SetCamCPCPrimitives(%d,%d,%d)\n",
		camWhiteBalance, camShutterSpeed, camGain ));
    OStatus result;

    if ( camWhiteBalance == 0 ) {
	OPrimitiveControl_CameraParam wb(ocamparamWB_INDOOR_MODE);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_WHITE_BALANCE,
					 &wb, sizeof (wb), 0, 0);
    } else if ( camWhiteBalance == 1 ) {
	OPrimitiveControl_CameraParam wb(ocamparamWB_FL_MODE);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_WHITE_BALANCE,
					 &wb, sizeof (wb), 0, 0);
    } else {
	OPrimitiveControl_CameraParam wb(ocamparamWB_OUTDOOR_MODE);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_WHITE_BALANCE,
					 &wb, sizeof (wb), 0, 0);
    }
    if ( result != oSUCCESS ) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "Brain::SetCamCPCPrimitives(1)",
                  "OPENR::ControlPrimitive() FAILED", result));
    }
    if ( camShutterSpeed == 0 ) {
	OPrimitiveControl_CameraParam shutter(ocamparamSHUTTER_SLOW);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_SHUTTER_SPEED,
					 &shutter, sizeof (shutter), 0, 0);
    } else if ( camShutterSpeed == 1 ) {
	OPrimitiveControl_CameraParam shutter(ocamparamSHUTTER_MID);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_SHUTTER_SPEED,
					 &shutter, sizeof (shutter), 0, 0);
    } else {
	OPrimitiveControl_CameraParam shutter(ocamparamSHUTTER_FAST);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_SHUTTER_SPEED,
					 &shutter, sizeof (shutter), 0, 0);
    }
    if ( result != oSUCCESS ) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "Brain::SetCamCPCPrimitives(2)",
                  "OPENR::ControlPrimitive() FAILED", result));
    }
    if ( camGain == 0 ) {
	OPrimitiveControl_CameraParam gain(ocamparamGAIN_LOW);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_GAIN,
					 &gain, sizeof (gain), 0, 0);
    } else if ( camGain == 1 ) {
	OPrimitiveControl_CameraParam gain(ocamparamGAIN_MID);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_GAIN,
					 &gain, sizeof (gain), 0, 0);
    } else {
	OPrimitiveControl_CameraParam gain(ocamparamGAIN_HIGH);
	result = OPENR::ControlPrimitive(fbkID, oprmreqCAM_SET_GAIN,
					 &gain, sizeof (gain), 0, 0);
    }
    if ( result != oSUCCESS ) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "Brain::SetCamCPCPrimitives(3)",
                  "OPENR::ControlPrimitive() FAILED", result));
    }
}

// ------------------------------------------------------------------------
void
Brain::ReadyCommand(const OReadyEvent& event)
{
    // MoNetがコマンドを受付られる状態になった
    if ( brainState == BRAIN_START ) {
	MonetExecuteDirect(SLEEP2SLEEP_NULL);
        brainState = BRAIN_WAITING_RESULT;
    }
}

void
Brain::NotifyResult(const ONotifyEvent& event)
{
    // MoNetから処理結果を受け取った
    if ( brainState == BRAIN_IDLE ) {
        ; // do nothing
    } else if ( brainState == BRAIN_START ) {
        observer[event.ObsIndex()]->AssertReady();
    } else if ( brainState == BRAIN_WAITING_RESULT ) {
	HeadExecute( hcmdInitialize, 0, NULL );
	StartProgram( 0 );
        observer[event.ObsIndex()]->AssertReady();
    } else if ( brainState == BRAIN_MONET ) {
        if (commandQueue.size() > 0) {
            MonetExecuteDirect(commandQueue.front());
            commandQueue.pop_front();
	    waitingResult = true;
	} else {
	    waitingResult = false;
	}
        observer[event.ObsIndex()]->AssertReady();
    } else if ( brainState == BRAIN_STN ) {
        if (commandQueue.size() > 0) {
            MonetExecuteDirect(commandQueue.front());
            commandQueue.pop_front();
	    waitingResult = true;
	} else {
	    waitingResult = false;
	    if ( !ProcessProgram() ) {
		StopProgram();
	    }
	}
	observer[event.ObsIndex()]->AssertReady();
    }
}

void
Brain::NotifyObjInfo( const ONotifyEvent& event )
{
    // Visionからボール情報を受け取った
    if ( brainState == BRAIN_IDLE ) {
        ; // do nothing
    } else {
	CdtInfo *cdtInfo = (CdtInfo*) event.Data(0);
	perception.SetCdtInfo( cdtInfo );
	memory.SetCdtInfo( cdtInfo );
	logging.LogCdtInfo( cdtInfo );
        observer[event.ObsIndex()]->AssertReady();
    }
}

void
Brain::NotifySensor( const ONotifyEvent& event )
{
    // OVirtualRobotからセンサーデータを受け取った
    OSensorFrameVectorData* sensorVec = (OSensorFrameVectorData*)event.Data(0);

    sensor.InitCommonSensorIndex(sensorVec);
    if ( brainState == BRAIN_IDLE ) {
	return;
    }
    OSYSDEBUG(("Brain::NotifySensor\n"));
    if ( brainState == BRAIN_STN ) {
	sensor.SetSensor( sensorVec );
	observer[event.ObsIndex()]->AssertReady();
    }
}

void
Brain::NotifyControl( const ONotifyEvent& event )
{
    int size;

    // Networkから制御情報を受け取った
    if ( brainState == BRAIN_IDLE ) {
        ; // do nothing
    } else {
	ControlInfo *ci = (ControlInfo*) event.Data(0);
	ControlInfo result;
	result.id = ci->id;
	*result.sValue = '\0';
	if ( verboseMode >= 9 ) {
	    OSYSPRINT(( "Brain::NotifyControl %d,%d\n", ci->id, ci->iValue ));
	}
	switch ( ci->id ) {
	case CtID_Monet:
	    MonetExecute( ci->iValue );
	    break;
	case CtID_MonetSync:
	    if ( commandQueue.size() <= 0 ) {
		MonetExecute( ci->iValue );
	    }
	    break;
	case CtID_StartSTN:
	    if ( brainState == BRAIN_START
		 || brainState == BRAIN_WAITING_RESULT ) {
		OSYSPRINT(( "Brain::NotifyControl ignore start in BRAIN_START\n" ));
		break;
	    }
	    if ( isValidKT(ci) ) {
		stnHolder.SetKickoff( isKickoff(ci) );
		stnHolder.SetTeamColor( isRedTeam(ci) );
		if ( isWin(ci) ) {
		    stnHolder.SetScore( SCORE_WIN );
		} else if ( isLose(ci) ) {
		    stnHolder.SetScore( SCORE_LOSE );
		} else {
		    stnHolder.SetScore( SCORE_DRAW );
		}
	    }
	    StartProgram( ci->iValue );
	    break;
	case CtID_StopSTN:
	    StopProgram();
	    break;
	case CtID_ReadSTN:
	    StopProgram();
	    stnHolder.ClearAll();
	    stnHolder.ReadAll();
	    FaceExecute( fcmdEarMove1 );
	    break;
	case CtID_GetCurrentStateSTN:
	    *result.sValue = '\0';
	    if ( brainState == BRAIN_STN ) {
		STNInfo stnInfo;
		stnHolder.GetCurrentInfo( &stnInfo );
		char *buf = result.sValue;
		sprintf( buf, "%02d", stnInfo.stnNo );
		for ( int i = 0; i < MAX_TASKS; i ++ ) {
		    sprintf( &buf[ i * 2 + 2 ], "%02d", stnInfo.stateIds[i] );
		}
	    }
	    subject[sbjControlResult]->SetData( &result, sizeof(result) );
	    subject[sbjControlResult]->NotifyObservers();
	    break;
	case CtID_GetCdtInfo:
	    strncpy( result.sValue, perception.CdtInfoString(),
		     sizeof result.sValue );
	    subject[sbjControlResult]->SetData( &result, sizeof(result) );
	    subject[sbjControlResult]->NotifyObservers();
	    break;
	case CtID_GetCdt:
	    size = cdtfile.ToString( NULL );
	    if ( size < sizeof result.sValue ) {
		cdtfile.ToString( result.sValue );
	    } else {
		*result.sValue = '\0';
	    }
	    subject[sbjControlResult]->SetData( &result, sizeof(result) );
	    subject[sbjControlResult]->NotifyObservers();
	    break;
	case CtID_ReadCdt:
	    cdtfile.Read( CDT_CONFIG );
	    cdtfile.SetCdtVectorData( fbkID );
	    cdtfile.Print();
	    FaceExecute( fcmdEarMove1 );
	    break;
	case CtID_Head:
	    HeadingPosition next;
	    next.tilt  = ci->dValue2[0];
	    next.pan   = ci->dValue2[1];
	    next.tilt2 = ci->dValue2[2];
	    next.mouth = ci->dValue2[3];
	    next.speed = 1.0;
	    HeadExecute( ci->iValue, ci->i2Value, &next );
	    break;
	case CtID_ReadHead:
	    HeadExecute( hcmdReadHeadings, 0, NULL );
	    FaceExecute( fcmdEarMove1 );
	    break;
	case CtID_Face:
	    FaceExecute( ci->iValue );
	    break;
	case CtID_SetPose:
	    PoseData pose;

	    pose.frontLeft[0]  = ci->dValue2[0];
	    pose.frontLeft[1]  = ci->dValue2[1];
	    pose.frontLeft[2]  = ci->dValue2[2];
	    pose.rearLeft[0]   = ci->dValue2[3];
	    pose.rearLeft[1]   = ci->dValue2[4];
	    pose.rearLeft[2]   = ci->dValue2[5];
	    pose.frontRight[0] = ci->dValue2[6];
	    pose.frontRight[1] = ci->dValue2[7];
	    pose.frontRight[2] = ci->dValue2[8];
	    pose.rearRight[0]  = ci->dValue2[9];
	    pose.rearRight[1]  = ci->dValue2[10];
	    pose.rearRight[2]  = ci->dValue2[11];
	    subject[sbjPose]->SetData( &pose, sizeof(pose) );
	    subject[sbjPose]->NotifyObservers();
	    break;
	case CtID_Internal:
	    stnHolder.InternalExecute( ci->iValue );
	    break;
	case CtID_SetCamParam:
	    camWhiteBalance = ((ci->iValue / 100) % 10) % 3;
	    camShutterSpeed = ((ci->iValue / 10) % 10) % 3;
	    camGain = (ci->iValue % 10) % 3;
	    SetCamCPCPrimitives();
	    break;
	default:
	    OSYSPRINT(("Brain::NotifyControl invalid ID (%d)", ci->id ));
	    break;
	}
	observer[event.ObsIndex()]->AssertReady();
    }
}

void
Brain::NotifyMessage( const ONotifyEvent& event )
{
    if ( brainState == BRAIN_IDLE ) {
        ; // do nothing
    } else {
	MessageInfo *mi = (MessageInfo*) event.Data(0);
	if ( verboseMode >= 9 ) {
	    OSYSPRINT(( "Brain::NotifyMessage msgId=%d\n", mi->msgId ));
	}
	observer[event.ObsIndex()]->AssertReady();
	stnHolder.SetMessageNo( mi->msgId );
    }
}

void
Brain::StartProgram( int stnNo )
{
    if ( verboseMode >= 7 ) {
	OSYSPRINT(( "Brain::StartProgram <%02d>\n", stnNo ));
    }
    if ( verboseMode >= 8 ) {
	stnHolder.Print( stnNo );
    }
    sensor.Initialize();
    ClearLastCmds();
    perception.UpdateNowMSec( GetNowMSec() );
    observer[obsSensor]->AssertReady();
    if ( stnHolder.ColdStart( stnNo ) ) {
	MonetExecute( monetcommandID_UNDEF );
	brainState = BRAIN_STN;
    } else {
	OSYSPRINT(( "Brain::StartProgram can't execute\n" ));
	brainState = BRAIN_MONET;
    }
}

void
Brain::StopProgram()
{
    stnHolder.StopAll();
    if ( brainState == BRAIN_STN ) {
	HeadExecute( hcmdStop, 0, NULL );
	SendCurrentInfo();
    }
    brainState = BRAIN_MONET;
}

bool
Brain::ProcessProgram()
{
    STNCmds cmds;

    cmds.monetCmd = 0;
    cmds.headCmd = hcmdNone;
    cmds.faceCmd = fcmdNone;
    cmds.internalCmd = icmdNone;
    if ( !stnHolder.Process( &cmds ) ) {
	OSYSPRINT(( "Brain::ProcessProgram terminated\n" ));
	return false;
    }
    MonetExecute( cmds.monetCmd );
    lastCmds.monetCmd = cmds.monetCmd;
    if ( cmds.headCmd != hcmdNone ) {
	HeadExecute( cmds.headCmd, 0, NULL );
	lastCmds.headCmd = cmds.headCmd;
    }
    if ( cmds.faceCmd != fcmdNone ) {
	FaceExecute( cmds.faceCmd );
	lastCmds.faceCmd = cmds.faceCmd;
    }
    if ( cmds.internalCmd != icmdNone ) {
	InternalExecute( cmds.internalCmd );
	lastCmds.internalCmd = cmds.internalCmd;
    }
    SendCurrentInfo();
    STNInfo stnInfo;
    stnHolder.GetCurrentInfo( &stnInfo );
    logging.LogSTNInfo( &stnInfo );
    return true;
}

void
Brain::MonetExecute(MoNetCommandID cmdID)
{
    if ( waitingResult ) {
	commandQueue.push_back( cmdID );
    } else {
	MonetExecuteDirect( cmdID );
	waitingResult = true;
    }
}

void
Brain::MonetExecuteDirect(MoNetCommandID cmdID)
{
    if ( verboseMode >= 8 && cmdID != monetcommandID_UNDEF ) {
	OSYSPRINT(( "Brain::MonetExecuteDirect [%d]\n", cmdID ));
    }
    logging.LogMoNetCmd( cmdID );
    MoNetCommand cmd(cmdID);
    subject[sbjCommand]->SetData(&cmd, sizeof(cmd));
    subject[sbjCommand]->NotifyObservers();
    memory.MoNetExecute( cmdID );
}

void
Brain::HeadExecute( int hcmd, int iValue, const HeadingPosition *hpos )
{
    if ( verboseMode >= 8 ) {
	OSYSPRINT(( "Brain::HeadExecute [%d]\n", hcmd ));
    }
    VCmdInfo vCmdInfo;
    vCmdInfo.hcmd = hcmd;
    vCmdInfo.iValue = iValue;
    // set heading position
    vCmdInfo.hpos.tilt = 0;
    vCmdInfo.hpos.tilt2 = 0;
    vCmdInfo.hpos.mouth = -3;
    vCmdInfo.hpos.speed = 1.0;
    if ( hcmd == hcmdSearchBall || hcmd == hcmdFaceToBall ) {
	vCmdInfo.hpos.pan = - memory.GetTheta(IO_BALL);
    } else if ( hcmd == hcmdSearchGoalB ) {
	vCmdInfo.hpos.pan = - memory.GetTheta(IO_GOAL_B);
    } else if ( hcmd == hcmdSearchGoalY ) {
	vCmdInfo.hpos.pan = - memory.GetTheta(IO_GOAL_Y);
    } else if ( hcmd == hcmdSearchBeaconBY ) {
	vCmdInfo.hpos.pan = - memory.GetTheta(IO_BEACON_BY);
    } else if ( hcmd == hcmdSearchBeaconYB ) {
	vCmdInfo.hpos.pan = - memory.GetTheta(IO_BEACON_YB);
    } else if ( hpos ) {
	vCmdInfo.hpos  = *hpos;
    } else {
	memset( &vCmdInfo.hpos, 0, sizeof vCmdInfo.hpos );
    }
    vCmdInfo.seqno = ++ hcmdSeqno;
    subject[sbjVCommand]->SetData((char*) &vCmdInfo, sizeof vCmdInfo );
    subject[sbjVCommand]->NotifyObservers();
}

void
Brain::FaceExecute( int fcmd )
{
    if ( verboseMode >= 9 ) {
	OSYSPRINT(( "Brain::FaceExecute [%d]\n", fcmd ));
    }
    subject[sbjFCommand]->SetData((char*) &fcmd, sizeof fcmd );
    subject[sbjFCommand]->NotifyObservers();
}

void
Brain::InternalExecute( int icmd )
{
    if ( icmd >= icmdMessage0 && icmd <= icmdMessage7 ) {
	MessageInfo mi;
	memset( &mi, 0, sizeof mi );
	mi.msgId = icmd - icmdMessage0;
	if ( verboseMode >= 8 ) {
	    OSYSPRINT(( "Brain::InternalExecute [%d]\n", icmd ));
	}
	subject[sbjMessage]->SetData((char*) &mi, sizeof mi );
	subject[sbjMessage]->NotifyObservers();
    }
}

void
Brain::SendCurrentInfo()
{
    STNInfo stnInfo;
    memset( &stnInfo, 0, sizeof stnInfo );
    stnHolder.GetCurrentInfo( &stnInfo );
    stnInfo.cmds = lastCmds;
    subject[sbjSTNInfo]->SetData(&stnInfo, sizeof(stnInfo));
    subject[sbjSTNInfo]->NotifyObservers();
}

void
Brain::ConfigFromString( const char *buf )
{
    int watchDogSec;
    int temp;

    while ( buf && *buf ) {
	if ( *buf == '#' ) {
	    // skip comment
	} else if ( !memcmp( buf, "STN ", 4 ) ) {
	    sscanf( &buf[4], "%d", &watchDogSec );
	    stnHolder.SetWatchDogSec( watchDogSec );
	} else if ( !memcmp( buf, "LOG ", 4 ) ) {
	    sscanf( &buf[4], "%d", &temp );
	    logging.SetMaxLogSize( temp );
	} else if ( !memcmp( buf, "LOGMASK ", 8 ) ) {
	    sscanf( &buf[8], "%x", &temp );
	    logging.SetLogMask( temp );
	} else if ( !memcmp( buf, "MEMDEFAULT ", 11 ) ) {
	    sscanf( &buf[11], "%d", &temp );
	    memory.SetDefaultTimeout( temp );
	} else if ( !memcmp( buf, "MEMBALL ", 8 ) ) {
	    sscanf( &buf[8], "%d", &temp );
	    memory.SetBallTimeout( temp );
	} else if ( !memcmp( buf, "CAMWB ", 6 ) ) {
	    sscanf( &buf[6], "%d", &temp );
	    camWhiteBalance = temp % 3;
	} else if ( !memcmp( buf, "CAMSS ", 6 ) ) {
	    sscanf( &buf[6], "%d", &temp );
	    camShutterSpeed = temp % 3;
	} else if ( !memcmp( buf, "CAMGA ", 6 ) ) {
	    sscanf( &buf[6], "%d", &temp );
	    camGain = temp % 3;
	}
	buf = strchr( buf, '\n' );
	if ( buf ) {
	    buf ++;
	}
    }
}

bool
Brain::ReadDefartConfig( const char *path )
{
    FILE* fp = fopen( path, "r" );
    if ( fp == NULL ) {
        return false;
    }
    OSYSPRINT(( "Brain::ReadDefartConfig path=%s.\n", path ));

    fseek( fp, 0L, SEEK_END );
    int size = ftell( fp );
    fseek( fp, 0L, SEEK_SET );
    char *filebuf = new char[ size + 2 ];
    fread( filebuf, size, 1, fp );
    filebuf[ size ] = '\n';
    filebuf[ size + 1 ] = 0;
    fclose( fp );
    ConfigFromString( filebuf );
    delete[] filebuf;

    return true;
}

int
Brain::GetHcmdSeqno() const
{
    return hcmdSeqno;
}

void
Brain::ClearLastCmds()
{
    lastCmds.monetCmd = 0;
    lastCmds.headCmd = hcmdNone;
    lastCmds.faceCmd = fcmdNone;
    lastCmds.internalCmd = icmdNone;
}
