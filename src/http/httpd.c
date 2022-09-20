#include "httpd.h"
#include "httpconst.h"
#include "httpresponse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#if (defined __linux__) || defined(__CYGWIN__)
   #include <unistd.h>
   #include <sys/socket.h>
   #include <netdb.h>
   #define EXIT(x) exit(x)
   #define closesocket(x) close(x)
   #define SOCKET int
#else
   #ifdef WIN32
      #include <windows.h>   
      #include <ws2def.h>
      #include <WS2tcpip.h>
      #define SHUT_RDWR SD_BOTH
      #define EXIT(x) ExitThread(x)
   #endif
#endif

// Client request

static SOCKET listenfd, clients[CONNMAX];
static void error(char *);
static void startServer(const char *);

typedef struct { char *name, *value; } header_t;

static header_t reqhdr[MAX_REQUEST_HEADERS+1] = { {"\0", "\0"} };
static int clientfd;

static char *buf;
static char _pico_hostname[BUFSIZE] = "\0";
static char *bufptr;
static int clientslot;
static int eob;

void serve_forever(const char *PORT)
{
   struct sockaddr_in clientaddr;
   socklen_t addrlen;
   int slot=0;
   int i;
    
   printf(
            "Server started %shttp://127.0.0.1:%s%s\n",
            "\033[92m",PORT,"\033[0m"
            );

   // Setting all elements to -1: signifies there is no client connected
   for (i=0; i<CONNMAX; i++)
        clients[i]=-1;

   startServer(PORT);
    
#ifndef WIN32
   // Ignore SIGCHLD to avoid zombie threads
   signal(SIGCHLD,SIG_IGN);
#endif

   // ACCEPT connections
   while (1)
   {
      addrlen = sizeof(clientaddr);
      clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

      if (clients[slot]<0)
      {
         perror("accept() error");
      }
      else
      {
#ifndef WIN32 
         if ( fork()==0 )
#endif
         {
            // I am now the client - close the listener: client doesnt need it
            HTTP_REQUEST *req;
            req = malloc(sizeof(HTTP_REQUEST));
            memset(req,0,sizeof(HTTP_REQUEST));
            clientslot=slot;
            init_response_headers();
            req->keepalive=1;
            respond(slot,req);
            free(req);
            closesocket(listenfd); 
            EXIT(0);
         } 
#ifndef WIN32
         else 
         {
            // I am still the server - close the accepted handle: server doesnt need it.
            closesocket(clients[slot]);
         }
#endif
      }
      while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
   }
}

//start server
void startServer(const char *port)
{
   struct addrinfo hints, *res, *p;

   // getaddrinfo for host
   memset (&hints, 0, sizeof(hints));
   hints.ai_family =  AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;

   // extract host name (if http name has been set)
   _pico_hostname[0] = '\0';
   int gai_result;
   if (gai_result=getaddrinfo(_pico_hostname,"http",&hints, &res)!=0)
   {
      dp("Note: cant find the FQDN (%s). Using local hostname instead.\r\n",gai_strerror(gai_result));
      // could not get it.  just use hostname
      strcpy(_pico_hostname, getenv("HOSTNAME"));
   }
   else
   {
      for(p=res; p!=NULL; p=p->ai_next) 
      {
         if (p->ai_canonname)
            dp("hostname: %s\r\n",p->ai_canonname);
      }      
      freeaddrinfo(res);
   }
   // get the Socket information for binding
   hints.ai_family=AF_INET;
   hints.ai_flags = AI_PASSIVE;
   if (getaddrinfo( NULL, port, &hints, &res) != 0)
   {
      perror ("getaddrinfo() error");
      EXIT(1);
   }
   // socket and bind
   for (p = res; p!=NULL; p=p->ai_next)
   {
      int option = 1;
      listenfd = socket (p->ai_family, p->ai_socktype, 0);
      setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option));
      if (listenfd == -1) continue;
      if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
   }
   if (p==NULL)
   {
      perror ("socket() or bind()");
      EXIT(1);
   }

   freeaddrinfo(res);

   // listen for incoming connections
   if ( listen (listenfd, 1000000) != 0 )
   {
      perror("listen() error");
      EXIT(1);
   }
}

// get request header
char* request_header(const char* name)
{
   header_t *h = reqhdr;
   while(h->name) {
      if (strcmp(h->name, name) == 0) return h->value;
      h++;
   }
   return NULL;
}

void reset_headers()
{
   header_t *h = reqhdr;
   while(h->name) {
      h->name="\0";
      h++;
   }
}

