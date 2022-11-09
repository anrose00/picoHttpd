void httpdRoute(HTTP_REQUEST *req, SOCKET sock)
{
   dp("starting route\r\n");

   if (strcmp("/",req->uri)==0 && strcmp("GET",req->method)==0 && strlen(req->querystring))
   {
      char log[BUFSIZE];
      sprintf(log,"Your parameters were: %s \n",req->querystring);
      dp( "In GET\r\n");
      OK(sock, log); 
   } else
   if (strcmp("/",req->uri)==0&&strcmp("GET",req->method)==0)
   {
      dp( "In GET\r\n");
      OK(sock, "Hello! You are using ", request_header("User-Agent"));
   } else 
   if (strcmp("/chunkme",req->uri)==0&&strcmp("GET",req->method)==0)
   {
      OK(sock, "chunked line 1\r\n","chunked line 2\r\n\r\n","chunked line 3\r\n and 4 in 1 chunk");
   } else
   if (strcmp("/",req->uri)==0&&strcmp("POST",req->method)==0)
   {
      char sBuf[50];
      sprintf(sBuf,"%u",req->payload_size);
      dp( "In Post handling %u\r\n",req->payload_size);
      dp("Received %s bytes\r\n",sBuf);
      dp("Payload: %s\r\n", req->payload);
      OK(sock, "Wow, seems that you POSTed ",sBuf," bytes. \r\n", 
         "Fetch the data using `payload` variable.");
   } else
      NOTFOUND(sock,"The requested resource cannot be found.\r\n");
}