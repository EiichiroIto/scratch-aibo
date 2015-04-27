//
// Copyright 2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/core_macro.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <UDPEndpointMsg.h>
#include <SystemTime.h>
#include "UDPBeacon.h"
#include "entry.h"

#define systemTimeToMilliSeconds(x) ((x).seconds*1000+(x).useconds/1000)

int
GetNowMSec()
{
    SystemTime current;
    GetSystemTime( &current );
    return systemTimeToMilliSeconds(current);
}

UDPBeacon::UDPBeacon() : udpBeaconState(USS_IDLE),
			 initSensorIndex(false),
			 latestSensorRegion(0),
			 sensorCount(0)
{
#ifdef DEFART
    memset( &cdtInfo, 0, sizeof cdtInfo );
    memset( &stnInfo, 0, sizeof stnInfo );
    stnInfo.stnNo = -1;
#endif /* DEFART */
    for ( int i = 0; i < NUM_JOINTS; i++ ) {
	jointID[i] = oprimitiveID_UNDEF;
	sensorIndex[i] = -1;
    }
}

OStatus
UDPBeacon::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("UDPBeacon::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

    //OPENR::SetMotorPower(opowerON);

    OpenPrimitives();

    ipstackRef = antStackRef("IPStack");

    for (int index = 0; index < UDPBEACON_CONNECTION_MAX; index++) {
        OStatus result = InitUDPBuffer(index);
        if (result != oSUCCESS) return oFAIL;
    }

    return oSUCCESS;
}

OStatus
UDPBeacon::DoStart(const OSystemEvent& event)
{
    OStatus result;
    OSYSDEBUG(("UDPBeacon::DoStart()\n"));

    for (int index = 0; index < UDPBEACON_CONNECTION_MAX; index++) {
        // 
        // Bind
        //
        result = CreateUDPEndpoint(index);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPBeacon::DoStart()",
                  "CreateUDPEndpoint() fail", index));
            continue;
        }
        result = Bind(index);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPBeacon::DoStart()",
                  "Bind() fail", index));
            continue;
        }
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    udpBeaconState = USS_START;

    return oSUCCESS;
}    

OStatus
UDPBeacon::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("UDPBeacon::DoStop()\n"));

    udpBeaconState = USS_IDLE;

    for (int index = 0; index < UDPBEACON_CONNECTION_MAX; index++) {
        if ( connection[index].state != CONNECTION_CLOSED &&
	     connection[index].state != CONNECTION_CLOSING) {

            // Connection close
            UDPEndpointCloseMsg closeMsg(connection[index].endpoint);
            closeMsg.Call(ipstackRef, sizeof(closeMsg));
            connection[index].state = CONNECTION_CLOSED;
        }
    }

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return oSUCCESS;
}

OStatus
UDPBeacon::DoDestroy(const OSystemEvent& event)
{
    OSYSDEBUG(("UDPBeacon::DoDestroy\n"));

    for ( int index = 0; index < UDPBEACON_CONNECTION_MAX; index++ ) {
	connection[index].sendBuffer.UnMap();
	antEnvDestroySharedBufferMsg destroySendBufferMsg(connection[index].sendBuffer);
	destroySendBufferMsg.Call(ipstackRef, sizeof(destroySendBufferMsg));
    }

    DELETE_ALL_SUBJECT_AND_OBSERVER;

    return oSUCCESS;
}

OStatus
UDPBeacon::Send(int index)
{
    OSYSDEBUG(("UDPBeacon::Send()\n"));

    if ( connection[index].sendSize == 0 ||
	 connection[index].state != CONNECTION_CLOSED ) {
	return oFAIL;
    }

    UDPEndpointSendMsg sendMsg( connection[index].endpoint,
				connection[index].sendAddress,
				connection[index].sendPort,
				connection[index].sendData,
				connection[index].sendSize );
    sendMsg.continuation = (void*)index;
    sendMsg.Send( ipstackRef, myOID_,
		  Extra_Entry[entrySendCont], sizeof(UDPEndpointSendMsg) );

    connection[index].state = CONNECTION_SENDING;
    return oSUCCESS;
}

void
UDPBeacon::SendCont(ANTENVMSG msg)
{
    OSYSDEBUG(("UDPBeacon::SendCont()\n"));

    UDPEndpointSendMsg* sendMsg = (UDPEndpointSendMsg*) antEnvMsg::Receive(msg);
    int index = (int)(sendMsg->continuation);
    if ( connection[index].state == CONNECTION_CLOSED ) {
        return;
    }

    if (sendMsg->error != UDP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "UDPBeacon::SendCont()",
                  "FAILED. sendMsg->error", sendMsg->error));
        Close(index);
        return;
    }
    char in_buff[64];
