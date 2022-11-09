#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>

#if (defined __linux__) || defined(__CYGWIN__)
   #define SOCKET int
#else
   #if (defined WIN32)
      #include <Winsock2.h>
   #endif
#endif

//Server control functions
#define DEFAULT_PORT_NO "12913"
#define MAXBUFFER 0x1FFFE
#define MAX_REQUEST_HEADERS 49
#define CONNMAX 1000
#define BUFSIZE 1024

typedef struct {
   int     conn;       // connection handle
   char    *method,    // "GET" or "POST"
           *uri,       // "/index.html" things before '?'
           *querystring,        // "a=1&b=2"     things after  '?'
           *prot;      // "HTTP/1.1"

   char    *payload;     // for POST
   int      payload_size;
   int      auth_attempts;
   int      keepalive;
} HTTP_REQUEST;

void serve_forever(const char *PORT);
char* request_header(const char* name);
char* pico_hostname();
int get_bytes();
void reset_request();
void reset_headers();
void respond(HTTP_REQUEST *req);

// user shall implement this function

void httpdRoute(HTTP_REQUEST *req);

// some usefule macros for `route()`
#define ROUTE_START()       if (0) {
#define ROUTE(METHOD,URI)   } else if (strcmp(URI,uri)==0&&strcmp(METHOD,method)==0) {
#define ROUTE_GET(URI)      ROUTE("GET", URI) 
#define ROUTE_POST(URI)     ROUTE("POST", URI) 
#define ROUTE_END()         } else printf(\
                                "HTTP/1.1 404 Not Found\r\n\r\n" \
                                "The requested resource cannot be found.\r\n" \
                            );

#ifdef DEBUG 
   #if (defined __linux__) || defined(__CYGWIN__)
      #define dp(...) \
        { \
          fprintf(stderr, __VA_ARGS__); \
        }
   #else
      #if (defined WIN32)
         #define dp(...) \
         { \
            char buffer0815[1024]; \
            sprintf(&buffer0815,__VA_ARGS__); \
            OutputDebugString(buffer0815); \
         }
      #endif
   #endif
#else
#define dp(...)
#endif
                            
#endif