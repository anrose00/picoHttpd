/* Mime type handling

   Author: Andreas Rosenberg
   License: LGPL

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "mimetypes.h"

list_mimetypes *plist_mimetypes;

/* read a line from a stream 
   - answer the number of chars read
   - chars are being put into buffer upto bufsize
*/

int read_line(FILE *file, char *buffer, int bufsize)
{
   int index=0;
   char c;
   
   *buffer = NUL;
   while (!feof(file))
   {
      buffer[index++] = c = fgetc(file);
      if (c == LF || index == bufsize)
      {
         buffer[index-1] = NUL;
         // if buffer is to small to process a line, skip chars until LF
         if (index == bufsize)
            while (!feof(file) && (fgetc(file) != LF));
         return index;
      }
   }
   return index;
}

/* fill a mimetype_entry given the appname, extlist and alloc size */

mimetype *fill_mimetype_entry(char *appname, char *extlist, int size_alloc)
{
   mimetype *mimetype_entry;
   int slen;
   
   mimetype_entry = malloc(sizeof(mimetype));
   if (!mimetype_entry)
      return NULL;
   
   mimetype_entry->entry = malloc(size_alloc);
   if (!mimetype_entry->entry)
   {
      free(mimetype_entry);
      return NULL;
   }
   memset(mimetype_entry->entry,0,size_alloc);
   slen = strlen(appname);
   strcpy(mimetype_entry->entry,appname);
   mimetype_entry->extlist = mimetype_entry->entry+slen+1;
   memcpy(mimetype_entry->extlist,extlist,size_alloc-slen);
   return mimetype_entry;
}

/* replace all occurrences of space with a replacement char */

char *str_replace_space(char *string, char repl)
{
   char *c=string;
   
   while (*c)
   {
      if (*c == SPACE)
         *c++=repl;
      else
         c++;
   }
   return string;
}

/* read mime types from file
   - count the number of lines
   - allocate the mime type list
   - reset and read each line and add each entry to the list
   - returns SUCCESS | ENOENT | ENOMEM
*/

int read_mimetypes(char *file_name)
{
   FILE *file;
   char line_buffer[BUFSIZE];
   char *c;
   int line_count=0,read_chars,size_mem;
   mimetype *mimetype_entry;

   file = fopen(file_name,"r");
   if (file)
   {
      while (!feof(file))
      {
         read_chars = read_line(file,&(line_buffer[0]),BUFSIZE-1);
         if (read_chars > 0)
            line_count++;
      }
      rewind(file);
        
      size_mem = sizeof(list_mimetypes)+(line_count+1*sizeof(char*));
      plist_mimetypes = malloc(size_mem);
      if (!plist_mimetypes)
         goto err_malloc1;
      memset(plist_mimetypes,0, size_mem);
      
      mimetype_entry = fill_mimetype_entry(APP_OCTET,APP_BIN"\0\0",sizeof(APP_OCTET)+sizeof(APP_BIN)+3);
      if (mimetype_entry)
         plist_mimetypes->mtypes[plist_mimetypes->num_types++] = mimetype_entry;
      else
         goto err_malloc2;
      
      while (!feof(file))
      {
         read_chars = read_line(file,line_buffer,BUFSIZE-1);
         if (read_chars > 0)
         {
            c = strchr(line_buffer,'=');
            // if we found an = sign, this looks like a valid line,otherwise skip
            if (c)
            {
               *c++ = NUL;
               str_replace_space(c,NUL);
               mimetype_entry = fill_mimetype_entry(line_buffer,c,read_chars);
               if (mimetype_entry)
                  plist_mimetypes->mtypes[plist_mimetypes->num_types++] = mimetype_entry;
               else
                  goto err_malloc2;

            }
         }
      }
      fclose(file);

      /*   
      { int i;
      for(i=plist_mimetypes->num_types-1;i>0;i--)
      {
         printf(plist_mimetypes->mtypes[i]->entry);
         printf(plist_mimetypes->mtypes[i]->extlist);
         printf("\r\n");
      }
      
      }*/
      return SUCCESS;
   }
   return ENOENT;
err_malloc2:
{
   int i;
   for(i=plist_mimetypes->num_types-1;i>0;i--)
   {
      free(plist_mimetypes->mtypes[i]);
   }
}
err_malloc1:
   fclose(file);
   return ENOMEM;         
}

/* find a mime type for a given file extension 
   - walk the list of entries
   - walk the list of file extensions for each entry
   - return the type/subtype if entry found, "application/octet-stream" otherwise 
     which must be the first entry in the list
   - argument maybe null
*/

char *find_mimetype(const char *ext)
{
   int i;
   char *c;
   
   if (ext)
   {
      for(i=plist_mimetypes->num_types-1;i>0;i--)
      {
         c = plist_mimetypes->mtypes[i]->extlist;
         do
         {
            if (strcmp(ext,c) == 0)
               return plist_mimetypes->mtypes[i]->entry;
            c += strlen(c)+1;
         }
         while (*c);
      }
   }
   return plist_mimetypes->mtypes[0]->entry;
}

