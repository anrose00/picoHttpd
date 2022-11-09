#include "httpresponse.h"
#include "httpconst.h"
#include "mimetypes.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


char *assign_string(char *targetstr, const char *sourcestr)
{
   if (targetstr)
   {
      if (strlen(targetstr)>0) free(targetstr);
   }
   int sz = strlen(sourcestr);
   targetstr = malloc(sizeof(char)*(1+sz));
   targetstr[sz]='\0';
   strncpy(targetstr, sourcestr, sz);
   return targetstr;
}

int sock_printf(SOCKET sock, const char *format_string, ...)
{
   va_list args;
   va_start(args, format_string);
   int size,res;
     
   char *sBuf = malloc(MAXCHUNK);
   if (!sBuf)
      return -1;
   size = vsprintf(sBuf,format_string,args);
   res = send(sock,sBuf,size,0);
   if (res == -1)
      dp("Socket send error");
   free(sBuf);
   va_end(args);
   return size;
}

char *http_description(int response_code)
{
   http_response_code *h = HTTP_RESPONSE_CODES;
   while(h->code)
   {
      if (h->code==response_code) return h->value;
      h++;
   }
   return NULL;
}

/// locate the indicated header or return a pointer to the next available.
header_r *find_response_header(const char* header)
{
   int i=0;
   header_r *h = reshdr;
   while(h->state==HAS_VALUE && i<MAX_RESPONSE_HEADERS)
   {
      if (strcmp(h->name, header) == 0) return h;
      h++;
      i++;
   }
   return (i>MAX_RESPONSE_HEADERS) ? NULL: h;
}

void init_response_headers()
{
   int i=0;
   header_r *h = reshdr;
   while(i<MAX_RESPONSE_HEADERS) {
      h->state=0;
      h++;
      i++;
   }
}

char *response_header(const char* header)
{
   header_r *h = find_response_header(header);
   return (h) ? h->value : NULL;
}

void reset_response_headers()
{
   release_response_header(&reshdr[0]);
   release_response_header(&reshdr[1]);
}

void release_response_header(header_r *responseheader)
{
   if (responseheader->state==HAS_VALUE)
   {
      if (responseheader->name) free(responseheader->name); responseheader->name=NULL;
      if (responseheader->format) free(responseheader->format); responseheader->format=NULL;
      if (responseheader->value) free(responseheader->value); responseheader->value=NULL;
      responseheader->state = 0;
   }
}

void add_response_header(const char *header, const char *format, const char *value)
{
   header_r *h = find_response_header(header);
   if (!h)
   {    
      dp("Response headers full - %s could not be added",header);
   } 
   else
   {
      // if this header has been used before, release memory
      //release_response_header(h);
      // the name and format are fixed length, so just allocate the right amount of space
      h->name = assign_string(h->name, header);
      h->format = assign_string(h->format, format);
      
      // length of the value is a bit ambigous here, so assign to a temporary before copying it in.
      char lvalue[BUFSIZE*2];
      snprintf(lvalue, BUFSIZE*2-1, format, value);
      h->value = assign_string(h->value, lvalue);
      h->state = HAS_VALUE;
   }
}

/// Implementation of the send header macro
void send_header(SOCKET sock, int code)
{
   char *description = http_description(code);
   sock_printf(sock,"HTTP/1.1 %u %s\r\n",code, description);
   dp("[R] HTTP/1.1 %u %s\r\n",code, description);
   int i=0;
   header_r *h = reshdr;
   while( (h->value || h->name) && i<MAX_RESPONSE_HEADERS)
   {
      if (h->name && h->value)
      {
        sock_printf(sock,"%s: %s\r\n", h->name, h->value);
        dp("[R] %s: %s\r\n", h->name, h->value);
      }
      h++;
   }
   sock_printf(sock,"\r\n");
   dp("\r\n");
}

/* The function that actually outputs the content determining if the 
   data needs to be sent as a stream (chunked) or if it can be sent with a content length.
   The va_list must consist of string ptrs and must be terminated with a NULL ptr. 
*/
   
