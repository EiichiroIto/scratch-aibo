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

#include <sys/stat.h>
#include <sys/unistd.h>
#include <OPENR/OSyslog.h>
#include <OPENR/ODebug.h>
#include <OPENR/OCalendarTime.h>
#include <stdlib.h>
#include "FtpDTP.h"
#include <stdio.h>

Port 
FtpDTP::SetIP(IPAddress ip)
{
    connectIP = ip;
    connectPort = FTP_PASV_DATA_PORT + (int)continuation;
    passive = true;
    Listen();
    return connectPort;
}

bool
FtpDTP::SetPort(char* ipport)
{
    longword buf;
    uint32 ip = 0;
    uint16 port = 0;

    buf = atoi(ipport);
    ip |= (buf << 24);
    while (*ipport++ != ',') {
        ;
    }
    buf = atoi(ipport);
    ip |= (buf << 16);
    while (*ipport++ != ',') {
        ;
    }
    buf = atoi(ipport);
    ip |= (buf << 8);
    while (*ipport++ != ',') {
        ;
    }
    buf = atoi(ipport);
    ip |= buf;
    while (*ipport++ != ',') {
        ;
    }

    buf = atoi(ipport);
    port |= (buf << 8);
    while (*ipport++ != ',') {
        ;
    }
    buf = atoi(ipport);
    port |= buf;

    OSYSDEBUG(("FtpDTP::Port():Set IP:%x Port:%d\n", ip, port));
    connectIP   = (IPAddress)ip;
    connectPort = (Port)port;
    passive = false;
    return true;
}

bool
FtpDTP::Store(char* filename)
{
    char file[MAX_STRING_LENGTH * 2];
    method = FTP_METHOD_STOR;

    if (filename[0] == '/') {
        sprintf(file, "%s", filename);
    } else {
        sprintf(file, "%s%s", ftpDir, filename);
    }

    DirNorm(file);

    if (!strncmp(file, ftpHome, strlen(ftpHome))) {
        if((fp = fopen(file, "wb")) == NULL) {
            OSYSLOG1((osyslogERROR, "Ftpd::File Open Failed %s.", file));
            return false;
        }
    } else {
        return false;
    }

    strncpy(ftpFile, filename, MAX_STRING_LENGTH);

    if (passive) {
        if (connection.state == CONNECTION_CONNECTED) {
            connection.recvSize = 0;
            Receive();
        }
    } else {
        Connect();
    }
    return true;
}

bool
FtpDTP::Retrieve(char* filename)
{
    char file[MAX_STRING_LENGTH * 2];
    method = FTP_METHOD_RETR;

    if (filename[0] == '/') {
        sprintf(file, "%s", filename);
    } else {
        sprintf(file, "%s%s", ftpDir, filename);
    }

    DirNorm(file);

    if (!strncmp(file, ftpHome, strlen(ftpHome))) {
        if((fp = fopen(file, "rb")) == NULL) {
            OSYSLOG1((osyslogERROR, "FtpDTP::File Open Failed %s.", file));
            return false;
        }
    } else {
        return false;
    }

    strncpy(ftpFile, filename, MAX_STRING_LENGTH);

    if (passive) {
        if (connection.state == CONNECTION_CONNECTED) {
            RetrieveSend();
        }
    } else {
        Connect();
    }
    return true;
}

bool
FtpDTP::RetrieveSend()
{
    if (dataType == FTP_DATA_A) {
       connection.recvSize
           = fread(connection.recvData, 1, FTP_BUFFER_SIZE / 2, fp);
       if (FTP_BUFFER_SIZE / 2 > connection.recvSize) {
            byte *cur_r = connection.recvData;
            byte *cur_s = connection.sendData;
            connection.sendSize = connection.recvSize;
            for (int i = 0; i < connection.recvSize; i++, cur_r++) {
                if (*cur_r == LF) {
                    if ((cur_r != connection.recvData)
                        && (*(cur_r - 1) != CR)) {
                        *cur_s++ = CR;
                        connection.sendSize++;
                    }
                    *cur_s++ = *cur_r;
                } else {
                    *cur_s++ = *cur_r;
                }
            }
            fclose(fp);
            method = FTP_METHOD_NOOP;
            Send();
        } else {
            byte *cur_r = connection.recvData;
            byte *cur_s = connection.sendData;
            connection.sendSize = connection.recvSize;
            for (int i = 0; i < connection.recvSize; i++, cur_r++) {
                if (*cur_r == LF) {
                    if ((cur_r != connection.recvData)
                        && (*(cur_r - 1) != CR)) {
                        *cur_s++ = CR;
                        connection.sendSize++;
                    }
                    *cur_s++ = *cur_r;
                } else {
                    *cur_s++ = *cur_r;
                }
            }
            Send();
        }
    } else {
        connection.sendSize
            = fread(connection.sendData, 1, FTP_BUFFER_SIZE, fp);
        if (FTP_BUFFER_SIZE > connection.sendSize) {
            fclose(fp);
            method = FTP_METHOD_NOOP;
            Send();
        } else {
            Send();
        }
    }
    return true;
}