//    OSYSDEBUG(("sendData : %s", connection[index].sendData));
    OSYSDEBUG(("sendAddress : %s\n", connection[index].sendAddress.GetAsString(in_buff)));
    OSYSDEBUG(("sendPort : %d\n\n", connection[index].sendPort));

    Close(index);
}

OStatus
UDPBeacon::Close(int index)
{
    OSYSDEBUG(("UDPBeacon::Close()\n"));

    if ( connection[index].state == CONNECTION_CLOSED ||
	 connection[index].state == CONNECTION_CLOSING ) {
	return oFAIL;
    }

    UDPEndpointCloseMsg closeMsg( connection[index].endpoint );
    closeMsg.continuation = (void*)index;
    closeMsg.Send( ipstackRef, myOID_,
		   Extra_Entry[entryCloseCont], sizeof(closeMsg));

    connection[index].state = CONNECTION_CLOSING;

    return oSUCCESS;
}

void
UDPBeacon::CloseCont(ANTENVMSG msg)
{
    OStatus result;
    OSYSDEBUG(("UDPBeacon::CloseCont()\n"));

    UDPEndpointCloseMsg* closeMsg = (UDPEndpointCloseMsg*)antEnvMsg::Receive(msg);
    int index = (int)(closeMsg->continuation);
    if ( connection[index].state == CONNECTION_CLOSED) {
        return;
    }

    connection[index].state = CONNECTION_CLOSED;

    result = CreateUDPEndpoint(index);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPBeacon::CloseCont()",
                  "CreateUDPEndpoint() fail", index));
        return;
    }
    result = Bind(index);
    if (result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d]",
                  "UDPBeacon::CloseCont()",
                  "Bind() fail", index));
    }
}

OStatus
UDPBeacon::InitUDPBuffer(int index)
{
    OSYSDEBUG(("UDPBeacon::InitUDPBuffer()\n"));

    connection[index].state = CONNECTION_CLOSED;

    // 
    // Allocate send buffer
    //
    antEnvCreateSharedBufferMsg sendBufferMsg(UDPBEACON_BUFFER_SIZE);

    sendBufferMsg.Call(ipstackRef, sizeof(sendBufferMsg));
    if (sendBufferMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "UDPBeacon::InitUDPBuffer()",
                  "Can't allocate send buffer",
                  index, sendBufferMsg.error));
        return oFAIL;
    }

    connection[index].sendBuffer = sendBufferMsg.buffer;
    connection[index].sendBuffer.Map();
    connection[index].sendData
        = (byte*)(connection[index].sendBuffer.GetAddress());

    return oSUCCESS;
}

OStatus
UDPBeacon::CreateUDPEndpoint(int index)
{
    OSYSDEBUG(("UDPBeacon::CreateUDPEndpoint()\n"));

    if ( connection[index].state != CONNECTION_CLOSED ) {
	return oFAIL;
    }

    //
    // Create UDP endpoint
    //
    antEnvCreateEndpointMsg udpCreateMsg(EndpointType_UDP,
                                         UDPBEACON_BUFFER_SIZE * 2);
    udpCreateMsg.Call(ipstackRef, sizeof(udpCreateMsg));
    if (udpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s[%d] antError %d",
                  "UDPBeacon::CreateUDPEndpoint()",
                  "Can't create endpoint",
                  index, udpCreateMsg.error));
        return oFAIL;
    }
    connection[index].endpoint = udpCreateMsg.moduleRef;

    return oSUCCESS;
}

OStatus
UDPBeacon::Bind(int index)
{
    OSYSDEBUG(("UDPBeacon::Bind()\n"));

    if ( connection[index].state != CONNECTION_CLOSED ) {
	return oFAIL;
    }

    // 
    // Bind
    //
    UDPEndpointBindMsg bindMsg( connection[index].endpoint, 
				IP_ADDR_ANY, UDPBEACON_PORT );
    bindMsg.Call( ipstackRef,sizeof(antEnvCreateEndpointMsg) );
    if ( bindMsg.error != UDP_SUCCESS ) {
        return oFAIL;
    }

    return oSUCCESS;
}

