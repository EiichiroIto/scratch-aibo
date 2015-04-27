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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include "FtpPI.h"

bool
FtpPI::RequestComplete()
{
    char* c = (char*)connection.recvData;

    for (int i = 0; i < connection.recvSize - 1; i++) {
        if ((c[i] == CR) && (c[i + 1] == LF)) {
            c[i] = '\0';
            return true;
        }
    }
    
    return false;
}

bool
FtpPI::CommandParser(char **cmd, char **param)
{
    char *cur = (char *)connection.recvData;
    *cmd = cur;

    while (isalpha(*cur)) {
        *cur = toupper(*cur);
        cur++;
    }

    while (*cur == SP) {
        *cur = '\0';
        cur++;
    }

    *param = cur;
    return true;
}

bool
FtpPI::RequestProcess()
{
    //
    // process all lines
    //
    char *cmd;
    char *param;
    CommandParser(&cmd, &param);
    if (strlen(cmd) >= MAX_STRING_LENGTH) {
        cmd[MAX_STRING_LENGTH - 1] = '\0';
    }

    if (strlen(param) >= MAX_STRING_LENGTH) {
        param[MAX_STRING_LENGTH - 1] = '\0';
    }

    OSYSDEBUG(("FtpPI::RequestProcess() cmd %s : %s\n", cmd, param));

    // process command
    if (!strcmp(cmd, "USER")) {
        state = FTP_LOGIN;
        ftpDTP.SetUser(param);
        Send(FTP_REPLY_NEED_PASSWD, "Password required for %s.", param);
        connection.recvSize = 0;
        Receive();
    } else if (!strcmp(cmd, "PASS")) {
        if (state == FTP_LOGIN) {
            OList<Passwd, MAX_LOGIN>::Iterator iter = passwd->Begin();
            OList<Passwd, MAX_LOGIN>::Iterator last = passwd->End();
            while (iter != last) {
                if (!strcmp((*iter).user, ftpDTP.GetUser())) {
                    if (!strcmp((*iter).pass, param) ||
                        !strcmp((*iter).pass, "*")) {
                        ftpDTP.SetHome((*iter).home);
                        state = FTP_GET_REQUEST;
                        Send(FTP_REPLY_USER_LOGIN, "User %s logged in.",
                             (*iter).user);
                        connection.recvSize = 0;
                        Receive();
                    } else {
                        state = FTP_NOT_LOGIN;
                        Send(FTP_REPLY_NOT_LOGIN, "Login incorrect.");
                        connection.recvSize = 0;
                        Receive();
                    }
                    break;
                }
                ++iter;
            }

            if (iter == last) {
                state = FTP_NOT_LOGIN;
                Send(FTP_REPLY_NOT_LOGIN, "Login incorrect.");
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_BAD_SEQUENCE, "Login with USER first.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "TYPE")) {
        switch(*param) {
        case 'I':
            ftpDTP.SetType(FTP_DATA_I);
            Send(FTP_REPLY_COMMAND_OK, "TYPE set to %s.", param);
            break;

        case 'A':
            ftpDTP.SetType(FTP_DATA_A);
            Send(FTP_REPLY_COMMAND_OK, "TYPE set to %s.", param);
            break;

        default:
            Send(FTP_REPLY_NOT_IMPLEMENT, "TYPE set to %s.", param);
            break;
        }
        connection.recvSize = 0;
        Receive();
    } else if (!strcmp(cmd, "PORT")) {
        if (state == FTP_GET_REQUEST) {
            if (ftpDTP.SetPort(param)) {
                Send(FTP_REPLY_COMMAND_OK, "PORT command successful.");
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "PASV")) {
        if (state == FTP_GET_REQUEST) {
            Port port = ftpDTP.SetIP(ipaddr);
            if (port){
                Send(FTP_REPLY_ENTER_PASSIVE,
                     "Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
                     (ipaddr.Address() & 0xff000000) >> 24,
                     (ipaddr.Address() & 0x00ff0000) >> 16,
                     (ipaddr.Address() & 0x0000ff00) >>  8,
                     (ipaddr.Address() & 0x000000ff),
                     (port & 0xff00) >>  8,
                     (port & 0x00ff));
                connection.recvSize = 0;
                Receive();
            }else {
                Send(FTP_REPLY_NOT_AVAILABLE, "Passive Mode Fail.");
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "RETR")) {
        if (state == FTP_GET_REQUEST) {
            if (!ftpDTP.Retrieve(param)) {
                Send(FTP_REPLY_NO_FILE, "No such file.");
                connection.recvSize = 0;
                Receive();
            } else {
                return true;
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "STOR")) {
        if (state == FTP_GET_REQUEST) {
            if (!ftpDTP.Store(param)) {
                Send(FTP_REPLY_NOT_TOKEN, "error.");
                connection.recvSize = 0;
                Receive();
            } else {
                return true;
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "RNFR")) {
        if (state == FTP_GET_REQUEST) {
            if (ftpDTP.RenameFrom(param)) {
                Send(FTP_REPLY_REQUESTED_FILE,
                     "File exists, ready for destination name");
                connection.recvSize = 0;
                Receive();
                return true;
            } else {
                Send(FTP_REPLY_NO_FILE, "%s : No such file or directory",
                     param);
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "RNTO")) {
        if (state == FTP_GET_REQUEST) {
            char *tmp = ftpDTP.GetFilename();
            if (*tmp) {
                if (ftpDTP.RenameTo(param)) {
                    Send(FTP_REPLY_FILE_ACTION_OK, "RNTO command successful.");
                    connection.recvSize = 0;
                    Receive();
                } else {
                    Send(FTP_REPLY_NO_FILE, "%s : No such file or directory",
                         param);
                    connection.recvSize = 0;
                    Receive();
                }
            } else {
                Send(FTP_REPLY_BAD_SEQUENCE, "Bad sequence of commands.");
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "LIST")) {
        if (state == FTP_GET_REQUEST) {
            if (!ftpDTP.List(param)) {
                Send(FTP_REPLY_NOT_TAKEN, "Not taken.");
                connection.recvSize = 0;
                Receive();
            } else {
                return true;
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "NLST")) {
        if (state == FTP_GET_REQUEST) {
            if (!ftpDTP.List(param)) {
                Send(FTP_REPLY_NOT_TAKEN, "Not taken.");
                connection.recvSize = 0;
                Receive();
            } else {
                return true;
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "CWD")) {
        if (state == FTP_GET_REQUEST) {
            if (ftpDTP.ChangeDir(param)) {
                Send(FTP_REPLY_FILE_ACTION_OK, "CWD command successful.");
                connection.recvSize = 0;
                Receive();
            } else {
                Send(FTP_REPLY_NO_FILE, "%s: No such file or directory.",
                     param);
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "CDUP")) {
        if (state == FTP_GET_REQUEST) {
            if (ftpDTP.ChangeDir("../")) {
                Send(FTP_REPLY_FILE_ACTION_OK, "CWD command successful.");
                connection.recvSize = 0;
                Receive();
            } else {
                Send(FTP_REPLY_NO_FILE, "%s: No such file or directory.",
                     param);
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "PWD")) {
        if (state == FTP_GET_REQUEST) {
            Send(FTP_REPLY_PATH_CREATED, "\"%s\" is current directory.",
                 ftpDTP.GetDirectry());
            connection.recvSize = 0;
            Receive();
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "MKD")) {
        if (state == FTP_GET_REQUEST) {
             if (ftpDTP.MakeDir(param)) {
                Send(FTP_REPLY_PATH_CREATED, "MKD command successful.");
                connection.recvSize = 0;
                Receive();
            } else {
                Send(FTP_REPLY_NO_FILE, "MKD command Failed.");
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "RMD")) {
        if (state == FTP_GET_REQUEST) {
             if (ftpDTP.RemoveDir(param)) {
                Send(FTP_REPLY_FILE_ACTION_OK, "RMD command successful.");
                connection.recvSize = 0;
                Receive();
            } else {
                Send(FTP_REPLY_NO_FILE, "RMD command Failed.");
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "DELE")) {
        if (state == FTP_GET_REQUEST) {
            if (ftpDTP.Delete(param)) {
                Send(FTP_REPLY_FILE_ACTION_OK, "DELE command successful.");
                connection.recvSize = 0;
                Receive();
            } else {
                Send(FTP_REPLY_NO_FILE, "DELE command Failed.");
                connection.recvSize = 0;
                Receive();
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "REBT")) {
        if (state == FTP_GET_REQUEST) {
            Send(FTP_REPLY_COMMAND_OK, "REBT command successful.");
            Close();
            OBootCondition bootCond(obcbBOOT_TIMER, 0, obcbttRELATIVE);
            OStatus result = OPENR::Shutdown(bootCond);
            if (result != oSUCCESS) {
                OSYSLOG1((osyslogWARNING, "System shutdown... failed" ));
                return false;
            }
        } else {
            Send(FTP_REPLY_NOT_LOGIN, "Please login with USER and PASS.");
            connection.recvSize = 0;
            Receive();
        }
    } else if (!strcmp(cmd, "SYST")) {
        sprintf((char *)connection.sendData,
                "%03d Aperios system type.\r\n",
                FTP_REPLY_SYSTEM_TYPE);
        TCPEndpointSendMsg sendMsg(connection.endpoint,
                                   connection.sendData,
                                   strlen((char *)connection.sendData));
            
        sendMsg.continuation = continuation;
        sendMsg.Send(ipstackRef,
                     myOID, Extra_Entry[entrySendContforPI],
                     sizeof(TCPEndpointSendMsg));
        connection.recvSize = 0;
        Receive();
    } else if (!strcmp(cmd, "HELP")) {
        sprintf((char *)connection.sendData,
                "%03d-%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%03d %s\r\n",
                FTP_REPLY_HELP_MESSAGE,
                "The following commands are recognized:",
                "   USER    PORT    RETR    MSND*   ALLO*   DELE    SITE*   XMKD*   CDUP",
                "   PASS    PASV    STOR    MSOM*   REST*   CWD     STAT*   RMD     XCUP*",
                "   ACCT*   TYPE    APPE*   MSAM*   RNFR*   XCWD*   HELP    XRMD*   STOU*",
                "   REIN*   STRU*   MLFL*   MRSQ*   RNTO*   LIST    NOOP    PWD ",
                "   QUIT    MODE*   MAIL*   MRCP*   ABOR*   NLST*   MKD     XPWD*   REBT",
                FTP_REPLY_HELP_MESSAGE,
                "(*'s => unimplemented)");
        
        TCPEndpointSendMsg sendMsg(connection.endpoint,
                                   connection.sendData,
                                   strlen((char *)connection.sendData));
        
        sendMsg.continuation = continuation;
        sendMsg.Send(ipstackRef,
                     myOID, Extra_Entry[entrySendContforPI],
                     sizeof(TCPEndpointSendMsg));
        connection.recvSize = 0;
        Receive();
    } else if (!strcmp(cmd, "NOOP")) {
        Send(FTP_REPLY_COMMAND_OK, "NOOP command successful.");
        Receive();
    } else if (!strcmp(cmd, "QUIT")) {
        Send(FTP_REPLY_SERVICE_CLOSE,
             "Goodbye. Thanks for using the AIBO FTP Server.");
        Close();
    } else {
        Send(FTP_REPLY_UNKNOWN_COMMAND,
             "'%s %s': command not understood.", cmd, param);
        connection.recvSize = 0;
        Receive();
    }

    ftpDTP.ResetFilename();    
    return true;
}
