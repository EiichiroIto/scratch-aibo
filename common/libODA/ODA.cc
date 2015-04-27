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

#include <string.h>
#include <ODA.h>

ODA::ODA() : toc(0)
{
}

ODA::ODA(byte* oda)
{
    Set(oda);
}

void
ODA::Set(byte* oda)
{
    toc = (ODATOC*)oda;
}

int
ODA::Find(char* name) const
{
    if (toc == 0) return -1;
    if (toc->info.magic != ODA_MAGIC_ODAR) return -1;

    for (int i = 0; i < toc->info.numFiles; i++) {
        if (strcmp(name, toc->entry[i].name) == 0) return i;
    }

    return -1;
}

int
ODA::Find(ODAMagic magic, char* name) const
{
    if (toc == 0) return -1;
    if (toc->info.magic != ODA_MAGIC_ODAR) return -1;
    
    if (magic == ODA_MAGIC_OSND) {

        for (int i = 0; i < toc->info.numFiles; i++) {
            if (toc->entry[i].magic == ODA_MAGIC_WAVE &&
                strcmp(name, toc->entry[i].name) == 0) return i;
        }

        for (int i = 0; i < toc->info.numFiles; i++) {
            if (toc->entry[i].magic == ODA_MAGIC_MIDI &&
                strcmp(name, toc->entry[i].name) == 0) return i;
        }

    } else {

        for (int i = 0; i < toc->info.numFiles; i++) {
            if (toc->entry[i].magic == magic &&
                strcmp(name, toc->entry[i].name) == 0) return i;
        }
    }

    return -1;
}

int
ODA::GetNumFiles() const
{
    if (toc == 0) return -1;
    if (toc->info.magic != ODA_MAGIC_ODAR) return -1;
    return toc->info.numFiles;
}

ODAMagic
ODA::GetMagic(int index) const
{
    if (toc == 0) return ODA_MAGIC_UNDEF;
    if (toc->info.magic != ODA_MAGIC_ODAR) return ODA_MAGIC_UNDEF;
    if (index >= toc->info.numFiles) return ODA_MAGIC_UNDEF;

    return toc->entry[index].magic;
}

char* 
ODA::GetName(int index) const
{
    if (toc == 0) return 0;
    if (toc->info.magic != ODA_MAGIC_ODAR) return 0;
    if (index >= toc->info.numFiles) return 0;

    return toc->entry[index].name;
}

int
ODA::GetSize(int index) const
{
    if (toc == 0) return -1;
    if (toc->info.magic != ODA_MAGIC_ODAR) return -1;
    if (index >= toc->info.numFiles) return -1;

    return toc->entry[index].size;
}

byte*
ODA::GetData(int index) const
{
    if (toc == 0) return 0;
    if (toc->info.magic != ODA_MAGIC_ODAR) return 0;
    if (index >= toc->info.numFiles) return 0;

    byte* ptr = (byte*)toc;
    return ptr + toc->entry[index].offset;
}

int
ODA::GetOffset(int index) const
{
    if (toc == 0) return -1;
    if (toc->info.magic != ODA_MAGIC_ODAR) return -1;
    if (index >= toc->info.numFiles) return -1;

    return toc->entry[index].offset;
}
