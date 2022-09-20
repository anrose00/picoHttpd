#ifndef _HTTPD_RESPONSE___
#define _HTTPD_RESPONSE___

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#if (defined __linux__) || defined(__CYGWIN__)
   #define SOCKET int
   #include <sys/socket.h>
#else
   #if (defined WIN32)
      #include <Winsock2.h>
   #endif
#endif

#define MAXCHUNK 32768
#define BUFSIZE 1024

#define OK(...) _ok(__VA_ARGS__,NULL)
#define NOTFOUND(...) _notfound(__VA_ARGS__);
#define NOTAUTHORIZED(...) _notauthorized(__VA_ARGS__);
#define FORBIDDEN(...) _forbidden(__VA_ARGS__);

/// Send content including a response code. Ensure the va_list has an additional NULL argument.
#define SEND_CONTENT(...) _send_content(__VA_ARGS__, NULL);

#define HAS_VALUE 43690 // eq to binar 10101010101010 - unlikely random value

#define MAX_RESPONSE_HEADERS 49
typedef struct { char *name, *format, *value; int state; } header_r;
static header_r reshdr[1+MAX_RESPONSE_HEADERS] = { {NULL, NULL,NULL} };

void _ok(SOCKET sock, const char *content, ...);
void _notfound(SOCKET sock, const char *content);
void _notauthorized(SOCKET sock, const char *realm, const char *content);
void _forbidden(SOCKET sock, const char *content);
void _send_content(SOCKET sock, int response_code, const char *content,...);
void _internal_send_content(SOCKET sock, int response_code, const char *content, va_list content_args);
void init_response_headers();

char* http_description(int response_code);
char* response_header(const char* header);
void add_response_header(const char *header, const char *format, const char *value);
void reset_response_headers();
void release_response_header(header_r *responseheader);
char* all_response_headers();
header_r *find_response_header(const char* header);
int sock_printf(SOCKET sock, const char *format_string, ...);

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
            sprintf(&buffer,__VA_ARGS__) \
            OutputDebugString(buffer); \
         }
      #endif
   #endif
#else
#define dp(...)   
#endif

#endif /* HTTPD_RESPONSE__ */