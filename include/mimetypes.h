#ifndef _MIMETYPES_H___
#define _MIMETYPES_H___

typedef struct {
   char *entry;      // a pointer to the string containing several several substrings delimited 
                     // by NUL and terminated by NUL,NUL
                     // the first substring is the mimetyp "type/subtype"
                     // the following stubstrings are known file extensions "txt" "htm" "html"..
   char *extlist;    // points to the second substring of entry (the list of file extensions)
} mimetype;

typedef struct {
   int      num_types;  // the number of types in the following list
   mimetype *mtypes[];  // an open array with pointers to mimetype structs
} list_mimetypes;


#define APP_OCTET "application/octet-stream"
#define APP_BIN   "bin"

#define BUFSIZE 1024
#define CR      '\r'
#define LF      '\n'
#define NUL     '\0'
#define SPACE   ' '

#define SUCCESS 0

int read_mimetypes(char *file_name);
char *find_mimetype(const char *ext);

#endif