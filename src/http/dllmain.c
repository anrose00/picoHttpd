/* Pico HTTP Server as Windows DLL

   Author: Andreas Rosenberg
   License: LGPL

   This DLL is intended to start a small HTTP on local host, to act as a handler for
   an OAuth redirect. The server only waits for a get request on localhost:xxx/ and 
   stores the querystring, which contains the authorization code. 
   
   Start the server using this exported function:
   
   HANDLE WINAPI StartHttpdThread(LPSTR portNumberS, LPSTR buffer) 
   
   and provide the port number and a buffer which will receive the query string. 
   The server runs in a new thread. Your main thread should wait and check for the 
   response using the API call 
   
   BOOL WINAPI OAuthDataAvail()
   
   If OAuthDataAvail answers true, the query string has been written to the provided 
   buffer.
   
*/

#include <windows.h>
#include <Winsock2.h>
#include "httpd.h"
#include "httpresponse.h"

DWORD  threadID;
HANDLE threadHandle;
HANDLE heapHandle=NULL;
LPSTR bufferOAuthData=NULL;
LPSTR portNumberH=NULL;

#define MSG_SUCCESS "OAuth process completed\r\n\r\nPlease close this browser tab.\r\n"
#define MSG_NOTFOUND "HTTP/1.1 404 Not Found\r\n\r\nThe requested resource cannot be found.\r\n"

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   BOOL result;

   switch (fdwReason) {
      case DLL_PROCESS_ATTACH:
      case DLL_PROCESS_DETACH:
         if (portNumberH && heapHandle)
         {
            HeapFree(heapHandle,0,portNumberH);
            portNumberH = NULL;
         }
         if (heapHandle)
         {
            HeapDestroy(heapHandle);
            heapHandle = NULL;
         }
         WSACleanup();
      default:
         result=TRUE;
         break;
   }
   return result;
}

DWORD WINAPI startHTTPD(LPVOID parBuffer)
{
   serve_forever((*(char **)parBuffer));   // returns, if HTTPD has processed its request
   return 0;
}

HANDLE WINAPI StartHttpdThread(LPSTR portNumberS, LPSTR buffer)
{
   WSADATA wsaData;
   int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   heapHandle = HeapCreate(0,0,4096);
   portNumberH = HeapAlloc(heapHandle,0,256);
   strcpy(portNumberH,portNumberS);

   bufferOAuthData = buffer;
   threadHandle = CreateThread(NULL, 0, &startHTTPD, &portNumberH, 0, &threadID);
   return threadHandle;
}

BOOL WINAPI OAuthDataAvail()
{
   return (bufferOAuthData && strlen(bufferOAuthData));
}

void httpdRoute(HTTP_REQUEST *req)
{
  
   if (strcmp("/",req->uri)==0 && strcmp("GET",req->method)==0 && strlen(req->querystring))
   {
      dp( "In GET\r\n");
      if (bufferOAuthData)
      {
         strcpy(bufferOAuthData,req->querystring);
      }
      OK(req->conn,MSG_SUCCESS);
   } else
   if (strcmp("/",req->uri)==0&&strcmp("GET",req->method)==0)
   {
        dp( "In GET\r\n");
   } else 
   if (strcmp("/",req->uri)==0&&strcmp("POST",req->method)==0)
   {
        dp( "In Post handling %u\r\n",req->payload_size);
   } else
      NOTFOUND(req->conn,MSG_NOTFOUND);
   
   req->keepalive = 0;
}