bool
FtpDTP::RenameFrom(char *file)
{
    char tmp[MAX_STRING_LENGTH * 2];

    if (file[0] == '/') {
        sprintf(tmp, "%s", file);
    } else {
        sprintf(tmp, "%s%s", ftpDir, file);
    }

    DirNorm(tmp);

    if (!strncmp(tmp, ftpHome, strlen(ftpHome))) {
        FILE *fp = fopen(tmp, "r");
        if (fp != NULL) {
            strncpy(ftpFile, tmp, MAX_STRING_LENGTH);
            fclose(fp);
            return true;
        }
    }

    return false;
}

bool
FtpDTP::RenameTo(char *file)
{
    char tmp[MAX_STRING_LENGTH * 2];

    if (file[0] == '/') {
        sprintf(tmp, "%s", file);
    } else {
        sprintf(tmp, "%s%s", ftpDir, file);
    }

    DirNorm(tmp);

    if (!strncmp(tmp, ftpHome, strlen(ftpHome))) {
        int ret = rename(ftpFile, tmp);
        if (ret == 0) {
            strcpy(ftpFile, "");
            return true;
        }
    }

    strcpy(ftpFile, "");
    return false;
}

bool
FtpDTP::List(char* dir)
{
    char tmp[MAX_STRING_LENGTH * 2];
    total = true;

    if ((dir[0] != '\0') && (dir[0] == '/')) {
        sprintf(tmp, "%s/", dir);
        listLong = false;
    } else if ((dir[0] != '\0') && (dir[0] == '-')) {
        sprintf(tmp, "%s", ftpDir);
        listLong = true;
    } else {
        sprintf(tmp, "%s%s", ftpDir, dir);
        listLong = false;
    }

    listLong = true;

    DirNorm(tmp);

    if (!strncmp(tmp, ftpHome, strlen(ftpHome))) {
        if (dirp = opendir(tmp)) {
            method = FTP_METHOD_LIST;
            strcpy(ftpFile, tmp);
            if (passive) {
                if (connection.state == CONNECTION_CONNECTED) {
                    if (!ListSend()) {
                        Close();
                    }
                }
            } else {
                Connect();
            }
            return true;
        }
    }
    return false;
}

bool
FtpDTP::ListSend()
{
    dirent *entry;
    char tmp[MAX_STRING_LENGTH];

    if (total) {
        sprintf((char *)connection.sendData,
                "total %d\r\n", 0);
        connection.sendSize = strlen((char *)connection.sendData);
        Send();
        total = false;
        return true;
    }

    entry = readdir(dirp);
    if (entry) {
        struct stat statbuf;
        sprintf(tmp, "%s/%s", ftpFile, entry->d_name);
        DirNorm(tmp);
        
        int ret = stat(tmp, &statbuf);
        if (listLong) {
            sprintf((char *)connection.sendData,
                    "%c%c%c%c%c%c%c%c%c%c %x %s %s %-d %s %d %d %s\r\n",
                    (statbuf.st_mode & S_IFMT) == S_IFDIR ? 'd' : '-',
                    (statbuf.st_mode & S_IRUSR) ? 'r' : '-',
                    (statbuf.st_mode & S_IWUSR) ? 'w' : '-',
                    (statbuf.st_mode & S_IFMT) == S_IFDIR ? 'x' : '-',
                    (statbuf.st_mode & S_IRUSR) ? 'r' : '-',
                    (statbuf.st_mode & S_IWUSR) ? 'w' : '-',
                    (statbuf.st_mode & S_IFMT) == S_IFDIR ? 'x' : '-',
                    (statbuf.st_mode & S_IRUSR) ? 'r' : '-',
                    (statbuf.st_mode & S_IWUSR) ? 'w' : '-',
                    (statbuf.st_mode & S_IFMT) == S_IFDIR ? 'x' : '-',
                    0,
                    "AIBO",
                    "AIBO",
                    statbuf.st_size,
                    "May",
                    11,
                    1999,
                    entry->d_name
                );
        } else {
            if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
                sprintf((char *)connection.sendData,
                        "%s/\r\n",
                        entry->d_name);
            } else {
                sprintf((char *)connection.sendData,
                        "%s\r\n",
                        entry->d_name);
            }
        }
        connection.sendSize = strlen((char *)connection.sendData);
        Send();
    } else {
        method = FTP_METHOD_NOOP;
        closedir(dirp);
        return false;
    }

    return true;
}

