# Pico HTTP Server in C 

This is a very simple HTTP server for Unix/Windows, written entirely in C.

This is a clone of the PICO project. It's very easy to use - simply add your routes into httpdRoute().

For more information about the HTTP protocol see: https://www.jmarshall.com/easy/http/

## How to use

1. include header `httpd.h`
2. write your httpdRoute() method, handling requests.
3. call `serve_forever("12913")` to start serving on port 12913

See `main.c`, a small example.

To log stuff, use `dp("message");`

View `httpd.h` for more information

based on <http://blog.abhijeetr.com/2010/04/very-simple-http-server-writen-in-c.html>

The original toolchain is based on make (Makefile)

There is an optional build based on cmake, which allows to build a windows DLL that acts as a
HTTP on localhost for accepting an OAuth redirect request. See comment in dllmain.c.
