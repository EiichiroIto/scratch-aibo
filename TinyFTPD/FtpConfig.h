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

#ifndef _FtpConfig_h_DEFINED
#define _FtpConfig_h_DEFINED

#define FTP_LISTEN_PORT    21
#define FTP_DATA_PORT      20
#define FTP_PASV_DATA_PORT 3000
#define FTP_CONNECTION_MAX 4
#define FTP_BUFFER_SIZE    8192
#define FTP_PASSWD_PATH    "/MS/OPEN-R/MW/CONF/PASSWD"
#define MAX_STRING_LENGTH  128
#define MAX_LOGIN          8

// some character constants
// ---------------------------------------

#define CR 13   // carriage return
#define LF 10   // line feed
#define HT 9    // horizontal tab
#define SP 32   // space

struct Passwd {
    char user[MAX_STRING_LENGTH];
    char pass[MAX_STRING_LENGTH];
    char home[MAX_STRING_LENGTH];

    Passwd() {
        user[0] = '\0';
        pass[0] = '\0';
        home[0] = '\0';
    }
};

enum FTPLoginState {
    FTP_NOT_LOGIN,
    FTP_LOGIN,
    FTP_GET_REQUEST,
    FTP_UNDEF
};

enum FTPDataType {
    FTP_DATA_I,
    FTP_DATA_A
};

// FTP Method
// --------------------------------------------------------
enum FTPMethod {
    FTP_METHOD_USER,
    FTP_METHOD_PASS,
    FTP_METHOD_PORT,
    FTP_METHOD_PASV,
    FTP_METHOD_TYPE,
    FTP_METHOD_MODE,
    FTP_METHOD_STRU,
    FTP_METHOD_RETR,
    FTP_METHOD_STOR,
    FTP_METHOD_APPE,
    FTP_METHOD_RNFR,
    FTP_METHOD_RNTO,
    FTP_METHOD_DELE,
    FTP_METHOD_CWD,
    FTP_METHOD_CDUP,
    FTP_METHOD_RMD,
    FTP_METHOD_MKD,
    FTP_METHOD_PWD,
    FTP_METHOD_LIST,
    FTP_METHOD_NLST,
    FTP_METHOD_SYST,
    FTP_METHOD_STAT,
    FTP_METHOD_HELP,
    FTP_METHOD_NOOP,
    FTP_METHOD_QUIT,
    FTP_METHOD_UNSUPPORTED
};

// FTP Status
// --------------------------------------------------------
enum FTPReplyCode
{
    // Positive Preliminary reply
    FTP_REPLY_RESTART_MARKER  = 110,
    FTP_REPLY_SERVER_READY    = 120,
    FTP_REPLY_TRANSFER_START  = 125,
    FTP_REPLY_OPEN_CONNECTION = 150,

    // Positive Completion reply
    FTP_REPLY_COMMAND_OK      = 200,
    FTP_REPLY_SUPERFLUOUS     = 202,
    FTP_REPLY_SYSTEM_STATUS   = 211,
    FTP_REPLY_HELP_MESSAGE    = 214,
    FTP_REPLY_SYSTEM_TYPE     = 215,
    FTP_REPLY_SERVICE_READY   = 220,
    FTP_REPLY_SERVICE_CLOSE   = 221,
    FTP_REPLY_CLOSE_DATA      = 226,
    FTP_REPLY_ENTER_PASSIVE   = 227,
    FTP_REPLY_USER_LOGIN      = 230,
    FTP_REPLY_FILE_ACTION_OK  = 250,
    FTP_REPLY_PATH_CREATED    = 257,

    // Positive Intermediate reply
    FTP_REPLY_NEED_PASSWD     = 331,
    FTP_REPLY_REQUESTED_FILE  = 350,

    // Transient Negative Completion reply
    FTP_REPLY_NOT_AVAILABLE   = 421,
    FTP_REPLY_TRANSFER_ABORT  = 426,
    FTP_REPLY_NOT_TAKEN       = 450,

    // Permanent Negative Completion reply
    FTP_REPLY_UNKNOWN_COMMAND = 500,
    FTP_REPLY_NOT_IMPLEMENT   = 502,
    FTP_REPLY_BAD_SEQUENCE    = 503,
    FTP_REPLY_NOT_LOGIN       = 530,
    FTP_REPLY_NO_FILE         = 550,
    FTP_REPLY_NOT_TOKEN       = 553,

    // Undefined Reply
    FTP_REPLY_NONE = 999
};

#endif /* _FtpConfig_h_DEFINED*/