void
UDPBeacon::OpenPrimitives()
{
    OSYSDEBUG(("UDPBeacon::OpenPrimitives\n"));

    for ( int i = 0; i < NUM_JOINTS; i++ ) {
        OStatus result = OPENR::OpenPrimitive( JOINT_LOCATOR[i],
					       &jointID[i]);
        if (result != oSUCCESS) {
            OSYSLOG1(( osyslogERROR, "UDPBeacon::OpenPrimitives failed(%s)\n",
		       JOINT_LOCATOR[i] ));

        }
    }
}

void 
UDPBeacon::InitSensorIndex(OSensorFrameVectorData* sensorVec)
{
    OSYSDEBUG(("UDPBeacon::InitSensorIndex\n"));

    for ( int i = 0; i < NUM_JOINTS; i++ ) {
	for ( int j = 0; j < sensorVec->vectorInfo.numData; j++ ) {
            OSensorFrameInfo* info = sensorVec->GetInfo(j);
	    if ( info->primitiveID == jointID[i] ) {
		sensorIndex[i] = j;
		OSYSDEBUG(("[%2d] %s\n",
			   sensorIndex[i], JOINT_LOCATOR[i]));
		break;
	    }
	}
    }
}

void
UDPBeacon::NotifySensor(const ONotifyEvent& event)
{
    if ( udpBeaconState == USS_IDLE ) {
	return;
    }

    RCRegion* rgn = event.RCData(0);
    rgn->AddReference();
    if ( initSensorIndex == false ) {
	OSensorFrameVectorData* sensorVec
	    = (OSensorFrameVectorData*)rgn->Base();
	InitSensorIndex(sensorVec);
	initSensorIndex = true;
    }

    if ( latestSensorRegion != 0 ) {
	latestSensorRegion->RemoveReference() ;
    }
    latestSensorRegion = rgn;

    observer[event.ObsIndex()]->AssertReady();

    int msec = GetNowMSec();

    // make counting bit randomly
    if ( ++ sensorCount < 32 + ((msec / 3) % 2) ) {
	return;
    }

    OSYSDEBUG(("UDPBeacon::NotifySensor(), sensorCount=%d\n",sensorCount));

    int index = 0;
    char tmpbuf[32];
    DefartBroadCastData sendPacket;

    memset( &sendPacket, ' ', sizeof sendPacket );
    sprintf( tmpbuf, "%08x", msec );
    memcpy( sendPacket.msec, tmpbuf, sizeof sendPacket.msec );
    SetupDefartData( &sendPacket );

    //
    // Send back the message
    //
    connection[index].sendAddress = IP_ADDR_BROADCAST;
    connection[index].sendPort    = UDPBEACON_PORT;
    connection[index].sendSize    = sizeof sendPacket;
    memcpy( connection[index].sendData, &sendPacket, sizeof sendPacket );

    if ( Send(index) != oFAIL ) {
	sensorCount = 0;
    }
}

#ifdef DEFART
void
UDPBeacon::SetupDefartData( DefartBroadCastData *data )
{
    memcpy( data->magic, DEFART_MAGIC, sizeof data->magic );
    memcpy( data->aibotag, AIBO_TAG, sizeof data->aibotag );
    data->dummy1[0] = data->dummy2[0] = data->dummy3[0]
	= data->dummy4[0] = data->dummy5[0] = data->dummy6[0]
	= data->dummy7[0] = data->dummy8[0]
	= ':';

    FillFoundInfo( &data->ball,     "BL", &cdtInfo.foundInfo[IO_BALL] );
    FillFoundInfo( &data->goalB,    "GB", &cdtInfo.foundInfo[IO_GOAL_B] );
    FillFoundInfo( &data->goalY,    "GY", &cdtInfo.foundInfo[IO_GOAL_Y] );
    FillFoundInfo( &data->beaconBY, "BY", &cdtInfo.foundInfo[IO_BEACON_BY] );
    FillFoundInfo( &data->beaconYB, "YB", &cdtInfo.foundInfo[IO_BEACON_YB] );

    FillSTNInfo( &data->stnInfo, "ST", &stnInfo );

    FillSensorInfo( &data->sensor, "SE" );
}

