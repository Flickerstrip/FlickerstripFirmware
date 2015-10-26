#ifndef networkutil_h
#define networkutil_h

#include "util.h"

void sendHttp(WiFiClient * client, int statusCode, const char * statusText, const char * contentType, const char * content) {
  int contentLength = strlen(content);
  char buffer[contentLength+300];
  int n = snprintf(buffer,contentLength+300,"HTTP/1.0 %d %s\r\nContent-Type: %s\r\nContent-Length:%d\r\n\r\n%s",statusCode,statusText,contentType,strlen(content),content);

  client->write((uint8_t*)buffer,n);
}

void sendOk(WiFiClient * client) {
  char content[] = "{\"type\":\"OK\"}";
  sendHttp(client,200,"OK","application/json",content);
}

void sendErr(WiFiClient * client, const char * err) {
  char content[strlen(err)+50];
  int n = snprintf(content,strlen(err)+50,"{\"type\":\"error\",\"message\":\"%s\"}",err);
  content[n] = 0;
  sendHttp(client,500,"Bad Request","application/json",content);
}

int readBytes(WiFiClient & client, char * buf, int length, int timeout) {
  long start = millis();
  int bytesRead = 0;
  while(bytesRead < length) {
    yield();
    if (client.connected() == false || millis() - start > timeout) break;
    if (client.available()) {
      int readThisTime = client.read((uint8_t*)buf,length-bytesRead);
      bytesRead += readThisTime;
      buf += readThisTime;
      if (bytesRead != 0) {
        start = millis();
      }
    }
  }
  return bytesRead;
}

int readUntil(WiFiClient * client, char * buffer, const char * search, long timeout) {
  long start = millis();
  int n = 0;
  int i = 0;
  int searchlen = strlen(search);
  while(client->connected()) {
    if (millis() - start > timeout) break;

    int b = client->read();
    if (b == -1) continue;
    start = millis();
    buffer[n++] = b;
    if (search[i] == b) {
      i++;
    } else {
      i = 0;
    }
    if (i >= searchlen) break;
  }

  return n;
}

int getContentLength(const char * buf) {
  char search[] = "content-length:";
  const char * ptr = stristr(buf,search);
  if (ptr == NULL) return 0;
  ptr += strlen(search);
  while(ptr[0] == '\n' || ptr[0] == '\r' || ptr[0] == '\t' || ptr[0] == ' ') ptr++;
  return atoi(ptr);
}

int findUrl(const char * buf, char ** loc) {
  char * ptr;

  ptr = strchr(buf,' ');
  ptr++;
  loc[0] = ptr;

  ptr = strchr(ptr,' ');

  return ptr - loc[0];
}

//Finds the location of the path in a header buffer, returns length and update the location
int findPath(const char * buf, char ** loc) {
  char * url;
  int n = findUrl(buf,&url);

  loc[0] = url;

  char * ptr = (char*)memchr(url,'?',n);
  if (ptr == NULL) return n;

  return ptr-url;
}

//Gets the raw string value for a specified GET argument, returns the length if found, -1 if not found
int findGet(const char * buf, char ** loc, const char * key) {
  char * url;
  int n = findUrl(buf,&url);

  char * ptr = strchr(url,'?');
  if (ptr == NULL) return -1;

  ptr = strstr(ptr+1,key);
  if (ptr == NULL) return -1;

  char * a = strchr(ptr+1,' ');
  char * b = strchr(ptr+1,'&');
  if (b != NULL and (a == NULL or a > b)) {
    a = b;
  }

  if (a == NULL) return -1;

  loc[0] = ptr + strlen(key) + 1;
  return a - loc[0];
}

//Gets an integer for the specified "GET" argument, returns true if successful, stores integer into "val"
bool getInteger(const char * buf, const char * key, int * val) {
  char * vloc;
  int vlen = findGet(buf,&vloc,key);
  if (vlen != -1) {
    char cval[vlen+1];
    memcpy(&cval,vloc,vlen);
    cval[vlen] = 0;
    int ival = atoi(cval);
    val[0] = ival;
    return true;
  } else {
    return false;
  }
}



// Debugging
void printFound(int n, char * loc) {
  char tmp[n+1];
  memcpy(&tmp,loc,n);
  tmp[n] = 0;
  Serial.println(tmp);
}

#endif
