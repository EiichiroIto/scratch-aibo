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

#ifndef CommandArc_h_DEFINED
#define CommandArc_h_DEFINED

#include <stdio.h>
#include <MoNetData.h>
#include "CommandNode.h"
#include "MoNetCommandInfo.h"

class CommandArc {
public:
    CommandArc(const CommandNode& s,
               const CommandNode& d,
               MoNetCommandInfo* c) : src(s), dest(d), commandInfo(c) {}
    ~CommandArc() {}

    const CommandNode& Source() const      { return src;  }
    const CommandNode& Destination() const { return dest; }
    int Distance() const                   { return 1;    }

    void Print() const { 
        printf("(");
        src.Print();
        printf("->");
        dest.Print();
        printf(")");
    }

    MoNetCommandInfo* CommandInfo() { return commandInfo; }

private:
    CommandNode       src;
    CommandNode       dest;
    MoNetCommandInfo* commandInfo;
};

#endif // CommandArc_h_DEFINED
