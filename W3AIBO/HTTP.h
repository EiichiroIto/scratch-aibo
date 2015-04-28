//
// Copyright 2003,2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef HTTP_h_DEFINED
#define HTTP_h_DEFINED

enum HTTPStatus {
	HTTP_CONTINUE,
	HTTP_SWITCHING_PROTOCOLS,
	HTTP_OK,
	HTTP_CREATED,
	HTTP_ACCEPTED,
	HTTP_NO_CONTENT,
	HTTP_BAD_REQUEST,
	HTTP_NOT_FOUND,
	HTTP_SERVER_ERROR,
	HTTP_NOT_IMPLEMENTED,
	HTTP_STATUS_END,
};

enum HTTPHeader {
	HTTP_SERVER,
	HTTP_CONTENT_TYPE,
	HTTP_CONTENT_LENGTH,
	HTTP_MIME_VERSION,
	HTTP_EXPIRES,
	HTTP_DATE,
	HTTP_HEADER_END,
};

static const char* const HTTP_VERSION = "HTTP/1.0";

class HTTP {
public:
    HTTP() {}
    ~HTTP() {}

    bool Parse(char* httpReq, char* method, char* uri, char* httpVer);

    int Status(char* dest, HTTPStatus st);
    int HeaderField(char* dest, HTTPHeader hdr, char* value);

private:
    static const int MAX_HEADER_SIZE = 2048;
    static const int BUFSIZE = MAX_HEADER_SIZE + 3; // '\r\n\0'

    char buffer[BUFSIZE];
};

#endif // HTTP_h_DEFINED