bool
FtpDTP::ChangeDir(char* dir)
{
    char tmp[MAX_STRING_LENGTH * 2];

    if (dir[0] == '/') {
        sprintf(tmp, "%s/", dir);
    } else {
        sprintf(tmp, "%s%s/", ftpDir, dir);
    }

    DirNorm(tmp);

    if (!strncmp(tmp, ftpHome, strlen(ftpHome))) {
        DIR* tmpdirp;
        if (tmpdirp = opendir(tmp)) {
            closedir(tmpdirp);
            strcpy(ftpDir, tmp);
            return true;
        }
    }

    return false;
}

bool
FtpDTP::MakeDir(char* dir)
{
    char tmp[MAX_STRING_LENGTH * 2];

    if (dir[0] == '/') {
        sprintf(tmp, "%s", dir);
    } else {
        sprintf(tmp, "%s%s", ftpDir, dir);
    }

    DirNorm(tmp);

    if (!strncmp(tmp, ftpHome, strlen(ftpHome))) {
        if (!mkdir(tmp, 0777)) {
            return true;
        }
    }

    return false;
}

bool
FtpDTP::RemoveDir(char* dir)
{
    char tmp[MAX_STRING_LENGTH * 2];

    if (dir[0] == '/') {
        sprintf(tmp, "%s", dir);
    } else {
        sprintf(tmp, "%s%s", ftpDir, dir);
    }

    DirNorm(tmp);

    if (!strncmp(tmp, ftpHome, strlen(ftpHome))) {
        if (!rmdir(tmp)) {
            return true;
        }
    }

    return false;
}

bool
FtpDTP::Delete(char* filename)
{
    char tmp[MAX_STRING_LENGTH * 2];

    if (filename[0] == '/') {
        sprintf(tmp, "%s", filename);
    } else {
        sprintf(tmp, "%s%s", ftpDir, filename);
    }

    DirNorm(tmp);

    if (!strncmp(tmp, ftpHome, strlen(ftpHome))) {
        if (!remove(tmp)) {
            return true;
        }
    }

    return false;
}

void
FtpDTP::Save(byte *data, int length, bool end)
{
    OSYSDEBUG(("FtpDTP::Save %d %d\n", length, end));
    if (length) fwrite(data, length, 1, fp);
    if (end) {
        fclose(fp);
        strcpy(ftpFile, "");
    }
}

size_t
FtpDTP::GetFileSize(char *name)
{
    struct stat statbuf;
    int ret = stat(name, &statbuf);
    if (ret == 0) {
        return statbuf.st_size;
    } else {
        return 0;
    }
}

void
FtpDTP::ResetFilename()
{
    strcpy(ftpFile, "");
}

void
FtpDTP::SetUser(char *user)
{
    strncpy(ftpUser, user, MAX_STRING_LENGTH);
}

void
FtpDTP::SetHome(char *home)
{
    sprintf(ftpHome, "%s/", home);

    DirNorm(ftpHome);
    strcpy(ftpDir, ftpHome);
}

void
FtpDTP::DirNorm(char *dir)
{
    char tmp[MAX_STRING_LENGTH * 2];
    int length = strlen(dir);
    int cur = 0;

    for (int i = 0; i < length + 1; i++) {
        OSYSDEBUG(("DirNorm %c\n", dir[i]));
        if (dir[i] == '.') {
            if ((dir[i + 1] == '.') && (dir[i + 2] == '/')) {
                cur -= 2;
                while(tmp[cur] != '/') {
                    cur--;
                }
                cur++;
            } else if (dir[i + 1] == '/') {
                i++;
            } else {
                tmp[cur++] = toupper(dir[i]);
            }
        } else if (dir[i] == '/') {
            if ((cur == 0) || ((cur != 0) && (tmp[cur - 1] != '/'))) {
                tmp[cur++] = toupper(dir[i]);
            }
        } else {
            tmp[cur++] = toupper(dir[i]);
        }
    }
        
    strncpy(dir, tmp, MAX_STRING_LENGTH);
    dir[MAX_STRING_LENGTH] = '0';
}
