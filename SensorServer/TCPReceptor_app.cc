//
// Copyright 2006 (C) Eiichiro ITO, GHC02331@nifty.com
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
// Eiichiro ITO, 1 Jun 2006
// mailto: GHC02331@nifty.com
//
// 2006.6.1 Originally Created.
//

#include <string.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <OPENR/core_macro.h>
#include "TCPReceptor.h"
#include "VisionInfo.h"
#include "command.h"
#include "parse.h"

void
TCPReceptor::ParseMessage(const char *str)
{
    OSYSPRINT(( "ParseMessage(%s)\n", str ));
    if ( strnicmp(str, "broadcast ", 10) == 0 ) {
      OSYSPRINT(( "broadcast=%s\n", &str[10] ));
      ParseBroadcast(&str[10]);
    } else if ( strnicmp(str, "sensor-update ", 14) == 0 ) {
      OSYSPRINT(( "sensor update=%s\n", &str[14] ));
      ParseSensorUpdate(&str[14]);
    } else if ( strnicmp(str, "peer-name ", 10) == 0 ) {
      OSYSPRINT(( "peer name=%s\n", &str[10] ));
    } else {
      OSYSPRINT(( "Unknown message\n" ));
    }
}

void
TCPReceptor::ParseBroadcast(const char *str)
{
  const char *p = str;
  Token t;
  int ret = parse_string(&p, &t);
  const char *broadcast = t.contents();
  OSYSPRINT(( "ret=%d, token=%s.\n", ret, broadcast ));
  if ( !stricmp( broadcast, "forward" ) ) {
    SendMoNet(5);
  } else if ( !stricmp( broadcast, "back" ) ) {
    SendMoNet(32);
  } else if ( !stricmp( broadcast, "left" ) ) {
    SendMoNet(75);
  } else if ( !stricmp( broadcast, "right" ) ) {
    SendMoNet(76);
  } else if ( !stricmp( broadcast, "search" ) ) {
    SendHead(hcmdSearchBall, 0);
  }
}

void
TCPReceptor::ParseSensorUpdate(const char *str)
{
  const char *p = str;
  int ret;
  do {
    Token tname;
    ret = parse_string(&p, &tname);
    if ( !ret ) {
      return;
    }
    Token tval;
    ret = parse_string(&p, &tval);
    OSYSPRINT(( "name=%s, val=%s.\n",
		tname.contents(), tval.contents() ));
  } while ( ret );
}

void
TCPReceptor::SendMoNet(int monet)
{
    ControlInfo ci;

    ci.id = CtID_MonetSync;
    ci.iValue = monet;
    subject[sbjControl]->SetData( &ci, sizeof(ci) );
    subject[sbjControl]->NotifyObservers();
}

void
TCPReceptor::SendHead(int hcmd, int arg)
{
    ControlInfo ci;

    ci.id = CtID_Head;
    ci.iValue = hcmd;
    ci.i2Value = arg;
    ci.dValue2[0] = 0;
    ci.dValue2[1] = 0;
    ci.dValue2[2] = 0;
    ci.dValue2[3] = 0;
    subject[sbjControl]->SetData( &ci, sizeof(ci) );
    subject[sbjControl]->NotifyObservers();
}

