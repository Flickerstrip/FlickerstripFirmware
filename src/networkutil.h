#ifndef networkutil_h
#define networkutil_h

#include <ArduinoJson.h>
#include "util.h"

char login_form[] = "<!DOCTYPE html><html><head><meta name=viewport content=\"width=device-width, initial-scale=1\"><title>Flickerstrip Configuration</title><style>body{background:#09F}.b{background:#fff;border-radius:5px;width:300px;margin:0 auto;padding:10px}form{width:100%;margin:0}.sub{float:right;background:#09F;color:#fff;border:none;width:100px;border-radius:5px;-webkit-box-shadow:1px 1px 3px 0 rgba(0,0,0,.78);-moz-box-shadow:1px 1px 3px 0 rgba(0,0,0,.78);box-shadow:1px 1px 3px 0 rgba(0,0,0,.78)}.i{display:block;width:96%;padding:5px;margin-bottom:5px}table{width:100%}#pw{margin-bottom:30px}.h{font-size:24px;text-align:center;font-weight:700;margin-bottom:10px}</style><body><div class=b><div class=h>Configure Flickerstrip</div><form method=POST action=/connect><input class=i name=ssid placeholder=\"Network SSID\"> <input class=i id=pw name=password type=password placeholder=\"Password\"><table><tr><td><label><input id=cb type=checkbox onclick=\"document.getElementById('pw').type=document.getElementById('cb').checked?'text':'password'\">Show password</label><td><input class=sub type=submit></table></form></div>";
char confirm_page[] = "<!DOCTYPE html><html><head><meta name=viewport content=\"width=device-width,initial-scale=1\"><title>Flickerstrip Configuration</title><style type=text/css>body{background:#09F}.b{background:#fff;border-radius:5px;width:300px;margin:0 auto;padding:10px}.h{font-size:24px;text-align:center;font-weight:700;margin-bottom:10px}</style><body><div class=b><div class=h>Configuring...</div><p>Please reconnect to your network and launch the Flickerstrip app.</p></div>";

void sendHttpHeader(WiFiClient * client, int statusCode, const char * statusText, const char * contentType, int contentLength) {
  char buffer[300];
  int n = snprintf(buffer,300,"HTTP/1.0 %d %s\r\nContent-Type: %s\r\nConnection: close\r\nContent-Length:%d\r\n\r\n",statusCode,statusText,contentType,contentLength);
  client->write((uint8_t*)buffer,n);
}

void sendHttp(WiFiClient * client, int statusCode, const char * statusText, JsonArray& json) {
  int contentLength = json.measureLength();
  char buffer[contentLength+300];
  int n = snprintf(buffer,contentLength+300,"HTTP/1.0 %d %s\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length:%d\r\n\r\n",statusCode,statusText,contentLength);

  client->write((uint8_t*)buffer,n);

  n = json.printTo(buffer,contentLength+300-n);
  client->write((uint8_t*)buffer,n);
}

void sendChunkedHttp(WiFiClient * client, int statusCode, const char * statusText, const char * contentType) {
  char buffer[300];
  int n = snprintf(buffer,300,"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nTransfer-Encoding: chunked\r\n\r\n",statusCode,statusText,contentType);

  debugHex((char*)&buffer,n);

  client->write((uint8_t*)buffer,n);
}

void sendHttp(WiFiClient * client, int statusCode, const char * statusText, JsonObject& json) {
  int contentLength = json.measureLength();
  char buffer[contentLength+300];
  int n = snprintf(buffer,contentLength+300,"HTTP/1.0 %d %s\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length:%d\r\n\r\n",statusCode,statusText,contentLength);

  client->write((uint8_t*)buffer,n);

  n = json.printTo(buffer,contentLength+300-n);
  client->write((uint8_t*)buffer,n);
}

void sendHttp(WiFiClient * client, int statusCode, const char * statusText, const char * contentType, const char * content) {
  int contentLength = strlen(content);
  char buffer[contentLength+300];
  int n = snprintf(buffer,contentLength+300,"HTTP/1.0 %d %s\r\nContent-Type: %s\r\nConnection: close\r\nContent-Length:%d\r\n\r\n%s",statusCode,statusText,contentType,strlen(content),content);

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

int readUntil(WiFiClient * client, char * buffer, int length, const char * search, long timeout) {
  long start = millis();
  int n = 0;
  int i = 0;
  int searchlen = strlen(search);
  while(n < length && client->connected()) {
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

int getHeader(const char * buf, int n, const char * field, char ** retptr) {
    char * ptr = (char*)stristr(buf,field); //TODO this should respect the n field
    if (ptr == NULL) return 0;
    ptr = strstr(ptr,":");
    retptr[0] = (ptr + 1);
    while(*retptr[0] == 20) retptr[0]++;
    ptr = strstr(ptr,"\r\n");
    return ptr - *retptr;
}

int getBoundary(const char * buf, int n, char * boundary) {
    boundary[0] = 0;
    char * contentType;
    int len = getHeader(buf,n,"Content-Type",&contentType);
    if (len == 0) return 0;

    char * ptr = strstr(contentType,"boundary=");
    if (ptr == NULL) return 0;
    ptr = strstr(ptr,"=") + 1;
    char * end = strstr(ptr,"\r\n");

    memcpy(boundary+2,ptr,end-ptr);
    boundary[0]='-';
    boundary[1]='-';
    boundary[end-ptr + 2] = 0;
    return end-ptr + 2;
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

  char * getStart = ptr;
  while(1) {
      ptr = strstr(ptr+1,key);
      if (ptr == NULL) return -1;
      if (ptr-1 == getStart || ptr[-1] == '&') break;
  }

  char * a = strchr(ptr+1,' ');
  char * b = strchr(ptr+1,'&');
  if (b != NULL and (a == NULL or a > b)) {
    a = b;
  }

  if (a == NULL) return -1;

  loc[0] = ptr + strlen(key) + 1;
  if (a < loc[0]) return 0;
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

//From http://arduino.stackexchange.com/questions/18007/simple-url-decoding
void urlDecode(char* str, int n) {
    // Create two pointers that point to the start of the data
    char* leader = str;
    char* follower = leader;

    // While we're not at the end of the string (current character not NULL)
    while (*leader && leader-str < n) {
        // Check to see if the current character is a %
        if (*leader == '%') {

            // Grab the next two characters and move leader forwards
            leader++;
            char high = *leader;
            leader++;
            char low = *leader;

            // Convert ASCII 0-9A-F to a value 0-15
            if (high > 0x39) high -= 7;
            high &= 0x0f;

            // Same again for the low byte:
            if (low > 0x39) low -= 7;
            low &= 0x0f;

            // Combine the two into a single byte and store in follower:
            *follower = (high << 4) | low;
        } else {
            // All other characters copy verbatim
            *follower = *leader;
        }

        // Move both pointers to the next character:
        leader++;
        follower++;
    }
    // Terminate the new string with a NULL character to trim it off
    *follower = 0;
}



// Debugging
void printFound(int n, char * loc) {
  char tmp[n+1];
  memcpy(&tmp,loc,n);
  tmp[n] = 0;
  Serial.println(tmp);
}

#endif
