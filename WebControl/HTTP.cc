//
// Copyright 2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <stdio.h>
#include <string.h>
#include "HTTP.h"

struct http_code {
  int num;
  char *str;
  int len;
};

static http_code _http_status[] = {
  {100, "Continue",               sizeof("Continue")},
  {101, "Switching Protocols",    sizeof("Switching Protocols")},
  {200, "Ok",                     sizeof("Ok")},
  {201, "Created",                sizeof("Created")},
  {202, "Accepted",               sizeof("Accepted")},
  {204, "No Content",             sizeof("No Content")},
  {400, "Bad Request",            sizeof("Bad Request")},
  {400, "Not Found",              sizeof("Not Found")},
  {500, "Internal Server Error",  sizeof("Internal Server Error")},
  {501, "Not Implemented",        sizeof("Not Implemented")},
  {200, "Ok",                     sizeof("Ok")},
};

static http_code _http_header[] = {
  {HTTP_SERVER,         "Server: ",         sizeof ("Server: ")},
  {HTTP_CONTENT_TYPE,   "Content-Type: ",	  sizeof ("Content-type: ")},
  {HTTP_CONTENT_LENGTH, "Content-Length: ", sizeof ("Content-length: ")},
  {HTTP_MIME_VERSION,   "MIME-Version: ",   sizeof ("MIME-Version: ")},
  {HTTP_EXPIRES,        "Expires: ",        sizeof ("Expires: ")},
  {HTTP_DATE,           "Date: ",           sizeof ("Date: ")},
  {HTTP_HEADER_END,     "\r\n",             sizeof ("\r\n")},
};

bool
HTTP::Parse(char* httpReq, char* method, char* uri, char* httpVer)
{
  int len;
  char* start;
  char* end;

  // Method
  start = end = httpReq;
  end   = httpReq;
  while (*end != ' ' && *end != '\0') end++;
  if (*end == '\0') return false;
  len = end - start;
  memcpy(method, start, len);
  method[len] = '\0';

  // URI
  end++;
  start = end;
  while (*end != ' ' && *end != '\0') end++;
  if (*end == '\0') return false;
  len = end - start;
  memcpy(uri, start, len);
  uri[len] = '\0';
  
  // HTTP version
  end++;
  start = end;
  while (*end != '\r' && *end != '\0') end++;
  if (*end == '\0') return false;
  len = end - start;
  memcpy(httpVer, start, len);
  httpVer[len] = '\0';

  return true;
}

int 
HTTP::Status(char* dest, HTTPStatus st)
{
  http_code* code = &_http_status[st];

  sprintf (buffer, "%s %d %s\r\n", HTTP_VERSION, code->num, code->str);
  int status_length = strlen(buffer);
  memcpy(dest, buffer, status_length);

  return status_length;
}

int
HTTP::HeaderField(char* dest, HTTPHeader hdr, char *value)
{
  http_code* code = &_http_header[hdr];
  int header_length = 0;
  int len;

  sprintf (buffer, "%s", code->str);
  len = strlen(buffer);
  memcpy(dest, buffer, len);
  dest += len;
  header_length += len;

  if (value) {
    len = strlen (value);
    if (len > MAX_HEADER_SIZE)
      value = "-- buffer too short! --";
    sprintf (buffer, "%s\r\n", value);
    len = strlen(buffer);
    memcpy(dest, buffer, len);
    header_length += len;
  }

  return header_length;
}