#if 0
void
TCPReceptor::ProcessReceivedMessage( int index, const byte* buf, int size )
{
    ControlInfo ci;
    char tmpbuf[32];

    OSYSDEBUG(("TCPReceptor::ProcessReceivedMessage code=[%-2.2s]\n", buf));

    memset( &ci, 0, sizeof ci );
    if ( !memcmp( buf, "MO", 2 ) ) {
	ci.id = CtID_Monet;
	ci.iValue = from4digits( (char*) &buf[2] );
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "MS", 2 ) ) {
	ci.id = CtID_MonetSync;
	ci.iValue = from4digits( (char*) &buf[2] );
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "ST", 2 ) ) {
	ci.id = CtID_StartSTN;
	ci.iValue = from2digits( (char*) &buf[2] );
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "SB", 2 ) ) {
	ci.id = CtID_StopSTN;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "SR", 2 ) ) {
	ci.id = CtID_ReadSTN;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "SS", 2 ) ) {
	ci.id = CtID_GetCurrentStateSTN;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	connectionIndex = index;
    } else if ( !memcmp( buf, "GC", 2 ) ) {
	imageType = IMAGE_LAYER_C;
	connectionIndex = index;
        observer[obsImage]->AssertReady();
    } else if ( !memcmp( buf, "GL", 2 ) ) {
	imageType = IMAGE_LAYER_L;
	connectionIndex = index;
        observer[obsImage]->AssertReady();
    } else if ( !memcmp( buf, "GM", 2 ) ) {
	imageType = IMAGE_LAYER_M;
	connectionIndex = index;
        observer[obsImage]->AssertReady();
    } else if ( !memcmp( buf, "GI", 2 ) ) {
	ci.id = CtID_GetCdtInfo;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	connectionIndex = index;
    } else if ( !memcmp( buf, "CG", 2 ) ) {
	ci.id = CtID_GetCdt;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	connectionIndex = index;
    } else if ( !memcmp( buf, "CR", 2 ) ) {
	ci.id = CtID_ReadCdt;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "HD", 2 ) ) {
	// 0123456789012345678901234
	// HDXXX,XXX,-XX,-XX,-XX,-XX
	ci.id = CtID_Head;
	char arg[ 4 ];
	memcpy( arg, &buf[2], 3 );  arg[3] = '\0'; // XXX(Command)
	ci.iValue = atoi(arg);
	memcpy( arg, &buf[6], 3 );  arg[3] = '\0'; // XXX(Arg1)
	ci.i2Value = atoi(arg);
	memcpy( arg, &buf[10], 3 );  arg[3] = '\0'; // -XX(TILT)
	ci.dValue2[0] = atof(arg);
	memcpy( arg, &buf[14], 3 ); arg[3] = '\0'; // -XX(PAN)
	ci.dValue2[1] = atof(arg);
	memcpy( arg, &buf[18], 3 ); arg[3] = '\0'; // -XX(TILT2)
	ci.dValue2[2] = atof(arg);
	memcpy( arg, &buf[22], 3 ); arg[3] = '\0'; // -XX(MOUTH)
	ci.dValue2[3] = atof(arg);
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "HR", 2 ) ) {
	ci.id = CtID_ReadHead;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "FC", 2 ) ) {
	ci.id = CtID_Face;
	ci.iValue = from2digits( (char*) &buf[2] );
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "SP", 2 ) ) {
	// SP-XXX.XX,-XXX.XX,-XXX.XX,-XXX.XX,-XXX.XX,-XXX.XX,\\
	// -XXX.XX,-XXX.XX,-XXX.XX,-XXX.XX,-XXX.XX,-XXX.XX,
	ci.id = CtID_SetPose;
	const byte *ptr = &buf[2];
	char arg[10];
	for ( int i = 0; i < 12; i += 1 ) {
	    memcpy( arg, ptr, 7 );  // -XXX.XX
	    arg[7] = 0;
	    ci.dValue2[i] = atof(arg);
	    ptr += 8;
	}
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "SC", 2 ) ) {
	ci.id = CtID_ClearSTN;
	ci.iValue = from2digits( (char*) &buf[2] );
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "SA", 2 ) ) {
	ci.id = CtID_AddSTN;
	ci.iValue = from2digits( (char*) &buf[2] );
	strncpy( ci.sValue, (char*) &buf[4], sizeof ci.sValue );
	char *p = strchr( ci.sValue, '$' );
	if ( p ) {
	    *p = 0;
	}
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "CS", 2 ) ) {
	ci.id = CtID_SendCdt;
	strncpy( ci.sValue, (char*) &buf[2], sizeof ci.sValue );
	char *p = strchr( ci.sValue, '$' );
	if ( p ) {
	    *p = 0;
	}
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "CW", 2 ) ) {
	ci.id = CtID_WriteCdt;
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "IN", 2 ) ) {
	ci.id = CtID_Internal;
	ci.iValue = from2digits( (char*) &buf[2] );
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "CP", 2 ) ) {
	ci.id = CtID_SetCamParam;
	ci.iValue = from4digits( (char*) &buf[2] );
	subject[sbjControl]->SetData( &ci, sizeof(ci) );
	subject[sbjControl]->NotifyObservers();
	SendOkWithTagString( index, "OK", "" );
    } else if ( !memcmp( buf, "$$", 2 ) ) {
	connection[index].receiveNext = false;
	SendOkWithTagString( index, "OK", "" );
    } else {
	OSYSPRINT(( "%s unknown command: (%-10.10s)\n",
		    "TCPReceptor::ProcessReceivedMessage",
		    buf ));

	SendOkWithTagString( index, "NG", "" );
    }
}
#endif

void
TCPReceptor::NotifyObjInfo( const ONotifyEvent& event )
{
  const int count_max = 10;
  static int count = count_max;
  char tmpbuf[128];

  // Visionから制御情報を受け取った
  if ( tcpReceptorState == TCPRS_IDLE ) {
    return;
  }
  if ( count -- < 0 ) {
    count = count_max;
    const CdtInfo *cdtInfo = (CdtInfo*) event.Data(0);
    const FoundInfo *ball = &cdtInfo->foundInfo[IO_BALL];
    if ( ball->found ) {
      sprintf(tmpbuf, "sensor-update ball true ballPan %.1f ballTilt %.1f ballDistance %d",
	      ball->objPos.pan, ball->objPos.tilt, ball->distance);
    } else {
      sprintf(tmpbuf, "sensor-update ball false");
    }
    SensorUpdate(tmpbuf);
  }
  observer[event.ObsIndex()]->AssertReady();
}