void _internal_send_content(SOCKET sock, int response_code, const char *content, va_list content_args)
{
   int contentlength = 0;
   if (content) contentlength = strlen(content);
    
   const char *arg_content = va_arg(content_args, const char *);
   if (!arg_content) 
   {  // there is only a single argument, so send content length and the data.
      if (contentlength>0)
      {
         char contentlengthstr[13];
         snprintf(contentlengthstr, 13, "%u\0", contentlength);
         add_response_header(HEADER_CONTENT_LENGTH, "%s", contentlengthstr);
         send_header(sock, response_code);
         sock_printf(sock, content);
         dp(content);
      }
      else
      {
         send_header(sock, response_code);
      }
   }
   else
   {
      // there are multiple arguments, so we will chunk the data.
      add_response_header(HEADER_TRANSFER_ENCODING,"%s",HEADER_TRANSFER_TYPE_CHUNKED); 
      send_header(sock, response_code);
      if (content)
      {
         sock_printf(sock,"%x\r\n%s\r\n",contentlength, content);
         dp("%x\r\n%s\r\n",contentlength, content); 
      } 
      while (arg_content)
      {
         contentlength=strlen(arg_content); 
         if (contentlength > 0)
         { 
            sock_printf(sock,"%x\r\n%s\r\n", contentlength, arg_content);
            dp("%x\r\n%s\r\n", contentlength, arg_content);
         }
         arg_content = va_arg(content_args, const char *);
      }
      sock_printf(sock,"0\r\n");
   }
   sock_printf(sock,"\r\n");
   dp("\r\n");
   reset_response_headers();
}

void send_file(SOCKET sock, int response_code, const char *file_name, int hdr_only)
{
   int  fsize,index;
   FILE *file;
   char cont_len[13];
   char str_line[BUFSIZE];
   char c;
   char *ext;
   
   file = fopen(file_name,"rb");
   if (file)
   {
      ext = strrchr(file_name,'.');
      if (ext)
         ext++;
      fseek(file, 0L, SEEK_END);
      fsize = ftell(file);
      sprintf(cont_len,"%d",fsize);
      rewind(file);
      add_response_header(HEADER_CONTENT_LENGTH, "%s", cont_len);
      add_response_header(HEADER_CONTENT_TYPE, "%s", find_mimetype(ext));
      add_response_header(HEADER_CONNECTION, "%s",HEADER_VALUE_CONNECTION_CLOSE);
      send_header(sock, response_code);

      /* send content if requested */
      if (!hdr_only)
      {
         index = 0;
         while (!feof(file))
         {
            str_line[index++] = c = fgetc(file);
            if (index == (BUFSIZE-1))
            {
               send(sock,str_line,index,0);
               index = 0;
            }
         }
         if (index > 0)
            send(sock,str_line,index,0);
      }
      fclose(file);
      sock_printf(sock,"\r\n");
      dp("\r\n");
   }
   else
   {
      NOTFOUND(sock,"The requested resource cannot be found.\r\n");
   }
   reset_response_headers();
}
/// A successful request has been received and content will be returned
void _ok(SOCKET sock, const char *content, ...) 
{
   va_list args;
   va_start(args, content);
   _internal_send_content(sock, 200, content, args);
   va_end(args);  
}

/// The requested resource cannot be found
void _notfound(SOCKET sock, const char *content)
{
   SEND_CONTENT(sock, 404, content); 
}
/// When the client sends no authentication header or the header
/// has been rejected, fewer times than the max limit)
void _notauthorized(SOCKET sock, const char *realm, const char *content)
{
   // ensure there is a safe realm to apply
   char myRealm[BUFSIZE]="PICOServer"; 
   int rl = 10;
   if (realm)
   {  
      rl=strlen(realm);
      strncpy(myRealm,realm,BUFSIZE-1);
   }
   myRealm[rl]=0; 
   add_response_header(HEADER_CONNECTION, "%s", HEADER_VALUE_CONNECTION_KEEP_ALIVE);
   add_response_header(HEADER_WWW_AUTHENTICATE,"Basic realm=\"%s\"",myRealm);
   SEND_CONTENT(sock, 401 , content);
}

/// When authentication fails and the server refuses to accept any more attemps 
/// from the client.
void _forbidden(SOCKET sock, const char *content)
{
   add_response_header(HEADER_CONNECTION,"%s", HEADER_VALUE_CONNECTION_CLOSE);
   SEND_CONTENT(sock, 403, content);
}

void _send_content(SOCKET sock, int response_code, const char *content, ...)
{
   va_list args;
   va_start(args, content);
   _internal_send_content(sock, response_code, content, args);
   va_end(args);
}