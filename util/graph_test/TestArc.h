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

#include <stdio.h>

class TestArc {
public:
    TestArc(const TestNode& s, const TestNode& d) : src(s), dest(d) {}
    ~TestArc() {}

    const TestNode& Source() const      { return src;  }
    const TestNode& Destination() const { return dest; }
    int Distance() const                { return 1;    }

    void Print() const { 
        printf("(");
        src.Print();
        printf("->");
        dest.Print();
        printf(")");
    }

private:
    TestNode src;
    TestNode dest;
};
