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

#ifndef ODA_h_DEFINED
#define ODA_h_DEFINED

#include <Types.h>

typedef longword ODAMagic;
const ODAMagic ODA_MAGIC_UNDEF = 0xffffffff; // UNDEFINED
const ODAMagic ODA_MAGIC_OSND  = 0x444e534f; // 'OSND' 
const ODAMagic ODA_MAGIC_ODAR  = 0x5241444f; // 'ODAR' 
const ODAMagic ODA_MAGIC_OMTN  = 0x4e544d4f; // 'OMTN'
const ODAMagic ODA_MAGIC_WAVE  = 0x45564157; // 'WAVE'
const ODAMagic ODA_MAGIC_MIDI  = 0x4944494d; // 'MIDI'
const ODAMagic ODA_MAGIC_LED   = 0x0044454c; // 'LED '
const ODAMagic ODA_MAGIC_SYN   = 0x004e5953; // 'SYN ' 

struct ODAInfo {            // 64 bytes (total)
    ODAMagic  magic;        //  4 bytes
    longword  version;		//  4 bytes
    longword  numFiles;     //  4 bytes
    longword  entrySize;    //  4 bytes
    byte      reserved[48]; // 48 bytes
};

struct ODAEntry {           // 144 bytes (total)
    ODAMagic  magic;        //   4 bytes
    char      name[128];	// 128 bytes
    longword  offset;		//   4 bytes
    longword  size;         //   4 bytes
    byte      reserved[4];	//   4 bytes
};

struct ODATOC {
    ODAInfo   info;
    ODAEntry  entry[1];
};

class ODA {
public:
    ODA();
    ODA(byte* oda);
    ~ODA() {}

    void Set(byte* oda);

    int Find(char* name) const;
    int Find(ODAMagic magic, char* name) const;
    int GetNumFiles() const;

    ODAMagic GetMagic (int index) const;
    char*    GetName  (int index) const;
    int      GetSize  (int index) const;
    byte*    GetData  (int index) const;
    int      GetOffset(int index) const;

private:
    ODATOC* toc;
};

#endif // ODA_h_DEFINED
