//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef PowerMonitor_h_DEFINED
#define PowerMonitor_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "def.h"

class PowerMonitor : public OObject {
public:
    PowerMonitor();
    virtual ~PowerMonitor() {}

    OSubject*   subject[numOfSubject];
    OObserver*	observer[numOfObserver];     

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void UpdatePowerStatus(void* msg);

private:
    void PrintPowerStatus(const OPowerStatus& ps);
};

#endif // PowerMonitor_h_DEFINED
