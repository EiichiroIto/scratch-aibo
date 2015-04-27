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

#ifndef Face_h_DEFINED
#define Face_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"
#include "command.h"

enum FaceState {
    FS_IDLE,
    FS_READY,
    FS_BUSY
};

static const size_t NUM_EARS = 2;
#ifdef ERS210
static const size_t NUM_LEDS = 6;
static const size_t NUM_MODES = 1;
#endif // ERS210
#ifdef ERS7
static const size_t NUM_HEADS = 2;
static const size_t NUM_MODES = 3;
static const size_t NUM_LANS = 1;
static const size_t NUM_FACES = 14;
#endif // ERS7

class Face : public OObject {
  public:
    Face();
    virtual ~Face() {}

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void Ready(const OReadyEvent& event);
    void Notify(const ONotifyEvent& event);

private:
    void OpenPrimitives();
    void NewCommandVectorData1();
    bool NewCommandVectorData2( int index, int numData,
				OCommandVectorData** cmdVecDataPtr);

    FaceState faceState;
    int lastFCmd;
    OPrimitiveID earID[NUM_EARS];
#ifdef ERS210
    OPrimitiveID ledID[NUM_LEDS];
    OPrimitiveID modeID[NUM_MODES];
#endif // ERS210
#ifdef ERS7
    OPrimitiveID headID[NUM_HEADS];
    OPrimitiveID modeID[NUM_MODES];
    OPrimitiveID lanID[NUM_LANS];
    OPrimitiveID faceID[NUM_FACES];
#endif // ERS7
    RCRegion* region[NumFaceCommands];
};

#endif // Face_h_DEFINED