void
UDPBeacon::FillFoundInfo( DefartFoundInfo *ptr, const char *tag,
			  const FoundInfo *foundInfo )
{
    char tmpbuf[ 16 ];

    memcpy( ptr->tag, tag, sizeof ptr->tag );
    ptr->found[0] = foundInfo->found ? '1' : '0' ;
    sprintf( tmpbuf, "%03d", foundInfo->objPos.x );
    memcpy( ptr->x, tmpbuf, sizeof ptr->x );
    sprintf( tmpbuf, "%03d", foundInfo->objPos.y );
    memcpy( ptr->y, tmpbuf, sizeof ptr->y );
    sprintf( tmpbuf, "%03d", foundInfo->objPos.w );
    memcpy( ptr->w, tmpbuf, sizeof ptr->w );
    sprintf( tmpbuf, "%03d", foundInfo->objPos.h );
    memcpy( ptr->h, tmpbuf, sizeof ptr->h );
    sprintf( tmpbuf, "%06.1f", foundInfo->objPos.pan );
    memcpy( ptr->pan, tmpbuf, sizeof ptr->pan );
    sprintf( tmpbuf, "%06.1f", foundInfo->objPos.tilt );
    memcpy( ptr->tilt, tmpbuf, sizeof ptr->tilt );
    sprintf( tmpbuf, "%03d", foundInfo->distance % 1000 );
    memcpy( ptr->distance, tmpbuf, sizeof ptr->distance );
}

void
UDPBeacon::FillSTNInfo( DefartSTNInfo *ptr, const char *tag,
			const STNInfo *stnInfo )
{
    char tmpbuf[ 16 ];

    memcpy( ptr->tag, tag, sizeof ptr->tag );
    sprintf( tmpbuf, "%02d", stnInfo->stnNo );
    memcpy( ptr->stnNo, tmpbuf, sizeof ptr->stnNo );
    for ( int i = 0; i < MAX_TASKS; i ++ ) {
	sprintf( tmpbuf, "%02d", stnInfo->stateIds[i] );
	memcpy( ptr->stateIds[i], tmpbuf, 2 );
    }
    sprintf( tmpbuf, "%03d", stnInfo->timer1Sec );
    memcpy( ptr->timer1, tmpbuf, 3 );
    sprintf( tmpbuf, "%03d", stnInfo->timer2Sec );
    memcpy( ptr->timer2, tmpbuf, 3 );
    sprintf( tmpbuf, "%03d", stnInfo->counter1 );
    memcpy( ptr->counter1, tmpbuf, 3 );
    sprintf( tmpbuf, "%03d", stnInfo->counter2 );
    memcpy( ptr->counter2, tmpbuf, 3 );
    sprintf( tmpbuf, "%03d", stnInfo->counter3 );
    memcpy( ptr->counter3, tmpbuf, 3 );

    sprintf( tmpbuf, "%02d", stnInfo->cmds.faceCmd );
    memcpy( ptr->face, tmpbuf, 2 );
    sprintf( tmpbuf, "%03d", stnInfo->cmds.headCmd );
    memcpy( ptr->head, tmpbuf, 3 );
    sprintf( tmpbuf, "%04d", stnInfo->cmds.monetCmd );
    memcpy( ptr->monet, tmpbuf, 4 );
    sprintf( tmpbuf, "%02d", stnInfo->cmds.internalCmd );
    memcpy( ptr->internal, tmpbuf, 2 );
}

void
UDPBeacon::FillSensorInfo( DefartSensorInfo *ptr, const char *tag )
{
    char tmpbuf[ 16 ];

    memcpy( ptr->tag, tag, sizeof ptr->tag );
    OSensorFrameVectorData* sensorVec
        = (OSensorFrameVectorData*)latestSensorRegion->Base();

    int sIndex;
    OSensorFrameInfo* sinfo;
    OSensorFrameData* sdata;

    sIndex = sensorIndex[BODY_PSD];
    sinfo = sensorVec->GetInfo( sIndex );
    sdata = sensorVec->GetData( sIndex );
    sprintf( tmpbuf, "%6d", sdata->frame[sinfo->numFrames - 1].value );
    memcpy( ptr->bodyPSD, tmpbuf, sizeof ptr->bodyPSD );

    sIndex = sensorIndex[ACC_FR];
    sinfo = sensorVec->GetInfo( sIndex );
    sdata = sensorVec->GetData( sIndex );
    sprintf( tmpbuf, "%9d", sdata->frame[sinfo->numFrames - 1].value );
    memcpy( ptr->accFR, tmpbuf, sizeof ptr->accFR );

    sIndex = sensorIndex[ACC_LR];
    sinfo = sensorVec->GetInfo( sIndex );
    sdata = sensorVec->GetData( sIndex );
    sprintf( tmpbuf, "%9d", sdata->frame[sinfo->numFrames - 1].value );
    memcpy( ptr->accLR, tmpbuf, sizeof ptr->accLR );

    sIndex = sensorIndex[ACC_UD];
    sinfo = sensorVec->GetInfo( sIndex );
    sdata = sensorVec->GetData( sIndex );
    sprintf( tmpbuf, "%9d", sdata->frame[sinfo->numFrames - 1].value );
    memcpy( ptr->accUD, tmpbuf, sizeof ptr->accUD );
}
#endif /* DEFART */

