#include "httpd.h"
#include "httpconst.h"
#include "httpresponse.h"
#include "mimetypes.h"
#include <stdio.h>
#include <stdarg.h>

#if (defined __linux__) || defined(__CYGWIN__)
  #include <signal.h> 
  #define SEP_BAD '\\'
  #define SEP_GOOD '/'
#else
  #define SEP_BAD '/'
  #define SEP_GOOD '\\'
#endif

#define HTTP_DOCROOT "t:/docext/SelfHtml81"

void enforcePlatformPathSeparator(char *string)
{
   char *ptr=string,c;
   
   while ( (c = *ptr) )
   {
      if (c == SEP_BAD)
         *ptr = SEP_GOOD;
      ptr++;
   }
}

void platformInit()
{
#ifdef WIN32
   WSADATA wsaData;
   int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void platformCleanup()
{
#ifdef WIN32
   WSACleanup();
#endif
}

char *str_documentRoot=NULL;

#ifndef _TESTING_
int main(int c, char** v)
{
   int iStatus; 
   
   iStatus = read_mimetypes("mimetypes.txt");
   if (iStatus != SUCCESS)
   {
      dp( "mimetypes.txt missing. Exiting\r\n");
      return 1;
   }
   platformInit();
      
   char *portNo = DEFAULT_PORT_NO;
   if (c>1) portNo=v[1];
   str_documentRoot = assign_string(str_documentRoot,HTTP_DOCROOT);
   serve_forever(portNo);
   
   if(str_documentRoot)
      free(str_documentRoot);
   
   platformCleanup();
   return 0;
}
#endif

void httpdRoute(HTTP_REQUEST *req)
{
   dp("starting route\r\n");

   if (strcmp("/",req->uri)==0 && strcmp("GET",req->method)==0 && strlen(req->querystring))
   {
      char log[BUFSIZE];
      sprintf(log,"Your parameters were: %s \n",req->querystring);
      dp( "In GET\r\n");
      OK(req->conn, log); 
   } 
   else
   if (strcmp("/",req->uri)==0&&strcmp("GET",req->method)==0)
   {
      dp( "In GET\r\n");
      OK(req->conn, "Hello! You are using ", request_header("User-Agent"));
   } 
   else 
      if (strcmp("GET",req->method)==0 && strncmp("/",req->uri,1)==0)
      {
         char str_path[BUFSIZE];
         strcpy(str_path,str_documentRoot);
         strcat(str_path,req->uri);
         enforcePlatformPathSeparator(str_path);
         send_file(req->conn,200,str_path,false);
         req->keepalive = 0;
      } 
      else
      if (strcmp("HEAD",req->method)==0 && strncmp("/",req->uri,1)==0)
      {
         char str_path[BUFSIZE];
         strcpy(str_path,str_documentRoot);
         strcat(str_path,req->uri);
         enforcePlatformPathSeparator(str_path);
         send_file(req->conn,200,str_path,true);
         req->keepalive = 0;
      }
      else
      if (strcmp("/",req->uri)==0&&strcmp("POST",req->method)==0)
      {
         char sBuf[50];
         sprintf(sBuf,"%u",req->payload_size);
         dp( "In Post handling %u\r\n",req->payload_size);
         dp("Received %s bytes\r\n",sBuf);
         dp("Payload: %s\r\n", req->payload);
         OK(req->conn, "Wow, seems that you POSTed ",sBuf," bytes. \r\n", 
            "Fetch the data using `payload` variable.");
      } 
      else
         NOTFOUND(req->conn,"The requested resource cannot be found.\r\n");
   
}