void reset_request()
{
   dp("\r\nPrepare for new request on slot:%u\r\n",clientslot);
   eob=0;
   bufptr=buf;
   reset_headers();
}

int get_bytes(HTTP_REQUEST *req)
{
   // move the buffer point to the end of the last buffer
   bufptr=bufptr+eob; 
    
   // calculate the remaining buffer size
   int buffsize = buf+MAXBUFFER-bufptr;
   if (buffsize>(MAXBUFFER/2)) buffsize=(MAXBUFFER/2);
   dp("Slot: %u, buffsize: %u bytes\r\n",clientslot, buffsize);

   // read the bytes from the socket.
   int rcvd = recv(clients[clientslot], bufptr, buffsize, 0);

   // handle outcome of recv call.
   if (rcvd < 0) 
   { // receive error
      dp("recv() error or session end\r\n");
      req->keepalive=0;
   }
   else if (rcvd == 0)
   {
      // receive socket closed
      dp("Client disconnected gracefully.\r\n");
      req->keepalive=0;
   }
   else // message received
   {
      dp("Received %u bytes buf=%u, bufptr=%u\r\n",rcvd, buf, bufptr);
      // terminate the incoming buffer;
      bufptr[rcvd] = '\0';
   }
   // update the new end of buffer 
   eob = eob+rcvd;

   // return the number of bytes recieved on this occaion.
   return rcvd;
}

//client connection
void respond(int n, HTTP_REQUEST *req)
{
   int rcvd;
   buf = malloc(MAXBUFFER);

   // header pointers.
   header_t *h = reqhdr;
   char *header_name, *header_value;
   char *t, *t2;
   char *eohptr=NULL;
   reset_request();

   while (req->keepalive)
   {
      rcvd=get_bytes(req);
        
      if (rcvd>0)
      {
         //wait for the headers to be completely loaded.
         if (!eohptr)
         {
            eohptr=strstr(buf, "\r\n\r\n");
            // if no double crlf is found, go back for more bytes.
            if (!eohptr) 
            {
               // TODO: if the buffer is full and still no header, then return a 413 Entity too large.
               continue;
            }
             // double crlf is found, reset bufptr back to the beginning. 
             // note: EOB will still be the last by byte received.
             bufptr=buf;
         }  
         int protcol_rcvd=0; 
         int headers_rcvd=0;

         // extract the protocol, URL and 
         if (!protcol_rcvd)
         {
            req->method = strtok(bufptr, " \t\r\n");
            req->uri = strtok(NULL, " \t");
            req->prot = strtok(NULL, " \t\r\n");

            if (req->method==NULL || req->uri == NULL || req->prot==NULL)
            {
               // send bad request 400 
               req->keepalive=0;
               break;
            } 

            protcol_rcvd=1;
            dp("\x1b[32m + [%s] %s\x1b[0m\n", req->method, req->uri);


            if (req->querystring = strchr(req->uri, '?'))
            {
               *req->querystring++ = '\0'; //split URI
            }
            else
            {
               req->querystring = req->uri - 1; //use an empty string
            }
         }

         if (!headers_rcvd && protcol_rcvd) 
         {
            header_t *h = reqhdr;
            while (h < reqhdr + MAX_REQUEST_HEADERS)
            {
               // is there a new header.
               header_name = strtok(NULL, "\r\n: \t");
               if (!header_name)
                   break;
              
               header_value = strtok(NULL, "\r\n");
               while (*header_value && *header_value == ' ')
                   header_value++;
               h->name = header_name;
               h->value = header_value;
               h++;
               dp("[H] %s: %s\n", header_name, header_value);
               t = header_value + 1 + strlen(header_value);
               if (t[1] == '\r' && t[2] == '\n')
               {
                  headers_rcvd=1;
                  break;
               }
            }
            if (headers_rcvd)
            {
               t++;                                        // now the *t shall be the beginning of user payload
               t2 = request_header(HEADER_CONTENT_LENGTH); // and the related header if there is
               req->payload = t;
               req->payload_size = t2 ? atol(t2) : (eob - (t - buf));
               if (t2)
               {
                  dp("Expecting %s bytes\r\n", t2);
                  dp("%u Bytes Received\r\n", req->payload_size);
               }
            }
         }
         if (req->payload)
         { 
            clientfd = clients[n];
            
            // call router
            httpdRoute(req,clientfd);
            reset_request();
         }
      }
   }
   free(buf);
   shutdown(clientfd, SHUT_RDWR);         //All further send and recieve operations are DISABLED...
   clients[n]=-1;
}

char* pico_hostname()
{
    return &_pico_hostname[0];
}