includepath = include
VPATH = test src/auth src/utils src/http include 
export C_INCLUDE_PATH = ./include
OBJDIR = obj/
Cflags = DEBUG

all: server

clean:
	@rm -rf ./obj/*.o
	@rm -rf ./bin/*

server: objectdir _server

_server:  main.c httpd.o httpresponse.o authenticate.o base64.o mimetypes.o
	gcc -g -o bin/server $< $(wildcard obj/*.o)

tests: objectdir _tests

_tests: test_cases.c httpresponse.o testmethods.o base64.o authenticate.o
	gcc -D_TESTING_ -g -o ./bin/test_cases $< $(wildcard obj/*.o)

responses: objectdir _responses

_responses: test_responses.c httpresponse.o 
	gcc -g -o ./bin/test_responses $< $(wildcard obj/*.o)

objectdir:
	mkdir -p $(OBJDIR) && mkdir -p bin/

## Compile with debugging.
%.o: %.c
	gcc -D$(Cflags) -g -c $< -o $(OBJDIR)$@