void
UDPBeacon::SetupTetoriData( TetoriBroadCastData *data )
{
    memcpy( data->magic, TETORI_MAGIC, sizeof data->magic );
    memcpy( data->aibotag, AIBO_TAG, sizeof data->aibotag );
    data->dummy1[0] = ':';
    data->dummy2[0] = ':';
    data->dummy3[0] = ':';
    data->dummy4[0] = ':';
    data->dummy5[0] = ':';

    OSensorFrameVectorData* sensorVec
        = (OSensorFrameVectorData*)latestSensorRegion->Base();

    FillLegData( &data->lf, "LF", sensorVec,
		 sensorIndex[LFLEG_J1],
		 sensorIndex[LFLEG_J2],
		 sensorIndex[LFLEG_J3] );

    FillLegData( &data->lr, "LR", sensorVec,
		 sensorIndex[LRLEG_J1],
		 sensorIndex[LRLEG_J2],
		 sensorIndex[LRLEG_J3] );

    FillLegData( &data->rf, "RF", sensorVec,
		 sensorIndex[RFLEG_J1],
		 sensorIndex[RFLEG_J2],
		 sensorIndex[RFLEG_J3] );

    FillLegData( &data->rr, "RR", sensorVec,
		 sensorIndex[RRLEG_J1],
		 sensorIndex[RRLEG_J2],
		 sensorIndex[RRLEG_J3] );
}

void
UDPBeacon::FillLegData( TetoriLegData *ptr, const char *tag,
			OSensorFrameVectorData* sensorVec,
			int j1SensorIdx, int j2SensorIdx, int j3SensorIdx )
{
    OSensorFrameInfo* sinfo;
    OSensorFrameData* sdata;
    slongword mrad;
    char tmpbuf[16];

    memcpy( ptr->tag, tag, sizeof ptr->tag );

    sinfo = sensorVec->GetInfo( j1SensorIdx );
    sdata = sensorVec->GetData( j1SensorIdx );
    mrad = sdata->frame[sinfo->numFrames - 1].value;
    sprintf( tmpbuf, "%6.1f", degrees( mrad/1000000.0 ) );
    memcpy( ptr->j1, tmpbuf, sizeof ptr->j1 );

    sinfo = sensorVec->GetInfo( j2SensorIdx );
    sdata = sensorVec->GetData( j2SensorIdx );
    mrad = sdata->frame[sinfo->numFrames - 1].value;
    sprintf( tmpbuf, "%6.1f", degrees( mrad/1000000.0 ) );
    memcpy( ptr->j2, tmpbuf, sizeof ptr->j2 );

    sinfo = sensorVec->GetInfo( j3SensorIdx );
    sdata = sensorVec->GetData( j3SensorIdx );
    mrad = sdata->frame[sinfo->numFrames - 1].value;
    sprintf( tmpbuf, "%6.1f", degrees( mrad/1000000.0 ) );
    memcpy( ptr->j3, tmpbuf, sizeof ptr->j3 );
#if 0
    OSYSPRINT(("[%s:%d] %d %d %d %d\n", tag, j3SensorIdx,
               sdata->frame[0].value, sdata->frame[1].value,
               sdata->frame[2].value, sdata->frame[3].value));
#endif
}

#ifdef DEFART
void
UDPBeacon::NotifyObjInfo( const ONotifyEvent& event )
{
    // Visionからボール情報を受け取った
    //OSYSPRINT(("NetworkUdp::NotifyObjInfo\n"));
    cdtInfo = *(CdtInfo*) event.Data(0);
    observer[event.ObsIndex()]->AssertReady();
}

void
UDPBeacon::NotifySTNInfo( const ONotifyEvent& event )
{
    // BrainからSTN情報を受け取った
    stnInfo = *(STNInfo*) event.Data(0);
    //OSYSPRINT(("NetworkUdp::NotifySTNInfo(%d)\n", stnInfo.stnNo));
    observer[event.ObsIndex()]->AssertReady();
}
#endif /* DEFART */
