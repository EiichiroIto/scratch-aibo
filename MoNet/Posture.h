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

#ifndef Posture_h_DEFINED
#define Posture_h_DEFINED
#include <list>
using namespace std;

class Posture {
  private:
    static const int MaxPostures = 128;
    static const int MaxPostureLen = 32;
    char name[ MaxPostures ][ MaxPostureLen ];
    int code[ MaxPostures ];
    int size;
    int nextPostureCode ;

  public:
    Posture();
    //~Posture();
    void Add( const char *_name, int _code );
    int Find( const char *_name ) const;
    void Print() const;
    void Read( const char *path );
};
#endif // Posture_h_DEFINED
